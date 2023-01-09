from paho.mqtt import client as mqtt_client
import sys
import time
from opcua import ua, Server, uamethod
import random


broker = "localhost"
port = 1883
topic_temperature = "temperatura/value"
topic_ventoinha = "ventoinha/value"
topic_ventoinha_override = "ventoinha/override"

client_id = f"python-mqtt-{random.randint(0, 1000)}"
msg_temperatura = float("6.7")
msg_ventoinha = False


def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        global msg_temperatura
        global msg_ventoinha
        # print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        if msg.topic == topic_temperature:
            msg_temperatura = msg.payload.decode()
        if msg.topic == topic_ventoinha:
            msg_ventoinha = msg.payload.decode()

    client.subscribe(topic_temperature)
    client.subscribe(topic_ventoinha)
    client.on_message = on_message


def mqtt_start():
    client = connect_mqtt()
    subscribe(client)
    client.loop_start()


# OPC UA Method
# uamethod automatically converts variants
@uamethod
def toggle_ventoinha(parent):
    client.publish(topic_ventoinha_override)
    print("Clicou")


def opcua_start():
    # setup our server
    server = Server()
    server.set_endpoint("opc.tcp://0.0.0.0:4840/UA/rasp")

    # setup our own namespace, not really necessary but should as spec
    uri = "http://examples.freeopcua.github.io"
    idx = server.register_namespace(uri)

    objects = server.get_objects_node()

    types = server.get_node(ua.ObjectIds.BaseObjectType)

    # Definir um novo tipo de objeto para a estufa
    # server.get_root_node().get_child(["0:Types", "0:ObjectTypes", "0:BaseObjectType"])
    estufa_type = types.add_object_type(idx, "Estufa")
    estufa_type.add_variable(0, "Temperatura", float(
        msg_temperatura)).set_modelling_rule(True)
    estufa_type.add_variable(1, "VentoinhaStatus",
                             bool(msg_ventoinha)).set_modelling_rule(True)
    estufa_type.add_method(2, "VentoinhaOverride",
                           toggle_ventoinha).set_modelling_rule(True)

    # Adicionar um objeto do tipo estufa ao address space
    my_obj = objects.add_object(idx, "Estufa", estufa_type.nodeid)
    variables = my_obj.get_variables()
    server.start()
    while True:
        variables[0].set_value(float(msg_temperatura))
        variables[1].set_value(msg_ventoinha)
        pass


if __name__ == "__main__":

    # MQTT Start
    mqtt_start()

    # OPCUA Start
    opcua_start()

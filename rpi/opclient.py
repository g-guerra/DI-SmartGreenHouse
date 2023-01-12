import random
import sys
import time

from opcua import Server, ua, uamethod
from paho.mqtt import client as mqtt_client

broker = "localhost"
port = 1883
topic_temperature = "temperatura/value"
topic_ventoinha = "ventoinha/value"
topic_ventoinha_mode = "ventoinha/mode"
topic_ventoinha_control = "ventoinha/control"
topic_led_control = "led/control"
topic_led_value = "led/value"

client_id = f"python-mqtt-{random.randint(0, 1000)}"
client = mqtt_client.Client(client_id)

msg_temperatura = float("6.7")
msg_ventoinha = False


def connect_mqtt():
    global client

    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

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
def publish_mqtt(parent, command):
    if command == "manual":
        client.publish(topic_ventoinha_mode, "manual")
    elif command == "auto":
        client.publish(topic_ventoinha_mode, "auto")
    elif command == "on":
        client.publish(topic_ventoinha_control, "on")
    elif command == "off":
        client.publish(topic_ventoinha_control, "off")
    elif command == "led_on":
        client.publish(topic_led_control, "on")
    elif command == "led_off":
        client.publish(topic_led_control, "off")
    elif "led_red" in command:
        client.publish(topic_led_value, str("red " + command.split(" ")[1]))
    elif "led_green" in command:
        client.publish(topic_led_value, str("green " + command.split(" ")[1]))
    elif "led_blue" in command:
        client.publish(topic_led_value, str("blue " + command.split(" ")[1]))
    # print(str("red " + command.split(" ")[1]))
    return


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
    estufa_type = types.add_object_type(idx, "Estufa")
    estufa_type.add_variable(0, "Temperatura", float(
        msg_temperatura)).set_modelling_rule(True)
    estufa_type.add_variable(1, "VentoinhaState",
                             bool(msg_ventoinha)).set_modelling_rule(True)
    estufa_type.add_method(2, "VentoinhaOverride",
                           publish_mqtt, [ua.VariantType.String]).set_modelling_rule(True)

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

from paho.mqtt import client as mqtt_client
import sys
import time
from opcua import ua, Server
import random

sys.path.insert(0, "..")

broker = "localhost"
port = 1883
topic = "testTopic"

client_id = f"python-mqtt-{random.randint(0, 1000)}"
message = "6.7"


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
        global message
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        message = msg.payload.decode()

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_start()


if __name__ == "__main__":

    # setup our server
    server = Server()
    server.set_endpoint("opc.tcp://0.0.0.0:4840/UA/rasp")

    # setup our own namespace, not really necessary but should as spec
    uri = "http://examples.freeopcua.github.io"
    idx = server.register_namespace(uri)

    # get Objects node, this is where we should put our nodes
    objects = server.get_objects_node()

    # populating our address space
    myobj = objects.add_object(idx, "MyObject")
    myvar = myobj.add_variable(idx, "Temp", 6.7)
    myvar.set_writable()  # Set MyVariable to be writable by clients

    run()

    # starting!
    server.start()

    try:
        count = 0
        while True:
            time.sleep(1)
            myvar.set_value(float(message))
            print(float(message))

    finally:
        # close connection, remove subcsriptions, etc
        server.stop()

from opcua import Server
from random import randint
import datetime
import time

server = Server()

url = "opc.tcp://192.168.1.203:4840"
server.set_endpoint(url)

name = "OPCUA_SIMULATION_SERVER"
addspace = server.register_namespace(name)

node = server.get_objects_node()

Param = node.add_object(addspace, "Parameters")

Temp = Param.add_variable(addspace, "Temperature", 0)
Hum = Param.add_variable(addspace, "Humidade", 0)

Temp.set_writable()
Hum.set_writable()

server.start()
print("Server started at {}".format(url))

while True:

    Temperature = randint(10, 50)
    Humidity = randint(200, 999)

    print(Temperature, Humidity)

    Temp.set_value(Temperature)
    Hum.set_value(Humidity)

    time.sleep(2)

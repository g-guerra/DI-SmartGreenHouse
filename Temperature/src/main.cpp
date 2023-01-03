#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_MCP9808.h"

// SSID/Password combination
const char *ssid = "nao_abrir";
const char *password = "qwertyuiop";

// MQTT Broker IP address
const char *mqtt_server = "192.168.5.228";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const int ledPin = 4;

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 temperature_sensor = Adafruit_MCP9808();

void setup_wifi();
void callback(char *topic, byte *message, unsigned int length);

void setup()
{
  Serial.begin(115200);

  if (!temperature_sensor.begin())
  {
    Serial.println("Couldn't find MCP9808!");
    while (1)
      ;
  }

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "on")
    {
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if (messageTemp == "off")
    {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // put your main code here, to run repeatedly:
  // Read and print out the temperature, then convert to *F
  float temp_c = temperature_sensor.readTempC();
  char tempString[8];
  dtostrf(temp_c, 1, 2, tempString);
  Serial.print("Temp: ");
  Serial.print(temp_c);
  client.publish("testTopic", tempString);
  Serial.print(" C\t");

  delay(250);

  temperature_sensor.shutdown_wake(1);
  delay(2000);
  temperature_sensor.shutdown_wake(0);
}
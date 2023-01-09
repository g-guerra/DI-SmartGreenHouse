#include "Adafruit_MCP9808.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>

#define FAN_PIN 4
#define TEMP_MAX 28
#define TEMP_MIN 25
// SSID/Password combination
// const char *ssid = "nao_abrir";
// const char *password = "qwertyuiop";
// const char *mqtt_server = "192.168.5.228";
const char *ssid = "NetFixe";
const char *password = "1DA7FFD5BB";
const char *mqtt_server = "192.168.1.178";

const char *fan_override_topic = "ventoinha/override";
const char *fan_value_topic = "ventoinha/value";
const char *temperature_value_topic = "temperatura/value";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 temperature_sensor = Adafruit_MCP9808();
bool fan_status = false;
bool fan_override = false;

void setup_wifi();
void callback(char *topic, byte *message, unsigned int length);
void reconnect();
void fan_control(float);

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

  pinMode(FAN_PIN, OUTPUT);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // Read and print out the temperature, then convert to C
  char temp_string[8];

  float temp_c = temperature_sensor.readTempC();

  dtostrf(temp_c, 1, 2, temp_string);
  // End temperatue check

  fan_control(temp_c);

  client.publish(temperature_value_topic, temp_string);
  Serial.print(fan_status ? "True" : "False");
  client.publish(fan_value_topic, fan_status ? "True" : "False");

  Serial.print("Temp: ");
  Serial.print(temp_c);
  Serial.print(" C\t");
  Serial.print("Fan: ");
  Serial.print(fan_status);
  Serial.print("\n");

  delay(250);
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

  pinMode(FAN_PIN, OUTPUT);
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

  // Changes the override value
  if (String(topic) == fan_override_topic)
  {
    fan_override = !fan_override;
    Serial.print("Changing fan override");
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
      client.subscribe(fan_override_topic);
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

void fan_control(float temp_c)
{
  if (fan_override == true)
  {
    fan_status = false;
    return;
  }
  if (temp_c > TEMP_MAX)
    fan_status = true;
  if (temp_c < TEMP_MIN)
    fan_status = false;
  digitalWrite(FAN_PIN, fan_status);
}
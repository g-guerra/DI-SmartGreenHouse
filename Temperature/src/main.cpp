#include "Adafruit_MCP9808.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>

#define FAN_PIN 4
#define TEMP_MAX 24
#define TEMP_MIN 22

enum fan_modes
{
  AUTO,
  MANUAL,
  ON,
  OFF
};
// SSID/Password combination
const char *ssid = "nao_abrir";
const char *password = "qwertyuiop";
const char *mqtt_server = "192.168.2.228";

const char *fan_control_topic = "ventoinha/control";
const char *fan_mode_topic = "ventoinha/mode";
const char *fan_value_topic = "ventoinha/value";
const char *temperature_value_topic = "temperatura/value";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 temperature_sensor = Adafruit_MCP9808();

enum fan_modes fan_state = OFF;
enum fan_modes fan_mode = AUTO;

void setup_wifi();
void callback(char *topic, byte *message, unsigned int length);
void reconnect();
void fan_control(char *, float);

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
  char fan_string[15];

  float temp_c = temperature_sensor.readTempC();

  dtostrf(temp_c, 1, 2, temp_string);
  // End temperatue check

  fan_control(fan_string, temp_c);

  client.publish(temperature_value_topic, temp_string);
  client.publish(fan_value_topic, fan_string);

  Serial.print("Temp: ");
  Serial.print(temp_c);
  Serial.print(" C\t");
  Serial.printf("Fan: %s", fan_string);
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

  if (String(topic) == fan_mode_topic)
  {
    if (messageTemp == "manual")
    {
      fan_mode = MANUAL;
    }
    else if (messageTemp == "auto")
    {
      fan_mode = AUTO;
    }
  }
  if (String(topic) == fan_control_topic)
  {
    if (messageTemp == "on")
    {
      fan_state = ON;
    }
    else if (messageTemp == "off")
    {
      fan_state = OFF;
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
      client.subscribe(fan_mode_topic);
      client.subscribe(fan_control_topic);
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

void fan_control(char *fan_string, float temp_c)
{
  if (fan_mode == MANUAL)
  {
    if (fan_state == ON)
    {
      digitalWrite(FAN_PIN, HIGH);
      strcpy(fan_string, "manual, on");
    }
    else
    {
      digitalWrite(FAN_PIN, LOW);
      strcpy(fan_string, "manual, off");
    }
  }
  else
  {
    if (temp_c > TEMP_MAX)
    {
      digitalWrite(FAN_PIN, HIGH);
      strcpy(fan_string, "auto, on");
    }
    if (temp_c < TEMP_MIN)
    {
      digitalWrite(FAN_PIN, LOW);
      strcpy(fan_string, "auto, off");
    }
  }
}
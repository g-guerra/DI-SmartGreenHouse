#include "Adafruit_MCP9808.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <string.h>

#define FAN_PIN 4
#define TEMP_MAX 29
#define TEMP_MIN 25

#define RED_PIN 33
#define GREEN_PIN 32
#define BLUE_PIN 18
#define RED_CHANNEL 3
#define GREEN_CHANNEL 4
#define BLUE_CHANNEL 5

enum modes
{
  AUTO,
  MANUAL,
  ON,
  OFF
};

// SSID/Password combination
const char *ssid = "nao_abrir";
const char *password = "qwertyuiop";
const char *mqtt_server = "192.168.158.228";

const char *fan_control_topic = "ventoinha/control";
const char *fan_mode_topic = "ventoinha/mode";
const char *fan_value_topic = "ventoinha/value";
const char *temperature_value_topic = "temperatura/value";
const char *led_control_topic = "led/control";
const char *led_value_topic = "led/value";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 temperature_sensor = Adafruit_MCP9808();

enum modes fan_state = OFF;
enum modes fan_mode = AUTO;
enum modes led_state = OFF;
int red_value = 0;
int green_value = 0;
int blue_value = 0;

void setup_wifi();
void callback(char *topic, byte *message, unsigned int length);
void reconnect();
void led_control();
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

  ledcSetup(RED_CHANNEL, 5000, 8);
  ledcSetup(GREEN_CHANNEL, 5000, 8);
  ledcSetup(BLUE_CHANNEL, 5000, 8);

  ledcAttachPin(RED_PIN, RED_CHANNEL);
  ledcAttachPin(GREEN_PIN, GREEN_CHANNEL);
  ledcAttachPin(BLUE_PIN, BLUE_CHANNEL);
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
  led_control();

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
  if (String(topic) == led_control_topic)
  {
    if (messageTemp == "on")
    {
      led_state = ON;
    }
    else if (messageTemp == "off")
    {
      led_state = OFF;
    }
  }
  if (String(topic) == led_value_topic)
  {
    if (messageTemp.charAt(0) == 'r')
    {
      red_value = messageTemp.substring(3).toInt();
    }
    if (messageTemp.charAt(0) == 'g')
    {
      green_value = messageTemp.substring(5).toInt();
    }
    if (messageTemp.charAt(0) == 'b')
    {
      blue_value = messageTemp.substring(4).toInt();
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
      client.subscribe(led_control_topic);
      client.subscribe(led_value_topic);
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
void led_control()
{
  if (led_state == ON)
  {
    ledcWrite(RED_CHANNEL, red_value);
    ledcWrite(GREEN_CHANNEL, green_value);
    ledcWrite(BLUE_CHANNEL, blue_value);
  }
  else
  {
    ledcWrite(RED_CHANNEL, 0);
    ledcWrite(GREEN_CHANNEL, 0);
    ledcWrite(BLUE_CHANNEL, 0);
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
/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-light-sensor
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"

const char* ssid = "HUAWEI-0C66";
const char* password = "RDQJ32QET5R";
const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* mqtt_server = "tcp://0.tcp.ap.ngrok.io";
//const int mqtt_port = 16451;


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsgLampu = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 21600; /*GMT OFFSET +5 HOURS(18000 SEC)*/
const int   daylightOffset_sec = 3600; /*1 hour daylight offset*/

#define MSG_BUFFER_SIZE_DHT11 (50)
char msgDHT11[MSG_BUFFER_SIZE_DHT11];
char timeStringBuff[50]; //50 chars should be enough
#define MQTTpubQos 2

int analogValue;

#define LIGHT_SENSOR_PIN 36 // ESP32 pin GIOP36 (ADC0)

void setup_wifi() {

  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("raw_data/in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
//  client.setServer(mqtt_server, mqtt_port);
  client.setKeepAlive(60);
  
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    client.loop();
  } 

  if (timeinfo.tm_min % 10 == 0) {
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
    // reads the input on analog pin (value between 0 and 4095)
    analogValue = analogRead(LIGHT_SENSOR_PIN);
  
    Serial.print("Analog Value = ");
    Serial.print(analogValue);   // the raw analog reading

    snprintf(msgDHT11, MSG_BUFFER_SIZE_DHT11, "%04d-%02d-%02d_%02d:%02d | Light: %d", 
        timeinfo.tm_year+1900, timeinfo.tm_mon+1,
        timeinfo.tm_mday, timeinfo.tm_hour, 
        timeinfo.tm_min,
        analogValue);


    client.publish("/raw_data/out", msgDHT11);
//    client.publish("raw_data", msgDHT11, MQTTpubQos);
    delay(590000);
  }
}

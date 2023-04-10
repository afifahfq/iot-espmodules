/*
  Tugas : Sensor DHT11 - MQTT
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"

const char* ssid = "Redmi Note 8";
const char* password = "123456gh";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsgLampu = 0;
unsigned long timewait = 10000; //every 15 minutes

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 18000; /*GMT OFFSET +5 HOURS(18000 SEC)*/
const int   daylightOffset_sec = 3600; /*1 hour daylight offset*/

#define MSG_BUFFER_SIZE_DHT11 (50)
char msgDHT11[MSG_BUFFER_SIZE_DHT11];
char timeStringBuff[50]; //50 chars should be enough

float humi;
float temp;

#include "DHT.h"
#define DHT11PIN 22

DHT dht(DHT11PIN, DHT11);

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


  // if ((char)payload[0] == '1' && (strcmp(topic, "/topic/mqtt/13519208/lampu/in") == 0)) {
  //   digitalWrite(lampu, HIGH);
  // } else if ((char)payload[0] == '0' && (strcmp(topic, "/topic/mqtt/13519208/lampu/in") == 0)) {
  //   digitalWrite(lampu, LOW);
  // }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("/topic/mqtt/sensorDHT11/in");
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
  Serial.begin(9600);

  /* Start the DHT11 Sensor */
  dht.begin();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
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
  
  if (timeinfo.tm_min % 2 == 0 ) {
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    
    humi = dht.readHumidity();
    temp = dht.readTemperature();
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("ºC ");
    Serial.print("Humidity: ");
    Serial.println(humi);

//    snprintf(msgDHT11, MSG_BUFFER_SIZE_DHT11, "%04d-%02d-%02d %02d:%02d:%02d",
//         timeinfo.tm_year+1900, timeinfo.tm_mon+1,
//         timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
//         timeinfo.tm_sec);
    snprintf(msgDHT11, MSG_BUFFER_SIZE_DHT11, "%04d-%02d-%02d %02d:%02d:%02d Temp:%.2fºC Hum:%.2f", 
        timeinfo.tm_year+1900, timeinfo.tm_mon+1,
        timeinfo.tm_mday, timeinfo.tm_hour, 
        timeinfo.tm_min, timeinfo.tm_sec, 
        temp, humi);
//    snprintf(msgDHT11, MSG_BUFFER_SIZE_DHT11, "Temperature: %.2fºC Humidity: %.2f", temp, humi);
  }

  client.publish("/topic/mqtt/sensorDHT11/out", msgDHT11);
  delay(59000);
}

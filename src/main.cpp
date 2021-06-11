#include <Arduino.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config-env.h"

#define DHTPIN 2
#define DHTTYPE DHT11
#define DEBUG

#define NTP_OFFSET   0      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

#define SENSOR_ID "OFFICE_#1"

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

float humidity = 0.0;
float temperature = 0.0;

void setup_wifi();
void setup_mqtt();
void reconnect();

void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  dht.begin();
  setup_wifi();
  setup_mqtt();

  timeClient.begin();

  #ifdef DEBUG
    Serial.println("Setup completed...");
  #endif
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  timeClient.update();

  //dht.computeHeatIndex

  humidity = dht.readHumidity();
  while (isnan(humidity)) {
    delay(100);
    humidity = dht.readHumidity(true);
  }

  temperature = dht.readTemperature();
  while (isnan(humidity)) {
    delay(100);
    temperature = dht.readTemperature(true);
  }

  #ifdef DEBUG
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.print(F("%  Temperature: "));
    Serial.print(temperature);
    Serial.println(F("°C"));
    Serial.print(F("MQTT Satus: "));
    Serial.println(client.state());
    Serial.print(F("Time: "));
    Serial.println(timeClient.getEpochTime());
  #endif

  char tempString[10];
  char json[64];
  dtostrf(temperature, 4, 2, tempString);
  sprintf(
    json, 
    "{\"sensorId\":\"%s\",\"sensor\":\"temperature\",\"value\":%s,\"time\":\"%lu\"}",
    SENSOR_ID,
    tempString,
    timeClient.getEpochTime()
  );

  client.publish(MQTT_TEMPERATURE_TOPIC, json);

  char humidityString[10];
  dtostrf(humidity, 4, 2, humidityString);
  sprintf(
    json,
    "{\"sensorId\":\"%s\",\"sensor\":\"humidity\",\"value\":%s,\"time\":\"%lu\"}",
    SENSOR_ID,
    humidityString,
    timeClient.getEpochTime()
  );
  client.publish(MQTT_HUMIDITY_TOPIC, json);

  delay(5000);
}

void setup_wifi() {
  delay(10);

  #ifdef DEBUG
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
  #endif

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG
      Serial.print(".");
    #endif
  }

  #ifdef DEBUG
    Serial.println("");
    Serial.println("WiFi connection established ");
    Serial.print("=> IP address: ");
    Serial.println(WiFi.localIP());
  #endif
}

void setup_mqtt() {
  client.setServer(MQTT_SERVER, 1883);
}

void reconnect() {
  //Boucle jusqu'à obtenur une reconnexion
  while (!client.connected()) {
    #ifdef DEBUG
      Serial.print("Connecting to MQTT Server...");
    #endif
    if (client.connect(MQTT_CLIENT_NAME)) {
      #ifdef DEBUG
        Serial.println("OK");
      #endif
    } else {
      #ifdef DEBUG
        Serial.print("KO, error : ");
        Serial.print(client.state());
      #endif
      delay(5000);
    }
  }
}
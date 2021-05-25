#include <Arduino.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "config-env.h"

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

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

  #ifdef DEBUG
    Serial.println("Setup completed...");
  #endif
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  float h = dht.readHumidity();
  float t = dht.readTemperature();

  #ifdef DEBUG
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C"));
    Serial.print(F("MQTT Satus: "));
    Serial.println(client.state());
  #endif

  char tempString[10];
  dtostrf(t, 4, 2, tempString);
  client.publish(MQTT_TEMPERATURE_TOPIC, tempString);

  char humidityString[10];
  dtostrf(h, 4, 2, humidityString);
  client.publish(MQTT_HUMIDITY_TOPIC, humidityString);

  delay(2000);
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
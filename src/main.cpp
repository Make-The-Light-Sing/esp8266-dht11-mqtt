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

void setup_wifi();
void setup_mqtt();
void reconnect();
void publishHumidity();
void publishTemperature();
void publishValue(const char* topic, const char* sensor, float value);

/**
 * Setup everything
 * @return void
 */
void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  dht.begin();
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);

  timeClient.begin();

  #ifdef DEBUG
    Serial.println("Setup completed...");
  #endif
}

/**
 * Main loop
 */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  timeClient.update();

  #ifdef DEBUG
    Serial.print(F("MQTT Satus: "));
    Serial.println(client.state());
    Serial.print(F("Time: "));
    Serial.println(timeClient.getEpochTime());
  #endif
  
  publishHumidity();
  publishTemperature();

  delay(5000);
}


/**
 * Read and publish temperature value
 */
void publishTemperature() {
  float temperature = 0.0;

  temperature = dht.readTemperature();
  while (isnan(temperature)) {
    delay(100);
    temperature = dht.readTemperature(true);
  }

  #ifdef DEBUG
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F("°C"));
  #endif

  publishValue(MQTT_TEMPERATURE_TOPIC, "temperature", temperature);
}

/**
 * Read and publish humidity value
 */
void publishHumidity() {
  float humidity = 0.0;

  humidity = dht.readHumidity();
  while (isnan(humidity)) {
    delay(100);
    humidity = dht.readHumidity(true);
  }

  #ifdef DEBUG
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F("%"));
  #endif

  publishValue(MQTT_HUMIDITY_TOPIC, "humidity", humidity);
}

/**
 * Publish a sensor value to a topic in MQTT
 */
void publishValue(const char* topic, const char* sensor, float value)
{
  char json[64];
  char valueString[10];
  dtostrf(value, 4, 2, valueString);
  sprintf(
    json,
    "{\"sensorId\":\"%s\",\"sensor\":\"%s\",\"value\":%s,\"time\":\"%lu\"}",
    SENSOR_ID,
    sensor,
    valueString,
    timeClient.getEpochTime()
  );
  client.publish(topic, json);
}

/**
 * Setup WiFi connection
 * @return void
 */
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

/**
 * Reconnect to MQTT Server when connection is lost
 * @return void
 */
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
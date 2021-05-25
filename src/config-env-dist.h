/**
 * Copy this file into 'config-env.h'
 * And edit the values with your own configuration
 */

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

/**
 * MQTT Configuration:
 */
#define MQTT_SERVER "127.0.0.1"
#define MQTT_CLIENT_NAME "my-dht11-sensor" // Client name used to identify ourself on MQTT server
#define MQTT_TEMPERATURE_TOPIC "sensor/temperature"
#define MQTT_HUMIDITY_TOPIC "sensor/humidity"

/**
 * Uncomment to activate debug information on Serial monitor
 */
// #define DEBUG
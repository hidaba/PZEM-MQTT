#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "sid_wifi"
#define WIFI_PASSWORD "password_wifi"
#define WIFI_HOSTNAME "ESP32C3_PZEM"

// MQTT Configuration
#define MQTT_BROKER "192.168.1.2"  // Change to your MQTT broker address
#define MQTT_PORT 1883                  // Default MQTT port
#define MQTT_USER "mosquitto"  // If authentication is needed
#define MQTT_PASSWORD "mosquitto"

// MQTT Topics for PZEM Data
#define MQTT_TOPIC_PZEM_1 "pzem32/energy/pzem1"
#define MQTT_TOPIC_PZEM_2 "pzem32/energy/pzem2"
#define MQTT_TOPIC_PZEM_3 "pzem32/energy/pzem3"
#define MQTT_TOPIC_CT "pzem32/energy/ct_current"
#define MQTT_TOPIC_A0 "pzem32/analog/A0"

#define MQTT_TOPIC_RELAY "pzem32/relay"


// MQTT Discovery Topics for Home Assistant
#define MQTT_DISCOVERY_PREFIX "homeassistant/"

#endif

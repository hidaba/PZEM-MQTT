#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "hidaba.home"
#define WIFI_PASSWORD "anolini in brodo!"
#define WIFI_HOSTNAME "ESP32C3_PZEM"

// MQTT Configuration
#define MQTT_BROKER "192.168.1.2"  // Change to your MQTT broker address
#define MQTT_PORT 1883                  // Default MQTT port
#define MQTT_USER "mosquitto"  // If authentication is needed
#define MQTT_PASSWORD "Sandoval23!"

// MQTT Topics for PZEM Data
#define MQTT_TOPIC_PZEM_1 "homeassistant/energy/pzem1"
#define MQTT_TOPIC_PZEM_2 "homeassistant/energy/pzem2"
#define MQTT_TOPIC_PZEM_3 "homeassistant/energy/pzem3"

// MQTT Topics for Relays
#define MQTT_TOPIC_RELAY "homeassistant/relay/"

// MQTT Discovery Topics for Home Assistant
#define MQTT_DISCOVERY_PREFIX "homeassistant"

#endif

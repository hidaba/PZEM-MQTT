#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "Your_WiFi_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"

// MQTT Configuration
#define MQTT_BROKER "mqtt.example.com"  // Change to your MQTT broker address
#define MQTT_PORT 1883                  // Default MQTT port
#define MQTT_USER "your_mqtt_username"  // If authentication is needed
#define MQTT_PASSWORD "your_mqtt_password"

// MQTT Topics for PZEM Data
#define MQTT_TOPIC_PZEM_1 "home/energy/pzem1"
#define MQTT_TOPIC_PZEM_2 "home/energy/pzem2"
#define MQTT_TOPIC_PZEM_3 "home/energy/pzem3"

// MQTT Topics for Relays
#define MQTT_TOPIC_RELAY "home/relay/"

// MQTT Discovery Topics for Home Assistant
#define MQTT_DISCOVERY_PREFIX "homeassistant"

#endif

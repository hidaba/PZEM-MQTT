#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>  // MQTT Library
#include "config.h"  // Include Wi-Fi & MQTT configuration

WiFiClient espClient;
PubSubClient mqttClient(espClient);

WebServer server(80);
WebSocketsServer webSocket(81);

// UART for Arduino Nano
#define RX_PIN 8
#define TX_PIN 7
HardwareSerial nanoSerial(1);

void connectWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
}


void callbackWebSocket(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        String message = String((char *)payload);
        message.trim();

        Serial.println("📩 Comando ricevuto: " + message);

        if (message.startsWith("RELAY")) {
            int relayNum = message.substring(6, 7).toInt();
            String state = message.substring(8);

            if (relayNum >= 1 && relayNum <= 4) {
                // 🔹 Inoltra il comando via seriale ad Arduino Nano
                String commandToArduino = "RELAY " + String(relayNum) + " " + state + "\n";
                nanoSerial.print(commandToArduino);
                Serial.println("📤 Inviato ad Arduino: " + commandToArduino);
            } else {
                Serial.println("⚠️ Numero relè non valido!");
            }
        }
    }
}


void sendPZEMDiscovery() {
    Serial.println("📡 Inviando autodiscovery MQTT per i PZEM...");

    struct PzemSensor {
        const char* name;
        const char* unique_id;
        const char* topic;
        const char* unit;
        const char* device_class;
        const char* state_class;
        const char* value_template;
    };

    const char* device_id = "pzem_monitor";

    PzemSensor pzemSensors[] = {
        {"PZEM 1 Voltage", "pzem_1_voltage", MQTT_TOPIC_PZEM_1, "V", "voltage", "measurement", "{{ value_json.voltage | float }}"},
        {"PZEM 1 Current", "pzem_1_current", MQTT_TOPIC_PZEM_1, "A", "current", "measurement", "{{ value_json.current | float }}"},
        {"PZEM 1 Power", "pzem_1_power", MQTT_TOPIC_PZEM_1, "W", "power", "measurement", "{{ value_json.power | float }}"},
        {"PZEM 1 Energy", "pzem_1_energy", MQTT_TOPIC_PZEM_1, "kWh", "energy", "total_increasing", "{{ value_json.energy | float }}"},
        {"PZEM 1 Frequency", "pzem_1_frequency", MQTT_TOPIC_PZEM_1, "Hz", "frequency", "measurement", "{{ value_json.frequency | float }}"},
        {"PZEM 1 Power Factor", "pzem_1_pf", MQTT_TOPIC_PZEM_1, "", "power_factor", "measurement", "{{ value_json.pf | float }}"},

        {"PZEM 2 Voltage", "pzem_2_voltage", MQTT_TOPIC_PZEM_2, "V", "voltage", "measurement", "{{ value_json.voltage | float }}"},
        {"PZEM 2 Current", "pzem_2_current", MQTT_TOPIC_PZEM_2, "A", "current", "measurement", "{{ value_json.current | float }}"},
        {"PZEM 2 Power", "pzem_2_power", MQTT_TOPIC_PZEM_2, "W", "power", "measurement", "{{ value_json.power | float }}"},
        {"PZEM 2 Energy", "pzem_2_energy", MQTT_TOPIC_PZEM_2, "kWh", "energy", "total_increasing", "{{ value_json.energy | float }}"},
        {"PZEM 2 Frequency", "pzem_2_frequency", MQTT_TOPIC_PZEM_2, "Hz", "frequency", "measurement", "{{ value_json.frequency | float }}"},
        {"PZEM 2 Power Factor", "pzem_2_pf", MQTT_TOPIC_PZEM_2, "", "power_factor", "measurement", "{{ value_json.pf | float }}"},

        {"PZEM 3 Voltage", "pzem_3_voltage", MQTT_TOPIC_PZEM_3, "V", "voltage", "measurement", "{{ value_json.voltage | float }}"},
        {"PZEM 3 Current", "pzem_3_current", MQTT_TOPIC_PZEM_3, "A", "current", "measurement", "{{ value_json.current | float }}"},
        {"PZEM 3 Power", "pzem_3_power", MQTT_TOPIC_PZEM_3, "W", "power", "measurement", "{{ value_json.power | float }}"},
        {"PZEM 3 Energy", "pzem_3_energy", MQTT_TOPIC_PZEM_3, "kWh", "energy", "total_increasing", "{{ value_json.energy | float }}"},
        {"PZEM 3 Frequency", "pzem_3_frequency", MQTT_TOPIC_PZEM_3, "Hz", "frequency", "measurement", "{{ value_json.frequency | float }}"},
        {"PZEM 3 Power Factor", "pzem_3_pf", MQTT_TOPIC_PZEM_3, "", "power_factor", "measurement", "{{ value_json.pf | float }}"}
    };

    for (auto& sensor : pzemSensors) {
        String discoveryTopic = String(MQTT_DISCOVERY_PREFIX) + "sensor/" + sensor.unique_id + "/config";

        String payload = "{";
        payload += "\"name\": \"" + String(sensor.name) + "\",";
        payload += "\"state_topic\": \"" + String(sensor.topic) + "\",";
        payload += "\"unique_id\": \"" + String(sensor.unique_id) + "\",";
        payload += "\"unit_of_measurement\": \"" + String(sensor.unit) + "\",";
        payload += "\"device_class\": \"" + String(sensor.device_class) + "\",";
        payload += "\"state_class\": \"" + String(sensor.state_class) + "\",";
        payload += "\"value_template\": \"" + String(sensor.value_template) + "\",";
        payload += "\"device\": {";
        payload += "   \"identifiers\": [\"" + String(device_id) + "\"],";
        payload += "   \"name\": \"ESP32 PZEM Monitor\",";
        payload += "   \"manufacturer\": \"Peacefair\",";
        payload += "   \"model\": \"PZEM-004T v3\",";
        payload += "   \"sw_version\": \"1.1\"";
        payload += "} }";

        bool result = mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
        
        if (result) {
            Serial.println("✅ Autodiscovery inviato per: " + String(sensor.name));
        } else {
            Serial.println("❌ ERRORE: Autodiscovery NON inviato per: " + String(sensor.name));
        }
    }
}





// 🔹 Autodiscovery per il CT Clamp
void sendCtClampDiscovery() {
    Serial.println("📡 Inviando autodiscovery MQTT per il CT Clamp...");

    struct CtSensor {
        const char* name;
        const char* unique_id;
        const char* unit;
        const char* device_class;
        const char* state_class;
        const char* value_template;
    };

    // ID del dispositivo per il CT Clamp
    const char* device_id = "ct_clamp_sensor";

    CtSensor ctSensors[] = {
        {"CT Voltage", "ct_voltage", "V", "voltage", "measurement", "{{ value_json.voltage }}"},
        {"CT Current", "ct_current", "A", "current", "measurement", "{{ value_json.current }}"},
        {"CT Power", "ct_power", "W", "power", "measurement", "{{ value_json.power }}"},
        {"CT Energy", "ct_energy", "kWh", "energy", "total_increasing", "{{ value_json.energy }}"}
    };

    for (auto& sensor : ctSensors) {
        String discoveryTopic = String(MQTT_DISCOVERY_PREFIX) + "sensor/" + sensor.unique_id + "/config";

        String payload = "{";
        payload += "\"name\": \"" + String(sensor.name) + "\",";
        payload += "\"state_topic\": \"" + String(MQTT_TOPIC_CT) + "\",";
        payload += "\"unique_id\": \"" + String(sensor.unique_id) + "\",";
        payload += "\"unit_of_measurement\": \"" + String(sensor.unit) + "\",";
        payload += "\"device_class\": \"" + String(sensor.device_class) + "\",";
        payload += "\"state_class\": \"" + String(sensor.state_class) + "\",";
        payload += "\"value_template\": \"" + String(sensor.value_template) + "\",";
        payload += "\"device\": {";
        payload += "   \"identifiers\": [\"" + String(device_id) + "\"],";
        payload += "   \"name\": \"CT Clamp Sensor\",";
        payload += "   \"manufacturer\": \"hidaba\",";
        payload += "   \"model\": \"SCT-013\",";
        payload += "   \"sw_version\": \"1.1\"";
        payload += "} }";

        mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
        Serial.println("✅ Autodiscovery inviato per: " + String(sensor.name));
    }
}


void connectMQTT() {
    if (!mqttClient.connected()) {
        Serial.println("📡 Connessione MQTT...");
        if (mqttClient.connect(WIFI_HOSTNAME, MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("✅ MQTT connesso!");
            sendPZEMDiscovery();
            sendCtClampDiscovery();
            Serial.println("📩 MQTT autodiscovery inviato con device info!");
        } else {
            Serial.print("❌ Errore MQTT, rc=");
            Serial.println(mqttClient.state());
            Serial.println(". Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    message.trim();

    Serial.println("📩 Messaggio ricevuto su MQTT: " + String(topic) + " → " + message);

    if (message == "ON" || message == "OFF") {
        int relayNum = -1;

        if (String(topic) == String(MQTT_TOPIC_RELAY) + "/1") relayNum = 1;
        if (String(topic) == String(MQTT_TOPIC_RELAY) + "/2") relayNum = 2;
        if (String(topic) == String(MQTT_TOPIC_RELAY) + "/3") relayNum = 3;
        if (String(topic) == String(MQTT_TOPIC_RELAY) + "/4") relayNum = 4;

        if (relayNum != -1) {
            // 📡 Invia il comando all'Arduino via seriale
            String command = "RELAY " + String(relayNum) + " " + message;
            nanoSerial.println(command);
            Serial.println("📤 Comando inviato all'Arduino: " + command);

            // 📤 Pubblica lo stato aggiornato su MQTT
            String stateTopic = String(MQTT_TOPIC_RELAY) + "/" + String(relayNum) + "/state";
            mqttClient.publish(stateTopic.c_str(), message.c_str(), true);
            Serial.println("📤 Stato relè pubblicato su MQTT: " + stateTopic + " → " + message);
        }
    }
}

void publishRelayState(int relayNum, String state) {
    String stateTopic = String(MQTT_TOPIC_RELAY) + "/" + String(relayNum) + "/state";
    mqttClient.publish(stateTopic.c_str(), state.c_str(), true);
    Serial.println("📤 Stato relè pubblicato su MQTT: " + stateTopic + " → " + state);
}


String pzemData = "Caricamento..."; // Dati ricevuti dai PZEM

// Funzione per formattare i dati nel formato corretto
String formattaDatiPZEM(String nome, float v, float a, float w, float kWh, float hz, float pf) {
    return nome + " " + String(v, 2) + "," + String(a, 2) + "," + String(w, 2) + "," + 
           String(kWh, 2) + "," + String(hz, 2) + "," + String(pf, 2);
}


void setup() {
    Serial.begin(115200);
    nanoSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

    // Connessione WiFi con hostname personalizzato
    WiFi.hostname(WIFI_HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connesso!");
    Serial.print("Hostname: "); Serial.println(WiFi.getHostname());


    // Configura il client MQTT con la porta corretta
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(callbackMQTT);
    
    mqttClient.setBufferSize(512);  // ⬅️ Aumentiamo la dimensione dei messaggi MQTT
    // Tenta la connessione iniziale a MQTT
    connectMQTT();

    // Setup Web Server
    server.on("/", handleRoot);
    server.begin();

    // Setup WebSocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // Setup OTA con hostname
    ArduinoOTA.setHostname(WIFI_HOSTNAME);
    ArduinoOTA.begin();

    Serial.println("ESP32 Web Server + OTA Pronto!");
}


void loop() {
    server.handleClient();
    webSocket.loop();
    ArduinoOTA.handle();

    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();

    // 🔹 Lettura dati dalla seriale di Arduino
    if (nanoSerial.available()) {
      
        String data = nanoSerial.readStringUntil('\n');
        data.trim();

        //Serial.println("📩 Dati ricevuti da Arduino: [" + data + "]");
        if (data.startsWith("RELAY")) {
            int relayNum = data.substring(6, 7).toInt();
            String state = data.substring(8);
            publishRelayState(relayNum, state);
            Serial.println(data);
        }

        if (data.startsWith("PZEM_")) {
            int pzemIndex = -1;
            if (data.startsWith("PZEM_1")) pzemIndex = 1;
            else if (data.startsWith("PZEM_2")) pzemIndex = 2;
            else if (data.startsWith("PZEM_3")) pzemIndex = 3;

            if (pzemIndex != -1) {
                // 🔹 Estrarre i dati dopo "PZEM_X"
                String values = data.substring(7);
                values.trim();

                float voltage, current, power, energy, frequency, pf;
                int parsed = sscanf(values.c_str(), "%f,%f,%f,%f,%f,%f", &voltage, &current, &power, &energy, &frequency, &pf);

                if (parsed != 6 || isnan(voltage)) {
                    Serial.println("⚠️ Errore nel parsing dei dati PZEM! Impostando a 0.");
                    voltage = current = power = energy = frequency = pf = 0.00;
                }

                // 🔹 Costruisce il messaggio JSON per MQTT e WebSocket
                String mqttPayload = "{";
                mqttPayload += "\"voltage\":" + String(voltage, 2) + ",";
                mqttPayload += "\"current\":" + String(current, 2) + ",";
                mqttPayload += "\"power\":" + String(power, 2) + ",";
                mqttPayload += "\"energy\":" + String(energy, 2) + ",";
                mqttPayload += "\"frequency\":" + String(frequency, 2) + ",";
                mqttPayload += "\"pf\":" + String(pf, 2) + "}";

                // 🔹 Pubblica su MQTT
                switch (pzemIndex) {
                    case 1: mqttClient.publish(MQTT_TOPIC_PZEM_1, mqttPayload.c_str(), true); break;
                    case 2: mqttClient.publish(MQTT_TOPIC_PZEM_2, mqttPayload.c_str(), true); break;
                    case 3: mqttClient.publish(MQTT_TOPIC_PZEM_3, mqttPayload.c_str(), true); break;
                }

                // 🔹 Invia i dati al WebSocket per aggiornare la pagina web
                String datiPZEM = "PZEM_" + String(pzemIndex) + " " + mqttPayload;
                webSocket.broadcastTXT(datiPZEM);
            }
        } 
        else if (data.startsWith("A0")) {
            // 🔹 Gestione dell'ingresso analogico
            int a0Value = data.substring(3).toInt();

            String mqttPayload = "{\"A0\":" + String(a0Value) + "}";

            // 🔹 Pubblica il valore su MQTT
            mqttClient.publish(MQTT_TOPIC_A0, mqttPayload.c_str(), true);

            // 🔹 Invia il valore al WebSocket per aggiornare la pagina web
            webSocket.broadcastTXT("A0 " + mqttPayload);
            
        }

    if (data.startsWith("CT")) {
      float tensione, correnteCT, potenzaAttiva, energiaTotale;
      int parsed = sscanf(data.substring(3).c_str(), "%f,%f,%f,%f", &tensione, &correnteCT, &potenzaAttiva, &energiaTotale);

      if (parsed != 4) {
          Serial.println("⚠️ Errore parsing CT Clamp!");
          tensione = correnteCT = potenzaAttiva = energiaTotale = 0.00;
      }

      // 📌 Costruisce JSON per MQTT e WebSocket
      String mqttPayload = "{";
      mqttPayload += "\"voltage\":" + String(tensione, 2) + ",";
      mqttPayload += "\"current\":" + String(correnteCT, 3) + ",";
      mqttPayload += "\"power\":" + String(potenzaAttiva, 1) + ",";
      mqttPayload += "\"energy\":" + String(energiaTotale, 3) + "}";

      // 📡 Pubblica su MQTT
      mqttClient.publish(MQTT_TOPIC_CT, mqttPayload.c_str(), true);

      // 🔹 Invia valore via WebSocket alla pagina web
      webSocket.broadcastTXT("CT " + mqttPayload);
      Serial.println("📤 CT Clamp Dati inviati: " + mqttPayload);
    }
    }
}


// Gestione richiesta pagina Web
void handleRoot() {
    server.send(200, "text/html", paginaHTML());
}

// WebSocket: ricezione comandi per i relè
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_TEXT) {
        String command = String((char*)payload);
        Serial.println("Comando ricevuto: " + command);
        nanoSerial.println(command);
        Serial.println("Comando inviato a arduino: " + command);
    }
}


String paginaHTML() {
    return R"rawliteral(
    <!DOCTYPE html>
    <html lang="it">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>ESP32 Monitor PZEM</title>
        <style>
            body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; }
            h2 { color: #333; }
            .card { background: white; padding: 20px; margin: 20px auto; width: 90%; max-width: 600px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.2); padding-bottom: 15px; }
            table { width: 100%; border-collapse: collapse; margin-top: 10px; }
            th, td { border: 1px solid #ddd; padding: 10px; text-align: center; }
            th { background-color: #4CAF50; color: white; }
            .switch { display: inline-block; width: 60px; height: 34px; position: relative; }
            .switch input { opacity: 0; width: 0; height: 0; }
            .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }
            .slider:before { position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }
            input:checked + .slider { background-color: #4CAF50; }
            input:checked + .slider:before { transform: translateX(26px); }
        </style>
    </head>
    <body>
        <h2>ESP32 Monitor PZEM</h2>
        <div class="card">
            <h3>Dati dai PZEM:</h3>
            <table>
                <tr>
                    <th>PZEM</th>
                    <th>Voltaggio (V)</th>
                    <th>Corrente (A)</th>
                    <th>Potenza (W)</th>
                    <th>Energia (kWh)</th>
                    <th>Frequenza (Hz)</th>
                    <th>PF</th>
                </tr>
                <tr id="PZEM_1">
                    <td>PZEM 1</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td>
                </tr>
                <tr id="PZEM_2">
                    <td>PZEM 2</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td>
                </tr>
                <tr id="PZEM_3">
                    <td>PZEM 3</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td>
                </tr>
            </table>
        </div>

        <div class="card">
            <h3>Ingresso Analogico A0:</h3>
            <p>Valore A0: <span id="A0">-</span></p>
        </div>

        <div class="card">
            <h3>Corrente Sensore CT Clamp</h3>
            <table>
                <tr><th>Voltaggio (V)</th><td id="CT_voltage">-</td></tr>
                <tr><th>Corrente (A)</th><td id="CT_current">-</td></tr>
                <tr><th>Potenza Attiva (W)</th><td id="CT_power">-</td></tr>
                <tr><th>Energia Totale (kWh)</th><td id="CT_energy">-</td></tr>
            </table>
        </div>

<script>
    var socket = new WebSocket("ws://" + window.location.hostname + ":81/");

    socket.onmessage = function(event) {
        console.log("📩 Dati ricevuti:", event.data);
        
        try {
            let message = event.data.split(" ");
            if (message.length < 2) return;

            let id = message[0];
            let jsonData = JSON.parse(message.slice(1).join(" "));

            if(id.startsWith("PZEM") && jsonData.voltage !== undefined) {
                document.querySelector(`#${id} td:nth-child(2)`).innerText = jsonData.voltage;
                document.querySelector(`#${id} td:nth-child(3)`).innerText = jsonData.current;
                document.querySelector(`#${id} td:nth-child(4)`).innerText = jsonData.power;
                document.querySelector(`#${id} td:nth-child(5)`).innerText = jsonData.energy;
                document.querySelector(`#${id} td:nth-child(6)`).innerText = jsonData.frequency;
                document.querySelector(`#${id} td:nth-child(7)`).innerText = jsonData.pf;
            } else if(id === "A0" && jsonData.A0 !== undefined) {
                document.getElementById("A0").innerText = jsonData.A0;
            }
              else if(id === "CT") {
              document.getElementById("CT_voltage").innerText = jsonData.voltage + " V";
              document.getElementById("CT_current").innerText = jsonData.current + " A";
              document.getElementById("CT_power").innerText = jsonData.power + " W";
              document.getElementById("CT_energy").innerText = jsonData.energy + " kWh";
            }

        } catch (error) {
            console.error("❌ Errore parsing:", error, event.data);
        }
    };

    function sendCommand(command) {
        socket.send(command);
    }
</script>

    </body>
    </html>
    )rawliteral";
}




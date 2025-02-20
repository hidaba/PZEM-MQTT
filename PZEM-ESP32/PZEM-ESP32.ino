#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>

// Imposta l'hostname personalizzato
const char* hostname = "ESP32C3_PZEM";

// Configurazione WiFi
const char* ssid = "hidaba.home";
const char* password = "anolini in brodo!";

// Definizione Web Server e WebSocket
WebServer server(80);
WebSocketsServer webSocket(81);

// Comunicazione con Arduino via UART
#define RX_PIN 8
#define TX_PIN 9
HardwareSerial nanoSerial(1);

String pzemData = "Caricamento..."; // Dati ricevuti dai PZEM

// Funzione per formattare i dati nel formato corretto
String formattaDatiPZEM(String nome, float v, float a, float w, float kWh, float hz, float pf) {
    return nome + " " + String(v, 2) + "," + String(a, 2) + "," + String(w, 2) + "," + 
           String(kWh, 2) + "," + String(hz, 2) + "," + String(pf, 2);
}

void setup() {
    Serial.begin(115200);
    nanoSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

    // Connessione WiFi con hostname personalizzato
    WiFi.hostname(hostname);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connesso!");
    Serial.print("Hostname: "); Serial.println(WiFi.getHostname());

    // Setup Web Server
    server.on("/", handleRoot);
    server.begin();

    // Setup WebSocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // Setup OTA con hostname
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.begin();

    Serial.println("ESP32 Web Server + OTA Pronto!");
}

void loop() {
    server.handleClient();
    webSocket.loop();
    ArduinoOTA.handle();

    if (nanoSerial.available()) {
        String data = nanoSerial.readStringUntil('\n');
        data.trim();  
        Serial.println("Dati ricevuti da Arduino: " + data);

        if (data.startsWith("PZEM_")) {
            int pzemIndex = -1;
            if (data.startsWith("PZEM_1")) pzemIndex = 1;
            else if (data.startsWith("PZEM_2")) pzemIndex = 2;
            else if (data.startsWith("PZEM_3")) pzemIndex = 3;

            if (pzemIndex != -1) {
                // Rimuoviamo i prefissi "V:", "A:", "W:", "kWh:", "Hz:", "PF:"
                data.replace("PZEM_1", "");
                data.replace("PZEM_2", "");
                data.replace("PZEM_3", "");
                data.replace("V:", "");
                data.replace("A:", "");
                data.replace("W:", "");
                data.replace("kWh:", "");
                data.replace("Hz:", "");
                data.replace("PF:", "");
                
                data.trim(); // Rimuove gli spazi rimasti

                float voltage = 0.0, current = 0.0, power = 0.0, energy = 0.0, frequency = 0.0, pf = 0.0;
                int parsed = sscanf(data.c_str(), "%f %f %f %f %f %f", &voltage, &current, &power, &energy, &frequency, &pf);

                // Se il parsing fallisce, imposta valori a "-"
                if (parsed != 6) {
                    Serial.println("⚠️ Errore nel parsing dei dati PZEM! Sostituzione con '-'.");
                    voltage = current = power = energy = frequency = pf = NAN;
                }

                // Controlla se i valori sono NaN e li sostituisce con "-"
                String datiPZEM = formattaDatiPZEM(
                    "PZEM_" + String(pzemIndex),
                    isnan(voltage) ? 0.00 : voltage,
                    isnan(current) ? 0.00 : current,
                    isnan(power) ? 0.00 : power,
                    isnan(energy) ? 0.00 : energy,
                    isnan(frequency) ? 0.00 : frequency,
                    isnan(pf) ? 0.00 : pf
                );

                webSocket.broadcastTXT(datiPZEM);
                Serial.println("✅ Dati inviati al WebSocket: " + datiPZEM);
            }
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
            <h3>Controllo Relè</h3>
            <label class="switch">
                <input type="checkbox" onclick="sendCommand('RELAY 1 ' + (this.checked ? 'ON' : 'OFF'))">
                <span class="slider"></span>
            </label> Relè 1 <br><br>
            <label class="switch">
                <input type="checkbox" onclick="sendCommand('RELAY 2 ' + (this.checked ? 'ON' : 'OFF'))">
                <span class="slider"></span>
            </label> Relè 2 <br><br>
            <label class="switch">
                <input type="checkbox" onclick="sendCommand('RELAY 3 ' + (this.checked ? 'ON' : 'OFF'))">
                <span class="slider"></span>
            </label> Relè 3 <br><br>
            <label class="switch">
                <input type="checkbox" onclick="sendCommand('RELAY 4 ' + (this.checked ? 'ON' : 'OFF'))">
                <span class="slider"></span>
            </label> Relè 4
        </div>
<script>
    var socket = new WebSocket("ws://" + window.location.hostname + ":81/");

    socket.onmessage = function(event) {
        console.log("Dati ricevuti:", event.data); // Debug per verificare i dati ricevuti

        var rows = event.data.split("\n"); // Divide i dati ricevuti per riga

        rows.forEach(row => {
            if (row.startsWith("PZEM_")) {
                var parts = row.split(" ");
                if (parts.length < 2) return; // Se i dati sono incompleti, ignora
                
                var id = parts[0].toUpperCase(); // es. "PZEM_1", "PZEM_2", "PZEM_3"
                var values = parts[1].split(",");

                if (values.length === 6) {
                    // Controlla se i valori sono numeri validi, altrimenti li sostituisce con "-"
                    for (let i = 0; i < values.length; i++) {
                        values[i] = isNaN(parseFloat(values[i])) ? "-" : values[i];
                    }

                    // Verifica se l'elemento esiste prima di aggiornarlo
                    let tableRow = document.getElementById(id);
                    if (tableRow) {
                        tableRow.innerHTML = `
                            <td>${id}</td>
                            <td>${values[0]}</td>
                            <td>${values[1]}</td>
                            <td>${values[2]}</td>
                            <td>${values[3]}</td>
                            <td>${values[4]}</td>
                            <td>${values[5]}</td>`;
                    } else {
                        console.error("Elemento non trovato:", id);
                    }
                }
            }
        });
    };

    function sendCommand(command) {
        console.log("Comando inviato:", command); // Debug per i relè
        socket.send(command);
    }
</script>


    </body>
    </html>
    )rawliteral";
}

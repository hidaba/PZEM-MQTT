#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>

// Definizione relè
#define RELAY1 2
#define RELAY2 3
#define RELAY3 4
#define RELAY4 5

// Definizione SoftwareSerial per PZEM-004T
#define PZEM_TX 6
#define PZEM_RX 7
SoftwareSerial pzemSerial(PZEM_RX, PZEM_TX);

// Definizione oggetti PZEM con indirizzi diversi
PZEM004Tv30 pzem1(pzemSerial, 0x01); // PZEM #1
PZEM004Tv30 pzem2(pzemSerial, 0x02); // PZEM #2
PZEM004Tv30 pzem3(pzemSerial, 0x03); // PZEM #3

// Definizione SoftwareSerial per ESP32-C3
SoftwareSerial espSerial(10, 11); // TX su 10, RX su 11

void setup() {

    Serial.begin(115200); // Debug
    espSerial.begin(115200); // Comunicazione con ESP32-C3
    pzemSerial.begin(9600);  // Comunicazione con PZEM-004T
    delay(10000);  // ⏳ Aspetta 10 secondi prima di partire
    
    // Setup relè
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);

    // Spegnimento iniziale relè
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, HIGH);
    digitalWrite(RELAY4, HIGH);

    Serial.println("Arduino Nano pronto!");
}

void loop() {
    // Lettura dati dai 3 PZEM-004T
    Serial.println("Lettura PZEM #1");
    inviaDatiPZEM(pzem1, "PZEM_1");

    Serial.println("Lettura PZEM #2");
    inviaDatiPZEM(pzem2, "PZEM_2");

    Serial.println("Lettura PZEM #3");
    inviaDatiPZEM(pzem3, "PZEM_3");

    // Lettura ingresso analogico
    int analogValue = analogRead(A0);
    espSerial.print("A0 ");
    espSerial.println(analogValue);

    // Controllo ricezione comandi da ESP32-C3
    if (espSerial.available()) {
        String command = espSerial.readStringUntil('\n');
        command.trim();

        if (command.startsWith("RELAY")) {
            int relayNum = command.substring(6, 7).toInt();
            String state = command.substring(8);
            int pin;

            switch (relayNum) {
                case 1: pin = RELAY1; break;
                case 2: pin = RELAY2; break;
                case 3: pin = RELAY3; break;
                case 4: pin = RELAY4; break;
                default: Serial.println("ERROR"); return;
            }

            digitalWrite(pin, (state == "ON") ? LOW : HIGH);
            Serial.println("OK");
        }
    }

    delay(2000);
}

void inviaDatiPZEM(PZEM004Tv30 &pzem, const char* nome) {
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    Serial.print(nome);
    Serial.print(" V: "); Serial.print(voltage);
    Serial.print(" A: "); Serial.print(current);
    Serial.print(" W: "); Serial.print(power);
    Serial.print(" kWh: "); Serial.print(energy);
    Serial.print(" Hz: "); Serial.print(frequency);
    Serial.print(" PF: "); Serial.println(pf);

    // Invio dati all'ESP32-C3 via Seriale
    espSerial.print(nome);
    espSerial.print(" ");
    espSerial.print(voltage); espSerial.print(",");
    espSerial.print(current); espSerial.print(",");
    espSerial.print(power); espSerial.print(",");
    espSerial.print(energy); espSerial.print(",");
    espSerial.print(frequency); espSerial.print(",");
    espSerial.println(pf);
}


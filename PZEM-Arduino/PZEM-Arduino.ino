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


// 📌 Parametri per il sensore a pinza CT
const int pinCT = A0;
const int numSamples = 100;     // Numero di letture per media
const float voltageRef = 5.0;   // Voltaggio di riferimento Arduino (5V)
const int adcResolution = 1024; // Risoluzione ADC Arduino
const float sensitivity = 0.066; // Sensibilità del sensore CT (66mV/A per SCT-013-030)

float energiaTotale = 0;  // Variabile per accumulare il consumo in kWh

float leggiCorrente() {
    float sum = 0;

    for (int i = 0; i < numSamples; i++) {
        int sensorValue = analogRead(pinCT);
        float voltage = (sensorValue * voltageRef) / adcResolution;
        float current = voltage / sensitivity;
        sum += current;
        delay(5);  // Piccolo ritardo per una lettura più stabile
    }

    return sum / numSamples; // Media delle letture
}

float calcolaVoltaggioMedio() {
    float v1 = pzem1.voltage();
    float v2 = pzem2.voltage();
    float v3 = pzem3.voltage();
    
    float somma = 0;
    int count = 0;

    if (!isnan(v1) && v1 > 10) { somma += v1; count++; }
    if (!isnan(v2) && v2 > 10) { somma += v2; count++; }
    if (!isnan(v3) && v3 > 10) { somma += v3; count++; }

    return (count > 0) ? somma / count : 230.0; // Se tutti falliscono, assume 230V
}


void setup() {

    Serial.begin(115200); // Debug
    espSerial.begin(115200); // Comunicazione con ESP32-C3
    pzemSerial.begin(9600);  // Comunicazione con PZEM-004T
    delay(5000);  // ⏳ Aspetta 5 secondi prima di partire
    
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
    Serial.print("A0 ");
    Serial.println(analogValue);
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


   // 📍 Calcola il voltaggio medio dai PZEM
    float tensioneMedia = calcolaVoltaggioMedio();

    Serial.print("🔍 Voltaggio medio calcolato: ");
    Serial.println(tensioneMedia);

    // 📍 Lettura corrente dal sensore CT Clamp
    float correnteCT = leggiCorrente();

    // 📌 Potenza Attiva (W) = Tensione media * Corrente
    float potenzaAttiva = tensioneMedia * correnteCT;

    // 📌 Energia Totale (kWh) = Potenza (W) * Tempo (h)
    energiaTotale += (potenzaAttiva * (2.0 / 3600.0)); // Ogni 2s → convertito in ore

    // 📩 Invio dati all'ESP32
    espSerial.print("CT ");
    espSerial.print(tensioneMedia, 2); espSerial.print(",");
    espSerial.print(correnteCT, 3); espSerial.print(",");
    espSerial.print(potenzaAttiva, 1); espSerial.print(",");
    espSerial.println(energiaTotale, 3);

    // Debug su seriale Arduino
    Serial.print("CT ");
    Serial.print(" V: "); Serial.print(tensioneMedia, 2);
    Serial.print(" A: "); Serial.print(correnteCT, 3);
    Serial.print(" W: "); Serial.print(potenzaAttiva, 1);
    Serial.print(" kWh: "); Serial.println(energiaTotale, 3);

    delay(2000);
}

void inviaDatiPZEM(PZEM004Tv30 &pzem, const char* nome) {
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();


if (isnan(voltage)) {
    voltage = 1.0; // 001.000 è semplicemente 1.0 in float
}


    //Serial.print(nome);
    //Serial.print(" V: "); Serial.print(voltage);
    //Serial.print(" A: "); Serial.print(current);
    //Serial.print(" W: "); Serial.print(power);
    //Serial.print(" kWh: "); Serial.print(energy);
    //Serial.print(" Hz: "); Serial.print(frequency);
    //Serial.print(" PF: "); Serial.println(pf);
    Serial.print(nome);
    Serial.print(" ");
    Serial.print(voltage); Serial.print(",");
    Serial.print(current); Serial.print(",");
    Serial.print(power); Serial.print(",");
    Serial.print(energy); Serial.print(",");
    Serial.print(frequency); Serial.print(",");
    Serial.println(pf);

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


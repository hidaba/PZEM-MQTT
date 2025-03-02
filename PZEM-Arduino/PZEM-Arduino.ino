#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>

// Definizione SoftwareSerial per PZEM-004T
#define PZEM_TX 6
#define PZEM_RX 7
SoftwareSerial pzemSerial(PZEM_RX, PZEM_TX);


// Definizione oggetti PZEM con indirizzi diversi
PZEM004Tv30 pzem1(pzemSerial, 0x01); // PZEM #1
PZEM004Tv30 pzem2(pzemSerial, 0x02); // PZEM #2
PZEM004Tv30 pzem3(pzemSerial, 0x03); // PZEM #3

// Definizione SoftwareSerial per ESP32-C3
//SoftwareSerial espSerial(10, 11); // TX su 10, RX su 11
SoftwareSerial espSerial(10, 11); // RX su 12, tx su 11

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

// Tempo di aggiornamento dello stato dei relè (ogni 5 secondi)
unsigned long lastStateUpdate = 0;


void setup() {
    Serial.begin(115200); // Debug
    espSerial.begin(9600); // Comunicazione con ESP32-C3
    pzemSerial.begin(9600);  // Comunicazione con PZEM-004T
    delay(1000);  // ⏳ Aspetta 1 secondi prima di partire
    Serial.println("Arduino Nano pronto!");
}

void loop() {
    // Lettura dati dai 3 PZEM-004T
    //Serial.println("Lettura PZEM #1");
    inviaDatiPZEM(pzem1, "PZEM_1");
    delay(100);
    //Serial.println("Lettura PZEM #2");
    inviaDatiPZEM(pzem2, "PZEM_2");
    delay(100);
    //Serial.println("Lettura PZEM #3");
    inviaDatiPZEM(pzem3, "PZEM_3");
    delay(100);
    // Lettura ingresso analogico
    int analogValue = analogRead(A0);
    espSerial.print("A0 ");
    espSerial.println(analogValue);
    delay(100);

   // 📍 Calcola il voltaggio medio dai PZEM
    float tensioneMedia = calcolaVoltaggioMedio();

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


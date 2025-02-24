#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>

#define PZEM_RX 6  // Pin RX su Arduino
#define PZEM_TX 7  // Pin TX su Arduino

SoftwareSerial pzemSerial(PZEM_RX, PZEM_TX);
PZEM004Tv30 pzem(pzemSerial);

void setup() {
    Serial.begin(115200);  // Debug monitor
    pzemSerial.begin(9600); // Comunicazione con PZEM

    Serial.println("⚡ PZEM Address Changer");
    
    uint8_t oldAddr = pzem.readAddress();
    Serial.print("🔹 Current PZEM Address: ");
    Serial.println(oldAddr, HEX);

    uint8_t newAddr = 2;  // Nuovo indirizzo
    
    
    
    
    
     da assegnare

    if (pzem.setAddress(newAddr)) {
        Serial.print("✅ Address changed successfully to: ");
        Serial.println(newAddr);
    } else {
        Serial.println("❌ Failed to change PZEM Address!");
    }

    Serial.println("🚀 Restart Arduino and use the new address.");
}

void loop() {
    // Nulla da fare dopo il cambio, riavvia Arduino per tornare all'uso normale.
}
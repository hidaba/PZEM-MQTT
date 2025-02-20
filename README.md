# ESP32-C3 + Arduino Nano + PZEM-004T: Energy Monitoring & Relay Control

This project uses an **ESP32-C3** and an **Arduino Nano** to monitor energy consumption using multiple **PZEM-004T** sensors and control four relays. The data is transmitted via **UART** from the Arduino Nano to the ESP32-C3, which then serves a **web interface** for real-time monitoring.

---

## 📌 **Features**
✅ **Real-time energy monitoring** with up to **3 PZEM-004T** sensors.  
✅ **Web interface** to display voltage, current, power, energy, frequency, and power factor.  
✅ **WebSocket communication** for fast data updates.  
✅ **Relay control** (4 relays) via the web interface.  
✅ **OTA updates** to update the ESP32-C3 firmware wirelessly.  
✅ **Automatic PZEM detection** (if a PZEM is not connected, it won't cause errors).

---

## 📌 **Hardware Used**
### **1️⃣ Microcontrollers**
- **ESP32-C3 Super Mini** (acts as the web server and WebSocket interface)
- **Arduino Nano** (handles PZEM communication and relay control)

### **2️⃣ Energy Monitoring**
- **PZEM-004T v3.0** (up to 3 units)

### **3️⃣ Relay Control**
- **4-channel relay module** (for controlling external devices)

### **4️⃣ Other Components**
- **Logic Level Shifter (5V ↔ 3.3V)** (for safe UART communication)
- **Jumper wires**
- **5V Power Supply** for the Arduino Nano and relays

---

## 📌 **Wiring Diagram**
### **🔹 Connections Between Arduino Nano and ESP32-C3**
| Arduino Nano | ESP32-C3 | Function |
|-------------|----------|----------|
| **TX (Pin 8)** | **RX (GPIO6)** | UART Communication |
| **RX (Pin 9)** | **TX (GPIO7)** | UART Communication |
| **GND** | **GND** | Common Ground |

### **🔹 Connections Between Arduino Nano and PZEM-004T**
| Arduino Nano | PZEM-004T | Function |
|-------------|-----------|----------|
| **6 (SoftwareSerial TX)** | **RX (PZEM)** | Serial Communication |
| **7 (SoftwareSerial RX)** | **TX (PZEM)** | Serial Communication |
| **GND** | **GND** | Common Ground |

### **🔹 Connections for Relays**
| Arduino Nano | Relay Module | Function |
|-------------|--------------|----------|
| **2** | **IN1** | Relay 1 |
| **3** | **IN2** | Relay 2 |
| **4** | **IN3** | Relay 3 |
| **5** | **IN4** | Relay 4 |
| **GND** | **GND** | Common Ground |

---

## 📌 **Software Setup**
### **1️⃣ Required Libraries**
Make sure you install the following libraries in the **Arduino IDE**:
- **ESP32**
- **WiFi.h** (for ESP32 Wi-Fi)
- **WebServer.h** (for hosting the web interface)
- **WebSocketsServer.h** (for real-time updates)
- **SoftwareSerial.h** (for Arduino Nano serial communication)
- **PZEM004Tv30.h** (for PZEM-004T communication)
- **ArduinoOTA.h** (for over-the-air updates)

### **2️⃣ Flashing the Firmware**
- Flash the **Arduino Nano** first (handles PZEM and relay control).
- Flash the **ESP32-C3** next (handles web interface and communication).
- Ensure the **baud rate is set to 115200** for correct UART communication.

---

## 📌 **How It Works**
1️⃣ **Arduino Nano reads data from PZEM-004T sensors** and sends it over UART.  
2️⃣ **ESP32-C3 receives data, processes it, and updates the web interface**.  
3️⃣ The **web interface displays real-time readings** from the PZEM-004T.  
4️⃣ Users can **control relays from the web interface** via WebSockets.  
5️⃣ If a PZEM-004T is missing, it is **automatically ignored**.  

---

## 📌 **Web Interface Preview**
The web interface allows users to:
- View **real-time energy data**.
- Toggle **relays ON/OFF**.
- Perform **OTA firmware updates**.

---

## 📌 **Future Improvements**
🚀 Possible future features:
- **MQTT integration** for smart home automation.
- **Data logging** to an SD card or cloud database.
- **Graphing system** to visualize energy consumption trends.

---

## 📌 **License**
This project is open-source. Feel free to modify and improve it!

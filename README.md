This project implements an RFID-based attendance or identification system using an ESP32 microcontroller and the MFRC522 RFID reader. When an RFID card is scanned, the ESP32 reads the unique UID of the card and sends it to a Google Apps Script Web API, which logs the UID into Google Sheets.

The system connects to WiFi, reads RFID card data, and transmits the UID to a cloud endpoint using an HTTP GET request.

This project demonstrates integration between embedded systems, IoT communication, and cloud data logging.

Hardware Components

ESP32 Development Board

MFRC522 RFID Reader Module

RFID Cards / Tags

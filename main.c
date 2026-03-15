#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

/* WiFi Credentials */
const char* ssid = "///";
const char* password = "////";

/* Server URL */
String serverName = "https://script.google.com/macros/s/AKfycbwgmJ-iL5MdkHIaltfWqp707KeKdWXp-94Yuv_T2T-G-ax_haZHZilSyca8EEPe7ybU/exec?uid=";

void setup() {

  Serial.begin(115200);
  delay(1000);

  /* SPI + RFID initialization */
  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  Serial.println("RFID Reader Initialized");

  /* Default key */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  /* WiFi Connection */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Scan your RFID card...");
}

void loop() {

  /* Check for card */
  if (!rfid.PICC_IsNewCardPresent()) return;

  if (!rfid.PICC_ReadCardSerial()) return;

  String uidString = "";

  Serial.print("Card UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) uidString += "0";

    uidString += String(rfid.uid.uidByte[i], HEX);

    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();

  sendData(uidString);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(2000);
}

/* Function to send UID to server */

void sendData(String uid) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String serverPath = serverName + uid;

    Serial.print("Sending to server: ");
    Serial.println(serverPath);

    http.begin(serverPath);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String payload = http.getString();
      Serial.println(payload);
    }
    else {

      Serial.print("Error sending request: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
  else {

    Serial.println("WiFi Disconnected");
  }
}

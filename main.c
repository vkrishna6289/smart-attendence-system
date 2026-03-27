#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 22
#define LED_PIN 4
#define BUZZER_PIN 21

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

const char* ssid = "kri";
const char* password = "12345678";

String serverName = "https://script.google.com/macros/s/AKfycbz3VB2VAr6962a7DoJwRQvB258dhU8Tm8GPl_XjSfOCVH1hZOhjPjv9sZlon17_pNBR/exec?uid=";

String pendingUID = "";
bool newScan = false;

/* Runs on Core 0 — handles blink */
void blinkTask(void* pvParameters) {
  while (true) {
    if (newScan) {
      for (int i = 0; i < 10; i++) {       // 10 x 200ms = 2 sec
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      newScan = false;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

/* Runs on Core 0 — handles HTTP */
void sendTask(void* pvParameters) {
  while (true) {
    if (pendingUID != "") {
      String uid = pendingUID;
      pendingUID = "";

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
          Serial.println(http.getString());
        } else {
          Serial.print("Error: ");
          Serial.println(httpResponseCode);
        }

        http.end();
      } else {
        Serial.println("WiFi Disconnected");
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();
  Serial.println("RFID Reader Initialized");

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  /* Launch blink and send tasks on Core 0 */
  xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(sendTask,  "SendTask",  8192, NULL, 1, NULL, 0);

  Serial.println("Scan your RFID card...");
}

void loop() {
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

  /* Trigger blink and HTTP simultaneously */
  pendingUID = uidString;
  newScan = true;

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(2000);
}

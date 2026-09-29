#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

#define LED_BUILTIN 2
// Wifi and Password
const char* ssid = "Technotouch";
const char* password = "5135Innova";

const char* version_url = "http://192.168.0.118:8080/version.txt";
const char* firmware_url = "http://192.168.0.118:8080/firmware.bin";

const int CURRENT_VERSION = 1;

unsigned long lastCheck = 0;
const unsigned long OTA_INTERVAL = 3600000;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  checkForUpdate();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);

  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  if (millis() - lastCheck >= OTA_INTERVAL) {
    lastCheck = millis();
    checkForUpdate();
  }
}

void checkForUpdate() {
  HTTPClient http;
  WiFiClient client;

  Serial.println("Checking for firmware update...");

  if (!http.begin(client, version_url)) {
    Serial.println("Version HTTP begin failed");
    return;
  }

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String serverVersion = http.getString();
    serverVersion.trim();

    int newVersion = serverVersion.toInt();

    Serial.println("Current version: " + String(CURRENT_VERSION));
    Serial.println("Server version: " + String(newVersion));

    if (newVersion > CURRENT_VERSION) {
      http.end();
      performOTA();
      return;
    }

    Serial.println("No update available");
  } else {
    Serial.println("Version check failed");
  }

  http.end();
}

void performOTA() {
  WiFiClient client;
  HTTPClient http;

  Serial.println("Starting OTA...");
  Serial.println(firmware_url);

  if (!http.begin(client, firmware_url)) {
    Serial.println("HTTP begin failed");
    return;
  }

  int httpCode = http.GET();

  Serial.println("HTTP Code: " + String(httpCode));

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();

    Serial.println("Firmware Size: " + String(contentLength));

    if (Update.begin(contentLength)) {
      size_t written = Update.writeStream(http.getStream());

      Serial.println("Written: " + String(written));

      if (written == contentLength) {
        if (Update.end()) {
          if (Update.isFinished()) {
            Serial.println("OTA SUCCESS");
            delay(2000);
            ESP.restart();
          }
        }
      }
    }
  }

  http.end();
}

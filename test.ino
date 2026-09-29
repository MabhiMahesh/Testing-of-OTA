#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

#define LED_BUILTIN 2

const char* ssid = "Technotouch";
const char* password = "5135Innova";

const char* version_url = "http://192.168.0.118:8080/version.txt";
const char* firmware_url = "http://192.168.0.118:8080/firmware.bin";

const int CURRENT_VERSION = 3;

void checkOTA() {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, version_url);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String serverVersion = http.getString();
    serverVersion.trim();

    int newVersion = serverVersion.toInt();

    if (newVersion > CURRENT_VERSION) {
      http.end();

      http.begin(client, firmware_url);

      httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();

        if (contentLength > 0 && Update.begin(contentLength)) {
          WiFiClient* stream = http.getStreamPtr();

          size_t written = Update.writeStream(*stream);

          if (written == contentLength) {
            if (Update.end() && Update.isFinished()) {
              ESP.restart();
            }
          }
        }
      }

      http.end();
    }
  }

  http.end();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  checkOTA();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);

  digitalWrite(LED_BUILTIN, LOW);
  delay(300);
}
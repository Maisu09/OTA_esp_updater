#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// sudo chmod a+rw /dev/ttyUSB0

const char *url = "https://ota-esp32-updater.s3.eu-north-1.amazonaws.com/firmware.bin";
const char *ssid = "mihai";
const char *pass = "12345678";

#include <WiFi.h>


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  initWiFi();

  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  Serial.println("Connected");

  // Begin OTA
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url); // Must be HTTPS!
    int httpCode = http.GET();

    if (httpCode == 200) {
      int contentLength = http.getSize();
      bool canBegin = Update.begin(contentLength);
      if (canBegin) {
        WiFiClient *client = http.getStreamPtr();
        size_t written = Update.writeStream(*client);
        if (written == contentLength) {
          Serial.println("Written : " + String(written) + " bytes successfully");
        } else {
          Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
        }

        if (Update.end()) {
          Serial.println("OTA done!");
          if (Update.isFinished()) {
            Serial.println("Restarting ESP");
            ESP.restart();
          } else {
            Serial.println("OTA not finished. Something went wrong.");
          }
        } else {
          Serial.println("Error Occurred. Error #: " + String(Update.getError()));
        }

      } else {
        Serial.println("Not enough space to begin OTA");
      }
    } else {
      Serial.println("Failed to fetch firmware. HTTP code: " + String(httpCode));
    }

    http.end();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}

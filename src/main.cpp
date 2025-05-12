#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "Resources/connectivity.h"
#include "Resources/public_key.h"
#include <fstream>


// const char* url = ;
const char* url = "https://ota-esp32-updater.s3.eu-north-1.amazonaws.com/firmware_v1.0.ino.bin?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=ASIASBGQLUEXHI3M3TFR%2F20250512%2Feu-north-1%2Fs3%2Faws4_request&X-Amz-Date=20250512T151909Z&X-Amz-Expires=604800&X-Amz-SignedHeaders=host&X-Amz-Security-Token=IQoJb3JpZ2luX2VjEC8aCmV1LW5vcnRoLTEiRjBEAiAfMg3QERFkEtNyRaRzCWs2FXeIfFjBNbthz8Lrw%2BxTKgIgZ1od05lfw7aStAgh1wBwiiR%2Fm2b1nrjVd4BfSeL%2BHxIq7AII2P%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FARAAGgwxNDAwMjM0MDY4OTQiDJwxPM6gI%2F%2BGct72CyrAAt7eu6hs63Xdh0DW1YGTHh%2FsnHVLKNvaOp7VCYjjK%2FDd9s6U%2BNLHE%2BlKjPIeVYvqSq31O5nLA5cXOznMLFGvsthqj2W3j5juDKFdzPDX9imGd7%2BPjDPwgLCFmXSyuILJhIRFUECMZedwFi1IUsisLxqWL9nCOou7mM5Zb5VFkjJjwTb0m6T8PvpF%2FWrUVoUPTH7EO1ykIBHMtmAcBQ%2BvWkgYZDgCmvEv6tYEyOYDvu%2F6KED8cypXDaQxfYOj2orGZ7u5C4I7Y0BF6coG1uk%2BmX38s8XL9k%2BWcnBt%2FVRc7zmp4f2S%2FpwJ3BsZ1%2FzaPkauM6IzjEKuTjhXhsHIQCt7srI1hpMqX5HNIFvFEbH1LBeUPpgMoQmQsq4Ok%2BRUFbz90z%2FTtu6%2BFBcPDl7HloqrAlbrnYidJnIXedSuk94Roe9PMJqaiMEGOq4CNAeRXBXtVrtmCUtYQiaKYlQoQV%2BgHZ87LfUrWpf%2BuRpl%2F4OGa96pDGRm8mG01Tth0onGNR%2BPl%2FZBQI7Gmwyk6izDVzlKB6f%2B8BscL1WhmEi%2FdqVlqfkpNOhwKzOzFRLRi8pvFX14%2FW7Fn94WQ09J2UNO3KB0UZ34e2S8%2Fz83WAahCm%2BiIdbq3iYIvQPMHPhORLR1lNBcav0PB064jbiEpsW74Vv3a5VbAUs%2FHBatq5nM1U%2Fa1kbgK8vni22937qkVTPSXrcc5iLMMLHqrLzUxRXh9OenpBDpJb8Qw%2BulJHyAWLV2SS0wTnXinfDWiG18iQCAtKES%2BqLK2RXIX0JHyLHhGlN2qZNNwcpffsrkSHQUQupOrHZnsELfBtgsfmVmMMYTpRVzroHpBEz%2BHyg%3D&X-Amz-Signature=a9d0aea906bbdba3f7e444b060894f4aa65d94ce0c1d33ec7c607d14aef4c0c0";
// const char* root_ca = CERTIFICATE;
const char *ssid = SSID;
const char *pass = PASS;


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

void updater(char url){
  // updates the esp with the specific bin file.
  ;
}


void setup() {
  Serial.begin(115200);
  initWiFi();
  
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  Serial.println("Connected");
  Serial.print("URL: ");
  Serial.println(url);

  // Begin OTA
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    WiFiClientSecure client;

    // client.setCACert(root_ca);
    client.setInsecure();

    https.begin(client, url); // HTTPS!
    int httpCode = https.GET();

    if (httpCode == 200) {
      int contentLength = https.getSize();
      bool canBegin = Update.begin(contentLength);
      if (canBegin) {
        // start_update(&https, contentLength);
        WiFiClient *client = https.getStreamPtr();
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
      Serial.println("Failed to fetch firmware. HTTP code: " + String(httpCode) + url);
    }

    https.end();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}

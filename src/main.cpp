#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "Resources/connectivity.h"
#include "Resources/certificate.h"
#include <fstream>
#include <ArduinoJson.h>

// sudo chmod a+rw /dev/ttyUSB0
// https://ota-esp32-updater.s3.eu-north-1.amazonaws.com/file.ino.bin?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=ASIASBGQLUEXIXCSCD4T%2F20250412%2Feu-north-1%2Fs3%2Faws4_request&X-Amz-Date=20250412T181602Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Security-Token=IQoJb3JpZ2luX2VjEGIaCmV1LW5vcnRoLTEiRjBEAiBHMYaXD7a1i3JwGc%2BWA2Z0dNMPRVK4JfProEbkKtcg3wIgTgzsRg4hTqnJHc52A8y9CbsexZz1QUUmdqyjYKHX7F8q%2BwII2%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FARAAGgwxNDAwMjM0MDY4OTQiDCkcOyUYqsgvLb7T0yrPAiX76G85I2940KBYa3yF%2BDAaei45lv%2BO1vh7yATD6YndQBgNjQ%2FG94%2FiImYUwW1aov4b3RDU5t3Ht%2F8%2BqRrYK54Q0w1Of%2FQ5DKtl160yk56zIs7C6sMfzPQ0w0HAByIB3mYF1qNg38aK9ZOI55bh2Qjg1KpjQuLP3dn%2FzRIl3tbSgFuhJOhSX%2FpkXdNsJEkgq2xkH%2Bs3uaF3ElN%2Bu4sGJ%2BdGAqgr%2FAWiWnbLF17oWyTtpQVU4V4DqqvZV3pAA%2BgG1WE6YPWCUxm3lRW0KXfINQtAI17I4%2F9VXi2H29EADK%2FKPigTpCYn5GnzRPZsdfYjSzsIrmjnBUOw1wzCSi%2BdmhzpMZk1pR%2BN1OXBAJF6K3314WQdYij9W5eEZ4pmI9YYfRouhjvhp1TbQsYnjLB0YZGQzO2nB6XMEnQmTceokco2RwllmV%2BPT53b4ezFkFCxMOaT6L8GOrQCT2AIxzp7vPxmqjePY3DmP9imXfthMkOjU4D5b8UPuRUgacp5nrWNSAuMh3zLSi4QMdAM5xtICvm6krpnh9G%2FM8f7Gg1sp%2BsKIVe2vzSw4z1lX51jJdURNMsh2GIwCwkOztEntPwlQF1X%2FY5EUjCqOzUaPbSpqRFeIYVo%2FhAiPKCe5A1CPYFh0mXmpw0w%2F8dLn0tBdLWTxi2iPijWEtn%2FCCSddo2uslWKbf1wy%2B6VyNorA8LZt4jp32H4bsU%2B5XarltxUmIRNca9dfFugwQP2%2B319OznPlWp77aTnqw6b6JueH3q40xNegfKfgyWF1R97v%2F2FxCZs%2F1tQoEUJjvsvwCET%2BO4Ukd7nRXU8sOp9YK9dIfBK5N8QORx8FexuvFzlAyswZgr6A5UWjnKedmuowK2GjC4%3D&X-Amz-Signature=b833055392b444197ec39cc931cc94db8d52542ae1d383d24f8a9cd6b974cafa
const char* url = "https://ota-esp32-updater.s3.eu-north-1.amazonaws.com/file.ino.bin?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=ASIASBGQLUEXIXCSCD4T%2F20250412%2Feu-north-1%2Fs3%2Faws4_request&X-Amz-Date=20250412T181602Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Security-Token=IQoJb3JpZ2luX2VjEGIaCmV1LW5vcnRoLTEiRjBEAiBHMYaXD7a1i3JwGc%2BWA2Z0dNMPRVK4JfProEbkKtcg3wIgTgzsRg4hTqnJHc52A8y9CbsexZz1QUUmdqyjYKHX7F8q%2BwII2%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FARAAGgwxNDAwMjM0MDY4OTQiDCkcOyUYqsgvLb7T0yrPAiX76G85I2940KBYa3yF%2BDAaei45lv%2BO1vh7yATD6YndQBgNjQ%2FG94%2FiImYUwW1aov4b3RDU5t3Ht%2F8%2BqRrYK54Q0w1Of%2FQ5DKtl160yk56zIs7C6sMfzPQ0w0HAByIB3mYF1qNg38aK9ZOI55bh2Qjg1KpjQuLP3dn%2FzRIl3tbSgFuhJOhSX%2FpkXdNsJEkgq2xkH%2Bs3uaF3ElN%2Bu4sGJ%2BdGAqgr%2FAWiWnbLF17oWyTtpQVU4V4DqqvZV3pAA%2BgG1WE6YPWCUxm3lRW0KXfINQtAI17I4%2F9VXi2H29EADK%2FKPigTpCYn5GnzRPZsdfYjSzsIrmjnBUOw1wzCSi%2BdmhzpMZk1pR%2BN1OXBAJF6K3314WQdYij9W5eEZ4pmI9YYfRouhjvhp1TbQsYnjLB0YZGQzO2nB6XMEnQmTceokco2RwllmV%2BPT53b4ezFkFCxMOaT6L8GOrQCT2AIxzp7vPxmqjePY3DmP9imXfthMkOjU4D5b8UPuRUgacp5nrWNSAuMh3zLSi4QMdAM5xtICvm6krpnh9G%2FM8f7Gg1sp%2BsKIVe2vzSw4z1lX51jJdURNMsh2GIwCwkOztEntPwlQF1X%2FY5EUjCqOzUaPbSpqRFeIYVo%2FhAiPKCe5A1CPYFh0mXmpw0w%2F8dLn0tBdLWTxi2iPijWEtn%2FCCSddo2uslWKbf1wy%2B6VyNorA8LZt4jp32H4bsU%2B5XarltxUmIRNca9dfFugwQP2%2B319OznPlWp77aTnqw6b6JueH3q40xNegfKfgyWF1R97v%2F2FxCZs%2F1tQoEUJjvsvwCET%2BO4Ukd7nRXU8sOp9YK9dIfBK5N8QORx8FexuvFzlAyswZgr6A5UWjnKedmuowK2GjC4%3D&X-Amz-Signature=b833055392b444197ec39cc931cc94db8d52542ae1d383d24f8a9cd6b974cafa";  // The newly generated URL (temporary ) aws s3 presign s3://bucket/file_in_bucket --expires-in time
// const char* root_ca = \
// "-----BEGIN CERTIFICATE-----\n" \
// "MIID0zCCArugAwIBAgIQC1RBc3sZbxLNPqT79rLJfzANBgkqhkiG9w0BAQsFADBh\n" \
// "MQswCQYDVQQGEwJVUzETMBEGA1UEChMKRGlnaUNlcnQgSW5jMRkwFwYDVQQLExBE\n" \
// "aWdpQ2VydCBHbG9iYWwgUm9vdDEiMCAGA1UEAxMZRGlnaUNlcnQgR2xvYmFsIFJv\n" \
// "b3QgRzIwHhcNMTQwNzE2MDAwMDAwWhcNMzQwNzE2MDAwMDAwWjBhMQswCQYDVQQG\n" \
// "EwJVUzETMBEGA1UEChMKRGlnaUNlcnQgSW5jMRkwFwYDVQQLExBEaWdpQ2VydCBH\n" \
// "bG9iYWwgUm9vdDEiMCAGA1UEAxMZRGlnaUNlcnQgR2xvYmFsIFJvb3QgRzIwggEi\n" \
// "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC+F+jsvikKy/65LWEx/TMkCDI/\n" \
// "9V4FqkSyG4Wh7hSJLQCcdXQgtVtoUU0Sr0ko09wU5/Y51j8D8G89rcvjF+rwk+Wy\n" \
// "4iFANkE+VYFUn/BBc7n7hK5Dr1VxKHgfNstH47qmYtbe4zJUaJ5pIM1dTzZ9Yb05\n" \
// "sl+f/2YSmWUN+e1L8nHrbt2xdQKBgQDPZ9FHOIczEya5zL5BqSMz2FvYldmRz+Hi\n" \
// "1fnRNHox0lrJNaPp3L0jHv7K/Ki+gN0gfwIDZ41XjmBqsINEn1T3pB2kl+eR2nTS\n" \
// "KvSp3RSLbxsSe0PXypPDCdI7iHaWy/8H7MkZHlFYzOeVHD9OtO0Z+VnkgY9JK+Et\n" \
// "sswDf44TWvIbAgMBAAGjQjBAMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFNLE\n" \
// "sNKR0BHLH0vY2nDQIGb6+JT2MA4GA1UdDwEB/wQEAwIBhjANBgkqhkiG9w0BAQsF\n" \
// "AAOCAgEAKmKk3N0CpjGhQ7v9YgPKzW0fvkYFFQFhjQ7pG2YY5F7+5G1kPGVh+7cG\n" \
// "zF5ZPZ3zU3YMbIskpQJ0k7ulU+zRSI6YIRFQEYef39Q4+1J9O/s9myo7u0PzV73j\n" \
// "0kzVfs8AjtKoaVHgMHqYjgJv1nAtv8vbLpynepTcGAfRWRBf6a4x+H7duCCJykPY\n" \
// "b2tZs+iaGG+XjF7v5w2m9YB27L3+F2S3JUzokfbM6UQ2o+Z7WvY8nX1Ti9VZhGPP\n" \
// "UHd45yWx/Vhf5WJS1xuL5CbnW/jcZYVvzz0rMKIct2twT4YvSSGBfEUCdG7uA60l\n" \
// "rY/Nx+yOnQ6MStqYxgPPWDVOwbyFjvFZqZJcJbEf/nI0G2ciJS1YQHzCJY+eyLCF\n" \
// "OtQUiTwXwfJS4zVNGkHZMr4Xhzrbt9O/N2v7l/3vItMGKx/Jr0VYLQNdjZh3sDUt\n" \
// "F3IvYNOyU5S3ac4UgE7BGH/u4wT7iwbFcudPBAlnS9M1q9f+kN2VDxS9fcmuk9qu\n" \
// "le+vPZJGeZbxr3ix6aEYw31KsFDP0XYB9NLZP3st8uH0tDdmB2TRsrUCu1ZkM1os\n" \
// "z9z9ApkAI9GLbKucg+BQkCpNVNzx6Tn6fEjx24g4Xd+pfZIlAbCjhBn6zDO8tTLC\n" \
// "BUg+P/6BlfcB/qQrxS8G0rrFJGdfQDLqhqu80V6LnmhkymA=\n" \
// "-----END CERTIFICATE-----\n";
const char* root_ca = CERTIFICATE;
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

char read_json() {
  FILE *file;
  file = fopen("src/Resources/firmware.json", "r");
  char buffer[255];
  while (fgets(buffer, sizeof(buffer), file)){
    Serial.println(buffer);
    delay(1000);
  }
  fclose(file);
  char linie ='c';
  return linie;

  // // Parse JSON
  // StaticJsonDocument<1024> doc;
  // DeserializationError error = deserializeJson(doc, json_file);

  // if (error) {
  //   Serial.print("JSON deserialization failed: ");
  //   Serial.println(error.c_str());
  //   return "";
  // }

  // // Extract version and link
  // String version = doc["version"].as<String>();
  // String link = doc["link"].as<String>();

  // Serial.println("Version: " + version);
  // Serial.println("Link: " + link);

  // return version + "\n" + link;
}

void updater(char url){
  // updates the esp with the specific bin file.
  ;
}

String VERSION;

void setup() {
  Serial.begin(115200);
  initWiFi();
  
  // Serial.println("FROM FILE");
  // Serial.print(lines);
  // Serial.println("FROM FILE");

  /// READ JSON FILE ///
  // char lines = read_json();

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

    https.begin(client, url); // Must be HTTPS!
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
      Serial.println("Failed to fetch firmware. HTTP code: " + String(httpCode));
    }

    https.end();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "Resources/connectivity.h"


// sudo chmod a+rw /dev/ttyUSB0

const char* url = "https://.eu-north-1.amazonaws.com/file.";  // The newly generated URL (temporary ) aws s3 presign s3://bucket/file_in_bucket --expires-in time
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIID0zCCArugAwIBAgIQC1RBc3sZbxLNPqT79rLJfzANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzETMBEGA1UEChMKRGlnaUNlcnQgSW5jMRkwFwYDVQQLExBE\n" \
"aWdpQ2VydCBHbG9iYWwgUm9vdDEiMCAGA1UEAxMZRGlnaUNlcnQgR2xvYmFsIFJv\n" \
"b3QgRzIwHhcNMTQwNzE2MDAwMDAwWhcNMzQwNzE2MDAwMDAwWjBhMQswCQYDVQQG\n" \
"EwJVUzETMBEGA1UEChMKRGlnaUNlcnQgSW5jMRkwFwYDVQQLExBEaWdpQ2VydCBH\n" \
"bG9iYWwgUm9vdDEiMCAGA1UEAxMZRGlnaUNlcnQgR2xvYmFsIFJvb3QgRzIwggEi\n" \
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC+F+jsvikKy/65LWEx/TMkCDI/\n" \
"9V4FqkSyG4Wh7hSJLQCcdXQgtVtoUU0Sr0ko09wU5/Y51j8D8G89rcvjF+rwk+Wy\n" \
"4iFANkE+VYFUn/BBc7n7hK5Dr1VxKHgfNstH47qmYtbe4zJUaJ5pIM1dTzZ9Yb05\n" \
"sl+f/2YSmWUN+e1L8nHrbt2xdQKBgQDPZ9FHOIczEya5zL5BqSMz2FvYldmRz+Hi\n" \
"1fnRNHox0lrJNaPp3L0jHv7K/Ki+gN0gfwIDZ41XjmBqsINEn1T3pB2kl+eR2nTS\n" \
"KvSp3RSLbxsSe0PXypPDCdI7iHaWy/8H7MkZHlFYzOeVHD9OtO0Z+VnkgY9JK+Et\n" \
"sswDf44TWvIbAgMBAAGjQjBAMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFNLE\n" \
"sNKR0BHLH0vY2nDQIGb6+JT2MA4GA1UdDwEB/wQEAwIBhjANBgkqhkiG9w0BAQsF\n" \
"AAOCAgEAKmKk3N0CpjGhQ7v9YgPKzW0fvkYFFQFhjQ7pG2YY5F7+5G1kPGVh+7cG\n" \
"zF5ZPZ3zU3YMbIskpQJ0k7ulU+zRSI6YIRFQEYef39Q4+1J9O/s9myo7u0PzV73j\n" \
"0kzVfs8AjtKoaVHgMHqYjgJv1nAtv8vbLpynepTcGAfRWRBf6a4x+H7duCCJykPY\n" \
"b2tZs+iaGG+XjF7v5w2m9YB27L3+F2S3JUzokfbM6UQ2o+Z7WvY8nX1Ti9VZhGPP\n" \
"UHd45yWx/Vhf5WJS1xuL5CbnW/jcZYVvzz0rMKIct2twT4YvSSGBfEUCdG7uA60l\n" \
"rY/Nx+yOnQ6MStqYxgPPWDVOwbyFjvFZqZJcJbEf/nI0G2ciJS1YQHzCJY+eyLCF\n" \
"OtQUiTwXwfJS4zVNGkHZMr4Xhzrbt9O/N2v7l/3vItMGKx/Jr0VYLQNdjZh3sDUt\n" \
"F3IvYNOyU5S3ac4UgE7BGH/u4wT7iwbFcudPBAlnS9M1q9f+kN2VDxS9fcmuk9qu\n" \
"le+vPZJGeZbxr3ix6aEYw31KsFDP0XYB9NLZP3st8uH0tDdmB2TRsrUCu1ZkM1os\n" \
"z9z9ApkAI9GLbKucg+BQkCpNVNzx6Tn6fEjx24g4Xd+pfZIlAbCjhBn6zDO8tTLC\n" \
"BUg+P/6BlfcB/qQrxS8G0rrFJGdfQDLqhqu80V6LnmhkymA=\n" \
"-----END CERTIFICATE-----\n";

const char *ssid = SSID;
const char *pass = PASS;

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
  Serial.print("URL: ");
  Serial.println(url);

  // Begin OTA
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    WiFiClientSecure client;

    client.setCACert(root_ca);
    // client.setInsecure();

    https.begin(client, url); // Must be HTTPS!
    int httpCode = https.GET();

    if (httpCode == 200) {
      int contentLength = https.getSize();
      bool canBegin = Update.begin(contentLength);
      if (canBegin) {
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

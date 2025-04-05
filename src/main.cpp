#include <Arduino.h>
#include "../src/Resources/connectivity.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
// sudo chmod a+rw /dev/ttyUSB0

const char *ssid = SSID;
const char *pass = PASS;

void setup()
{
  wifi_connect();
  ota_check();

  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  pinMode(LED_BUILTIN, OUTPUT);
}

void wifi_connect()
{
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected");
}

void ota_check()
{
  
}

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

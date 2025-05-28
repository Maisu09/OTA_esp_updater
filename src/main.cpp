#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "mbedtls/base64.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/aes.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "Resources/connectivity.h"  // SSID & PASS

const char *ssid = SSID;
const char *pass = PASS;
const char *gateway = "https://ur94px0mnc.execute-api.eu";  // Your API Gateway
const char *api_key = "";  // Your API Key

// Your RSA private key
const char private_key_pem[] = R"KEY(
-----BEGIN PRIVATE KEY-----

-----END PRIVATE KEY-----
)KEY";


// Connect to WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());
}

bool base64Decode(const String &input, std::vector<uint8_t> &output) {
    size_t olen = 0;
    size_t inputLen = input.length();
    output.resize((inputLen * 3) / 4 + 1);

    int res = mbedtls_base64_decode(
        output.data(), output.size(), &olen,
        reinterpret_cast<const uint8_t *>(input.c_str()), inputLen
    );
    if (res != 0) {
        Serial.println("Base64 decode failed");
        return false;
    }
    output.resize(olen);
    return true;
}

// Decrypt Base64 RSA-encrypted AES key
std::vector<uint8_t> decryptBase64RSA(const String &encryptedB64) {
  mbedtls_pk_context pk;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;

  mbedtls_pk_init(&pk);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  const char *pers = "rsa_decrypt";
  mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                        (const unsigned char *)pers, strlen(pers));

  if (mbedtls_pk_parse_key(&pk, (const unsigned char *)private_key_pem,
                           strlen(private_key_pem) + 1, NULL, 0) != 0) {
    Serial.println("Failed to parse private key");
    return {};
  }

  size_t olen;
  uint8_t encrypted_bin[512];
  if (mbedtls_base64_decode(encrypted_bin, sizeof(encrypted_bin), &olen,
                            (const unsigned char *)encryptedB64.c_str(),
                            encryptedB64.length()) != 0) {
    Serial.println("Base64 decode failed");
    return {};
  }

  uint8_t decrypted[256];
  size_t decrypted_len;
  if (mbedtls_pk_decrypt(&pk, encrypted_bin, olen, decrypted, &decrypted_len,
                         sizeof(decrypted), mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
    Serial.println("RSA decryption failed");
    return {};
  }

  mbedtls_pk_free(&pk);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);

  return std::vector<uint8_t>(decrypted, decrypted + decrypted_len);
}

// AES-CBC Decrypt (data includes IV as first 16 bytes)
String aesDecrypt(const String &base64Ciphertext, const std::vector<uint8_t> &aesKey) {
  Serial.println("Base64 ciphertext: " + base64Ciphertext);
  Serial.printf("AES key size: %d\n", aesKey.size());
  std::vector<uint8_t> encrypted(512);
  size_t olen;
  int res = mbedtls_base64_decode(encrypted.data(), encrypted.size(), &olen,
                                  (const unsigned char *)base64Ciphertext.c_str(),
                                  base64Ciphertext.length());
  if (res != 0) {
    Serial.printf("Base64 decode failed (AES), mbedtls error: %d\n", res);
    return "";
  }
  encrypted.resize(olen);

  std::vector<uint8_t> iv(encrypted.begin(), encrypted.begin() + 16);
  std::vector<uint8_t> ciphertext(encrypted.begin() + 16, encrypted.end());

  std::vector<uint8_t> decrypted(ciphertext.size());
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, aesKey.data(), 128);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertext.size(),
                        iv.data(), ciphertext.data(), decrypted.data());
  mbedtls_aes_free(&aes);

  // Remove PKCS7 padding
  uint8_t padLen = decrypted.back();
  if (padLen > 16) {
    Serial.println("Invalid padding");
    return "";
  }
  decrypted.resize(decrypted.size() - padLen);

  return String((char *)decrypted.data());
}

// Fetch Encrypted Data (JSON contains AES key + encrypted URL)
bool fetchEncryptedFields(String &out_enc_key, String &out_enc_data) {
  HTTPClient https;
  WiFiClientSecure client;
  client.setInsecure();

  https.begin(client, gateway);  // Your API Gateway
  https.addHeader("x-api-key", api_key);  // Your API Key

  int code = https.GET();
  if (code != 200) {
    Serial.printf("API call failed: %d\n", code);
    https.end();
    return false;
  }

  String body = https.getString();
  https.end();

  Serial.println("API JSON: " + body);

  ArduinoJson::JsonDocument doc1;
  DeserializationError error1 = deserializeJson(doc1, body);
  if (error1) {
    Serial.println("Failed to parse outer JSON");
    return false;
  }

  String innerJson = doc1["body"].as<String>();

  ArduinoJson::JsonDocument doc2;
  DeserializationError error2 = deserializeJson(doc2, innerJson);
  if (error2) {
    Serial.println("Failed to parse inner JSON");
    return false;
  }

  out_enc_key = doc2["key"].as<String>();
  out_enc_data = doc2["data"].as<String>();
  Serial.println("Encrypted Key:");
  Serial.println(out_enc_key);

  Serial.println("Encrypted Data:");
  Serial.println(out_enc_data);

  return true;
}

// Perform OTA update using decrypted presigned URL
void performOTA(const String &url) {
  HTTPClient https;
  WiFiClientSecure client;
  client.setInsecure();

  https.begin(client, url);
  int code = https.GET();
  if (code != 200) {
    Serial.printf("Firmware download failed: %d\n", code);
    https.end();
    return;
  }

  int len = https.getSize();
  bool canBegin = Update.begin(len);
  if (!canBegin) {
    Serial.println("Update.begin failed");
    https.end();
    return;
  }

  WiFiClient *stream = https.getStreamPtr();
  size_t written = Update.writeStream(*stream);

  if (written == len) Serial.println("Written all firmware bytes");
  else Serial.printf("Partial write: %d/%d\n", written, len);

  if (Update.end() && Update.isFinished()) {
    Serial.println("OTA update complete. Rebooting...");
    ESP.restart();
  } else {
    Serial.println("OTA update failed");
  }

  https.end();
}

void setup() {
  Serial.begin(115200);
  initWiFi();

  String enc_key_b64, enc_data_b64;
  if (!fetchEncryptedFields(enc_key_b64, enc_data_b64)) return;

  Serial.print("enc_data_b64: [");
  Serial.print(enc_data_b64);
  Serial.println("]");
  Serial.printf("Length: %d\n", enc_data_b64.length());

  // Sanitize and pad base64 string
  enc_data_b64.replace("\n", "");
  enc_data_b64.replace("\\n", "");
  enc_data_b64.replace("\\", "");
  enc_data_b64.trim();
  while (enc_data_b64.length() % 4 != 0) {
    enc_data_b64 += "=";
  }
  Serial.print("Sanitized enc_data_b64: [");
  Serial.print(enc_data_b64);
  Serial.println("]");
  Serial.printf("Sanitized Length: %d\n", enc_data_b64.length());

  std::vector<uint8_t> aes_key = decryptBase64RSA(enc_key_b64);
  if (aes_key.empty()) {
    Serial.println("RSA decryption failed");
    return;
  }

  String decrypted_url = aesDecrypt(enc_data_b64, aes_key);
  if (decrypted_url == "") {
    Serial.println("AES decryption failed");
    return;
  }

  Serial.println("Decrypted URL: " + decrypted_url);
  performOTA(decrypted_url);
}

void loop() {
  // Optional: Periodic OTA check
  delay(60000);  // Every 60s
}

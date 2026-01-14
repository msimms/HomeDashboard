// MIT License
//
// Copyright (c) 2025 Mike Simms
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//  Wiring (ESP8266 -> MAX6675):
//    3V3 -> VCC
//    GND -> GND
//    GPIO14 (D5) -> SCK
//    GPIO12 (D6) -> SO
//    GPIO5  (D1) -> CS
//
//  Payload example:
//    { "device":"esp-12f-ac-inlet", "celsius": 27.38, "ts": 1734558123 }
//
//  Notes:
//  - Post interval defaults to 5s.
//  - MAX6675 converts every ~220ms; no need to read faster than ~4–5 Hz.
//  - For HTTPS, set a fingerprint or (easier but less secure) setInsecure().

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <LittleFS.h>
#include <max6675.h>
#include "arduino_secrets.h"

// Pin mapping (change if you wired differently)
const int PIN_SO  = 12; // GPIO12 (D6)
const int PIN_CS  = 5;  // GPIO5  (D1)
const int PIN_SCK = 14; // GPIO14 (D5)

MAX6675 tc(PIN_SCK, PIN_CS, PIN_SO);

BearSSL::CertStore g_certStore;

/// @function load_certs
void load_certs() {
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status()!=WL_CONNECTED) {
    delay(200);
  }

  // Time is required for CA validation
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8*3600*2) {
    delay(500);
    now = time(nullptr);
  }

  // Mount FS and load cert bundle files
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  // loads certs.idx / certs.ar from LittleFS
  int num_certs = g_certStore.initCertStore(LittleFS, "/certs.idx", "/certs.ar");
  Serial.printf("Certs loaded: %d\n", num_certs);
}

/// @function post_status
void post_status(String str) {

  // Attempt to connect to Wi-Fi network.
  Serial.println("[INFO] Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("[INFO] Attempting to connect to the network: ");
    Serial.println(SECRET_SSID);
    int wifi_status = WiFi.begin(SECRET_SSID, SECRET_PASS);

    // Wait a few seconds for connection.
    delay(5000);
  }

  // Network is connected....
  Serial.println("[INFO] Wifi connected!");

  // Attempt to connect to Wi-Fi network.
  Serial.println("[INFO] Connecting to WiFi...");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("[INFO] Attempting to connect to the network: ");
    Serial.println(SECRET_SSID);
    int wifi_status = WiFi.begin(SECRET_SSID, SECRET_PASS);

    // Wait a few seconds for connection.
    delay(5000);
  }

  // Network is connected....
  Serial.println("[INFO] Wifi connected!");

  // Load the certificates.
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setCertStore(&g_certStore);
  client->setX509Time(time(nullptr));
 
  // Send.
  HTTPClient https;
  char buff[128];
  snprintf(buff, sizeof(buff) - 1, "https://%s/api/1.0/update_status", STATUS_URL);
  if (https.begin(*client, buff)) {
    https.addHeader("Content-Type", "application/json");
    int code = https.POST(str);
    https.end();
  }
}

void setup() {

  // Initialize serial and wait for port to open.
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("MAX6675 + ESP8266 ready");
}

void loop() {
  double c = tc.readCelsius();   // ~220 ms conversion latency internally

  // Sanity check.
  if (isnan(c)) {
    Serial.println("Thermocouple missing/open!");
  } else {

    // For serial debugging.
    double f = c * 9.0 / 5.0 + 32.0;
    Serial.printf("Temp: %.2f °C (%.2f °F)\n", c, f);

    // Format the output.
    char buff[600];
    snprintf(buff, sizeof(buff) - 1, "{\"collection\": \"ac\", \"api_key\": \"%s\", \"ac_outlet_temp\": %f}", API_KEY, c);
    Serial.println(buff);

    // Send.
    //post_status(buff);
  }
  delay(10000);
}

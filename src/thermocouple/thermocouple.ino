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
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <max6675.h>
#include "arduino_secrets.h"

// Pin mapping (change if you wired differently)
const int PIN_SO  = 12; // GPIO12 (D6)
const int PIN_CS  = 5;  // GPIO5  (D1)
const int PIN_SCK = 14; // GPIO14 (D5)

MAX6675 tc(PIN_SCK, PIN_CS, PIN_SO);

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";   

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

  // Endpoint.
  char endpointUrl[64];
  snprintf(endpointUrl, sizeof(endpointUrl) - 1, "https://%s/api/1.0/update_status", STATUS_URL);

  // Connect to the server.
  WiFiClientSecure client;
  Serial.println("[INFO] Connecting to the status server...");
  //client.setCACert(root_ca); // Your root CA certificate
  client.setInsecure();
  if (client.connect(STATUS_URL, STATUS_PORT)) {

    // Send HTTPS POST
    Serial.println("[INFO] Sending HTTPS POST...");
    HTTPClient http;
    http.begin(client, endpointUrl); // Use root_ca for HTTPS
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(str);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("[INFO] Response: ");
      Serial.print(response);
    } else {
      Serial.printf("[ERROR] Error sending HTTPS POST to %s, code %d\n", endpointUrl, httpResponseCode);
    }
    http.end();
  }
  else {
    Serial.println("[ERROR] Failed to connect to the status server.");
  }
}

void setup() {

  // Initialize serial and wait for port to open.
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("[INFO] MAX6675 + ESP8266 ready");
}

void loop() {
  double c = tc.readCelsius();   // ~220 ms conversion latency internally

  // Sanity check.
  if (isnan(c)) {
    Serial.println("[INFO] Thermocouple missing/open!");
  } else {

    // For serial debugging.
    double f = c * 9.0 / 5.0 + 32.0;
    Serial.printf("[INFO] Temp: %.2f °C (%.2f °F)\n", c, f);

    // Format the output.
    char buff[600];
    snprintf(buff, sizeof(buff) - 1, "{\"collection\": \"ac\", \"api_key\": \"%s\", \"ac_outlet_temp\": %f}", API_KEY, c);
    Serial.print("[DEBUG] ");
    Serial.println(buff);

    // Send.
    post_status(buff);
  }
  delay(60000);
}

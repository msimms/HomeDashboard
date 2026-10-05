// Created by Michael Simms

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "time.h"
#include "arduino_secrets.h"

// The "pin" for the onboard LED.
const int LED = 2;

// The pin for the pump relay.
const int PUMP_PIN = 18;

// Eastern US timezone, including daylight saving time
const char* TIMEZONE = "EST5EDT,M3.2.0/2,M11.1.0/2";

// The time at which watering will happen.
// Can be overriden when settings are read.
int g_waterHour = 16;

// The duratino of watering.
// Can be overriden when settings are read.
int g_waterSecs = 120;

// Function prototypes
void blink(int num_blinks);
void irrigate(int num_seconds);
int parseSettingResponse(String response);
int getSettingSecure(String requestUrl);
int getSetting(String requestUrl);
void getSettings();
void connectWiFi();
void setup();
void loop();

/// @function blink
/// Blinks the LED the specified number of times, once per second.
void blink(int num_blinks) {
  for (int i = 0; i < num_blinks; ++i) {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
}

/// @function irrigate
/// Blinks the LED while running the pump.
void irrigate(int num_seconds) {
  digitalWrite(PUMP_PIN, HIGH);
  for (int i = 0; i < num_seconds; ++i) {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
  digitalWrite(PUMP_PIN, LOW);
}

/// @function parseSettingResponse
int parseSettingResponse(String response) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      int value = doc["value"].as<int>();
      return value;
    }
    return -1;
}

/// @function getSettingSecure
int getSettingSecure(String requestUrl) {
  if (WiFi.status() == WL_CONNECTED) {

    // Connect to the client.
    WiFiClientSecure client;
    HTTPClient https;

    client.setInsecure();
    if (https.begin(client, requestUrl)) {
      Serial.println("[INFO] Sending the request...");
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.print("[INFO] HTTP status: ");
        Serial.println(httpCode);
        String response = https.getString();
        Serial.print("[INFO] HTTP response: ");
        Serial.println(response);
        return parseSettingResponse(response);
      }
      else {
        Serial.print("[ERROR] HTTPS GET failed: ");
        Serial.println(https.errorToString(httpCode));
      }
      https.end();
    }
    else {
      Serial.println("[ERROR] Unable to start HTTPS connection");
    }
  }
  return -1;
}

/// @function getSetting
int getSetting(String requestUrl) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Send the GET request
    Serial.println("[INFO] Sending the request...");
    Serial.println(requestUrl);
    http.begin(requestUrl);
    
    // Send and read the response.
    int httpResponseCode = http.GET();    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      return parseSettingResponse(response);
    } else {
      Serial.print("[ERROR] Error on sending GET: ");
      Serial.println(httpResponseCode);
    }
    
    // Clean up
    http.end();
  }
  return -1;
}

/// @function getSettings
void getSettings() {
  String requestUrl = STATUS_URL;
  requestUrl.concat("/api/1.0/setting?key=irrigation_time&api_key=");
  requestUrl.concat(API_KEY);
  int temp = getSettingSecure(requestUrl);
  if (temp >= 0 && temp < 24) {
    Serial.print("[INFO] Setting watering hour to: ");
    Serial.println(temp);
    g_waterHour = temp;
  }

  requestUrl = STATUS_URL;
  requestUrl.concat("/api/1.0/setting?key=irrigation_duration&api_key=");
  requestUrl.concat(API_KEY);
  temp = getSettingSecure(requestUrl);
  if (temp >= 0 && temp < 86400) {
    Serial.print("[INFO] Setting watering duration to: ");
    Serial.println(temp);
    g_waterSecs = temp;
  }
}

/// @function connectWiFi
void connectWiFi() {
  Serial.print("[INFO] Connecting to Wi-Fi");
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[INFO] Wi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Get time from NTP.
  configTime(0, 0, "pool.ntp.org");
  Serial.println("[INFO] Time synchronized!");

  // Set local timezone.
  setenv("TZ", TIMEZONE, 1);
  tzset();
}

/// @function setup
void setup() {
  // Initialize the serial port.
  Serial.begin(115200);

  // Make sure the pump is off.
  Serial.println("[INFO] Disabling pump!");
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  // Connect to Wifi and retrieve the latest settings, if any.
  connectWiFi();
  getSettings();
}

/// @function loop
void loop() {
  struct tm timeinfo;

  // See if it's time to run the irrigation.
  if (getLocalTime(&timeinfo)) {
    Serial.printf("[INFO] Current Time: %02d:%02d:%02d\n",
                  timeinfo.tm_hour,
                  timeinfo.tm_min,
                  timeinfo.tm_sec);

    if (timeinfo.tm_hour == g_waterHour && timeinfo.tm_min == 0) {
      Serial.println("[INFO] Starting irrigation!");
      irrigate(g_waterSecs);
      Serial.println("[INFO] Irrigation complete!");
    }
  }

  delay(5000);
}

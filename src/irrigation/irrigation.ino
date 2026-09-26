// Created by Michael Simms

#include <WiFi.h>
#include "time.h"
#include "arduino_secrets.h"

// The "pin" for the onboard LED.
const int LED = 2;

// The pin for the pump relay.
const int PUMP_PIN = 18;

const int WATER_HOUR   = 16;
const int WATER_MINUTE = 17;

// Eastern US timezone, including daylight saving time
const char* TIMEZONE = "EST5EDT,M3.2.0/2,M11.1.0/2";

void blink(int num_blinks);
void irrigate(int num_seconds);
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

/// @function connectWiFi
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Get time from NTP.
  configTime(0, 0, "pool.ntp.org");
  Serial.println("Time synchronized!");

  // Set local timezone.
  setenv("TZ", TIMEZONE, 1);
  tzset();
}

/// @function setup
void setup() {
  Serial.begin(115200);
  connectWiFi();

  // Make sure the pump is off.
  Serial.println("Disabling pump!");
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
}

/// @function loop
void loop() {
  struct tm timeinfo;

  // See if it's time to run the irrigation.
  if (getLocalTime(&timeinfo)) {
    Serial.printf("%02d:%02d:%02d\n",
                  timeinfo.tm_hour,
                  timeinfo.tm_min,
                  timeinfo.tm_sec);

    if (timeinfo.tm_hour == WATER_HOUR && timeinfo.tm_min == WATER_MINUTE) {
      Serial.println("Starting irrigation!");
      irrigate(120);
      Serial.println("Irrigation complete!");
    }
  }

  delay(1000);
}

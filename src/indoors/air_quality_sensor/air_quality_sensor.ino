// Created by Michael Simms

// Reads from the co2, humidity, and temperature sensor and writes the result to the serial port.

#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <Adafruit_SGP40.h>

SensirionI2CScd4x scd4x; // CO2, temp, and humidity
Adafruit_SGP40 sgp; // VOC

// The "pin" for the onboard LED.
int LED = 13;

/// @function printUint16Hex
void printUint16Hex(uint16_t value) {
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}

/// @function printSerialNumber
void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
  Serial.print("Serial: 0x");
  printUint16Hex(serial0);
  printUint16Hex(serial1);
  printUint16Hex(serial2);
  Serial.println();
}

/// @function setup
/// Called once, at program start
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(100);
  }

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // Stop potentially previously started measurement.
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error: Failed to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  // Start Measurement.
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error: Failed to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  // Start the SGP VOC sensor.
  if (!sgp.begin()){
    Serial.println("SGP40 sensor not found :(");
  }

  // Force a short wait before attempting the first read
  delay(10000);
}

/// @function loop
/// Called repeatedly
void loop() {
  uint16_t error;
  char errorMessage[256];

  // Turn the LED on.
  digitalWrite(LED, HIGH);

  // Read the CO2, temperature, and humidity measurements.
  uint16_t co2;
  float temperature;
  float humidity;
  error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error) {
    Serial.print("Error: Failed to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (co2 == 0) {
    Serial.println("Error: Invalid sample detected, skipping.");
  } else {
    // Read the raw VOC measurement as well as the temperature and humidity compensated version.
    uint16_t sraw = sgp.measureRaw(temperature, humidity);
    int32_t voc_index = sgp.measureVocIndex(temperature, humidity);

    // Output:
    Serial.print(co2);
    Serial.print("\t");
    Serial.print(temperature);
    Serial.print("\t");
    Serial.print(humidity);
    Serial.print("\t");
    Serial.print(sraw);
    Serial.print("\t");
    Serial.println(voc_index);
  }

  // Turn the LED off.
  digitalWrite(LED, LOW);

  // Rate limit.
  delay(600000);
}

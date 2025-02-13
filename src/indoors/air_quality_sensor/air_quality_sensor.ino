// Created by Michael Simms

// Reads from the co2, humidity, and temperature sensor and writes the result to the serial port.

#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <Adafruit_SGP40.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SensirionI2cScd4x scd4x; // CO2, temp, and humidity
Adafruit_SGP40 sgp; // VOC

// The "pin" for the onboard LED.
int LED = 13;

void updateDisplay(char* msg) {
  display.clearDisplay();

  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner
  display.println(msg);

  display.display();
}

/// @function setup
/// Called once, at program start
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(100);
  }

  Wire.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
  }
  else {
    Serial.println(F("Error: SSD1306 allocation failed!"));
  }

  // Start the CO2 sensor.
  scd4x.begin(Wire, SCD41_I2C_ADDR_62);

  // Ensure sensor is in clean state
  uint16_t error = scd4x.wakeUp();
  if (error) {
      Serial.println("Error trying to execute wakeUp()!");
  }

  // Stop potentially previously started measurement.
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.println("Error: Failed to execute stopPeriodicMeasurement()!");
  }

  // Start Measurement.
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.println("Error: Failed to execute startPeriodicMeasurement()!");
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
  // Turn the LED on.
  digitalWrite(LED, HIGH);

  // Read the CO2, temperature, and humidity measurements.
  uint16_t co2;
  float temperature;
  float humidity;
  uint16_t error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error) {
    Serial.println("Error: Failed to execute readMeasurement()!");
  } else if (co2 == 0) {
    Serial.println("Error: Invalid sample detected, skipping.");
  } else {
    // Read the raw VOC measurement as well as the temperature and humidity compensated version.
    uint16_t sraw = sgp.measureRaw(temperature, humidity);
    int32_t voc_index = sgp.measureVocIndex(temperature, humidity);

    // Format the output.
    char tempBuf[12];
    dtostrf(temperature, 4, 2, tempBuf);
    char humidityBuf[12];
    dtostrf(humidity, 4, 2, humidityBuf);
    char buff[32];
    snprintf(buff, sizeof(buff) - 1, "%u\t%s\t%s\t%u\t%u\t", co2, tempBuf, humidityBuf, sraw, voc_index);

    // Output to serial:
    Serial.println(buff);

    // Output to the display:
    updateDisplay(buff);
  }

  // Turn the LED off.
  digitalWrite(LED, LOW);

  // Rate limit.
  delay(600000);
}

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

#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <Wire.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include "Arduino_LED_Matrix.h" // Include the LED_Matrix library
#include "arduino_secrets.h" 

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 g_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ArduinoLEDMatrix g_matrix;  // Create an instance of the ArduinoLEDMatrix class

// The "pin" for the onboard LED.
int LED = 13;

#define LOGO_HEIGHT 32
#define LOGO_WIDTH  32
#define LEFT_LOGO_MARGIN 4
#define LEFT_TEXT_MARGIN (LEFT_LOGO_MARGIN + LOGO_WIDTH + 4)
static const unsigned char PROGMEM g_logo_bmp[] =
{ 0x00, 0x00, 0x00, 0x00,
  0x00, 0x05, 0xA0, 0x00,
  0x00, 0x3F, 0xFC, 0x00,
  0x00, 0xF8, 0x1F, 0x00,
  0x01, 0xC0, 0x03, 0x80,
  0x03, 0x80, 0x00, 0xE0,
  0x06, 0x00, 0x00, 0x60,
  0x0E, 0x05, 0x00, 0x30,
  0x18, 0x81, 0x08, 0x18,
  0x19, 0x74, 0xD4, 0x18,
  0x32, 0x45, 0x94, 0x0C,
  0x31, 0x14, 0x90, 0x0C,
  0x30, 0x51, 0x48, 0x06,
  0x20, 0x14, 0x00, 0x06,
  0x60, 0x11, 0x00, 0x06,
  0x20, 0x05, 0x4D, 0x86,
  0x60, 0x14, 0x94, 0x06,
  0x20, 0x11, 0x90, 0x86,
  0x70, 0x15, 0x50, 0xC6,
  0x20, 0x10, 0x08, 0x0C,
  0x30, 0x10, 0x00, 0x04,
  0x30, 0x15, 0xD5, 0x8C,
  0x18, 0x15, 0x15, 0x18,
  0x1C, 0x11, 0x11, 0x18,
  0x0C, 0x14, 0x91, 0x30,
  0x06, 0x08, 0x44, 0x70,
  0x03, 0x80, 0x00, 0xE0,
  0x01, 0xC0, 0x03, 0x80,
  0x00, 0xF8, 0x1F, 0x00,
  0x00, 0x3F, 0xFC, 0x00,
  0x00, 0x05, 0xC0, 0x00,
  0x00, 0x00, 0x00, 0x00 };
  
// HX711 circuit wiring for three load cells.
const int LOADCELL1_DOUT_PIN = 2;
const int LOADCELL1_SCK_PIN = 3;
const int LOADCELL2_DOUT_PIN = 4;
const int LOADCELL2_SCK_PIN = 5;
const int LOADCELL3_DOUT_PIN = 6;
const int LOADCELL3_SCK_PIN = 7;
const int LOADCELL4_DOUT_PIN = 8;
const int LOADCELL4_SCK_PIN = 9;

// HX711 objects.
HX711 g_hx711_1;
HX711 g_hx711_2;
HX711 g_hx711_3;
HX711 g_hx711_4;

// The most recent raw values.
long g_raw_value_1 = 0;
long g_raw_value_2 = 0;
long g_raw_value_3 = 0;
long g_raw_value_4 = 0;

// Will return this as an error code when reading an HX711.
#define ERROR_NUM -1

// Configuration values.
float g_tare_value = 0.0;
float g_calibration_value = 0.0;
float g_calibration_weight = 0.0;
bool g_tare_set = false;
bool g_cal_set = false;

/// @function setup_wifi
void setup_wifi() {

  // Make sure we were given a network to connect to.
  if (strlen(SECRET_SSID) == 0) {
    Serial.println("[ERROR] SSID not specified!");
    return;
  }
}

/// @function print_wifi_status
void print_wifi_status() {

  // Print the SSID of the attached network.
  Serial.print("[INFO] SSID: ");
  Serial.println(WiFi.SSID());

  // Print the board's IP address.
  IPAddress ip = WiFi.localIP();
  Serial.print("[INFO] IP Address: ");
  Serial.println(ip);

  // Print the received signal strength.
  long rssi = WiFi.RSSI();
  Serial.print("[INFO] Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/// @function post_status
void post_status(String str) {

  // Attempt to connect to Wi-Fi network:
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
  WiFiSSLClient ssl; // TLS socket

  HttpClient http(ssl, STATUS_URL, STATUS_PORT);
  http.beginRequest();
  http.post("/api/1.0/update_status");
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("Content-Length", str.length());
  http.sendHeader("Connection: close\r\n");
  http.beginBody();
  http.print(str);
  http.print("\r\n"); // end of headers
  http.endRequest();

  int status = http.responseStatusCode();
  String body = http.responseBody();
  Serial.print("[INFO] Status: "); Serial.println(status);
  Serial.println(body);

  WiFi.end();
}

/// @function float_is_valid
bool float_is_valid(float num) {
  return num >= 0.001;
}

/// @function draw_logo
void draw_logo(void) {
  g_display.clearDisplay();
  g_display.drawBitmap(LEFT_LOGO_MARGIN,
    ((g_display.height() - LOGO_HEIGHT) / 2) + 2,
    g_logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  g_display.display();
}

/// @function update_display
void update_display(char* msg) {

  // Logo.
  draw_logo();

  // Message.
  g_display.setTextSize(1); // X pixel scale
  g_display.setTextColor(SSD1306_WHITE); // Draw white text
  g_display.setCursor(LEFT_TEXT_MARGIN, 16); // Start at top-left corner, after the logo
  g_display.println(msg);
  g_display.display();
}

/// @function update_display_with_weight
void update_display_with_weight(char* msg) {

  // Logo.
  draw_logo();

  // Message.
  g_display.setTextSize(2); // X pixel scale
  g_display.setTextColor(SSD1306_WHITE); // Draw white text
  g_display.setCursor(LEFT_TEXT_MARGIN, 12); // Start at top-left corner, after the logo
  g_display.println(msg);
  g_display.display();
}

/// @function setup_display
/// Configures the external LCD.
void setup_display(void) {

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (g_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    draw_logo();
  }
  else {
    Serial.println("[ERROR] SSD1306 allocation failed!");
  }
}

/// @function setup_scale
/// Called once to initializae all of the HX711s.
void setup_scale(void) {

  // Make sure the load cell amplifiers are on.
  g_hx711_1.power_up();
  delay(10);
  g_hx711_2.power_up();
  delay(10);
  g_hx711_3.power_up();
  delay(10);
  g_hx711_4.power_up();
  delay(10);

  // Configure the load cell amplifiers.
  g_hx711_1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);
  delay(10);
  g_hx711_2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);
  delay(10);
  g_hx711_3.begin(LOADCELL3_DOUT_PIN, LOADCELL3_SCK_PIN);
  delay(10);
  g_hx711_4.begin(LOADCELL4_DOUT_PIN, LOADCELL4_SCK_PIN);
  delay(10);
}

/// @function readHx711
/// Called to read a value from a single HX711.
long readHx711(HX711 hx) {
  uint8_t loop_count = 0;
  while (!hx.is_ready() && loop_count < 100) {
    delay(10);
    ++loop_count;
  }
  if (hx.is_ready()) {
    return hx.read_average();
  }
  return ERROR_NUM;
}

/// @function read_scale_value
// Called to read a value from the scale.
long read_scale_value(void) {
  long sum = 0;

  // Load Cell 1
  g_raw_value_1 = readHx711(g_hx711_1);
  if (g_raw_value_1 == ERROR_NUM) {
    Serial.println("[ERROR] Failed to read from HX711 #1");
    return ERROR_NUM;
  }
  else {
    sum = sum + g_raw_value_1;
  }

  // Load Cell 2
  g_raw_value_2 = readHx711(g_hx711_2);
  if (g_raw_value_2 == ERROR_NUM) {
    Serial.println("[ERROR] Failed to read from HX711 #2");
    return ERROR_NUM;
  }
  else {
    sum = sum + g_raw_value_2;
  }

  // Load Cell 3
  g_raw_value_3 = readHx711(g_hx711_3);
  if (g_raw_value_3 == ERROR_NUM) {
    Serial.println("[ERROR] Failed to read from HX711 #3");
    return ERROR_NUM;
  }
  else {
    sum = sum + g_raw_value_3;
  }

  // Load Cell 4
  g_raw_value_4 = readHx711(g_hx711_4);
  if (g_raw_value_4 == ERROR_NUM) {
    Serial.println("[ERROR] Failed to read from HX711 #4");
    return ERROR_NUM;
  }
  else {
    sum = sum + g_raw_value_4;
  }
  return sum;
}

/// @function compute_weight
// Returns the weight, in grams, or -1 upon error.
float compute_weight(float measured_value) {

  // Make sure we have tare and calibration values and a calibration weight.
  if (!g_tare_set && !g_cal_set) {
    Serial.println("[ERROR] No tare or calibration value!");
    return (float)ERROR_NUM;
  }
  if (!g_tare_set) {
    Serial.println("[ERROR] No tare value!");
    return (float)ERROR_NUM;
  }
  if (!g_cal_set) {
    Serial.println("[ERROR] Not calibrated!");
    return (float)ERROR_NUM;
  }
  if (!float_is_valid(g_tare_value)) {
    Serial.println("[ERROR] Invalid tare value!");
    return (float)ERROR_NUM;
  }
  if (!float_is_valid(g_calibration_value)) {
    Serial.println("[ERROR] Invalid calibration value!");
    return (float)ERROR_NUM;
  }

  // Don't go below the floor.
  if (measured_value < g_tare_value) {
    return 0.0;
  }

  float m = g_calibration_weight / (g_calibration_value - g_tare_value);
  float b = 0.0 - (m * g_tare_value);
  float weight = m * measured_value + b;
  if (weight < 0.0) {
    weight = 0.0;
  }
  return weight;
}

/// @function read_config_values
/// Reads a comma separated list of tare value, calibration value, and calibration weight (in g)
/// from the serial input.
void read_config_values() {

  // Is any data available?
  if (Serial.available() > 0) {

    // The tare value should be first.
    char c = Serial.read(); // Consume space or newline
    float value = Serial.parseFloat();

    // Sanity check the value. Return on error.
    if (float_is_valid(value)) {
      g_tare_value = value;
      g_tare_set = true;
    }
    else {
      return;
    }

    // The calibration scale value should be second.
    c = Serial.read(); // Consume space or newline
    value = Serial.parseFloat();

    // Sanity check the value. Return on error.
    if (float_is_valid(value)) {
      g_calibration_value = value;
    }
    else {
      return;
    }

    // The calibration weight should be third.
    c = Serial.read(); // Consume space or newline
    value = Serial.parseFloat();

    // Sanity check the value. Return on error.
    if (float_is_valid(value)) {
      g_calibration_weight = value;
      g_cal_set = true;
    }
    else {
      return;
    }
  }
}

/// @function read_given_weight_value
/// Reads the calibration weight (in g) from the serial input.
void read_given_weight_value(void) {

  // Is any data available?
  if (Serial.available() > 0) {

    // There should be a space or newline followed by the calibration weight, in grams.
    char c = Serial.read(); // Consume space or newline
    float value = Serial.parseFloat();

    // Sanity check the value.
    if (!float_is_valid(value)) {
      g_calibration_weight = value;
      g_cal_set = true;
    }
  }
}

/// @function setup
/// Called once, at program start
void setup() {

  // Set the serial IO rate.
  Serial.begin(9600);
  while (!Serial) {
    delay(100);
  }

  // Initialize the I2C interface.
  Wire.begin();

  // Initialize the external display.
  setup_display();

  // Initialize the scale.
  setup_scale();

  // Initialize the UNO R4's built-in display
  g_matrix.begin();
  g_matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);

  // Connect to the wifi network.
  setup_wifi();
}

/// @function loop
/// Called repeatedly
void loop() {

  float weight = (float)-1.0;

  // Turn the LED on.
  digitalWrite(LED, LOW);

  // Raw value from the scale.
  float raw_value = read_scale_value();

  // Turn the LED off.
  digitalWrite(LED, LOW);

  // Is a tare or calibration needed? If so, update the display.
  if (!g_tare_set) {

    // Update the LCD screen.
    update_display("Tare needed!");

    // Make the onboard LED panel display a sad face.
    g_matrix.loadFrame(LEDMATRIX_EMOJI_SAD);
  }
  else if (!g_cal_set) {

    // Update the LCD screen.
    update_display("Cal. needed!");

    // Make the onboard LED panel display a sad face.
    g_matrix.loadFrame(LEDMATRIX_EMOJI_SAD);
  }

  // Calculate weight and update the display.
  else {

    // Convert to weight.
    weight = compute_weight(raw_value);
    if (weight >= 0.0) {

      // Update the LCD screen.
      char buff[64];
      snprintf(buff, sizeof(buff) - 1, "%0.1fg", weight);
      update_display_with_weight(buff);

      // Make the onboard LED panel display a happy face.
      g_matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
    }
    else {

      // Update the LCD screen.
      update_display("Error!");

      // Make the onboard LED panel display a sad face.
      g_matrix.loadFrame(LEDMATRIX_EMOJI_SAD);
    }
  }

  // Print all the raw values.
  Serial.println("[DATA] Raw Values = " + String(raw_value) + ":" + String(g_raw_value_1) + ":" + String(g_raw_value_2) + ":" + String(g_raw_value_3) + ":" + String(g_raw_value_4) + ":" + String(weight));

  // Make sure all serial data has been sent before trying to read.
  //Serial.flush();

  // Are we being sent a command?
  if (Serial.available() > 0) {

    // Read the command from the serial port.
    char received_char = Serial.read();
    if (received_char == (char)-1) {

      // Update the LCD screen.
      update_display("No cmd!");
    }

    // A request to compute a new tare value was received..
    else if (received_char == 'T') {

      // Update the LCD screen.
      update_display("Tareing...");

      // Copy the raw value.
      g_tare_value = raw_value;
      g_tare_set = true;

      // Output it for debugging and for the client.
      Serial.print("[DATA] Tare Value = ");
      Serial.println(g_tare_value, 1);
    }

    // Compute a new weight value.
    else if (received_char == 'W') {

      // Update the LCD screen.
      update_display("Calibrating...");

      // Copy the raw value.
      g_calibration_value = raw_value;

      // We should've been given the calibration weight.
      read_given_weight_value();

      // Output it for debugging and for the client.
      Serial.print("[DATA] Given Value = ");
      Serial.println(g_calibration_value, 1);
      Serial.print("[DATA] Given Weight (g) = ");
      Serial.println(g_calibration_weight, 1);
    }

    // Receive configuration values from the command and control computer.
    else if (received_char == 'C') {

      // Update the LCD screen.
      update_display("Reading Config...");

      // Read the values.
      read_config_values();

      // Output it for debugging and for the client.
      Serial.print("[INFO] Tare Value = ");
      Serial.println(g_tare_value, 1);
      Serial.print("[INFO] Config Value = ");
      Serial.println(g_calibration_value, 1);
      Serial.print("[INFO] Config Weight (g) = ");
      Serial.println(g_calibration_weight, 1);

      update_display("Config Read!");
    }

    // Not sure what we just read.
    else {
      update_display("Invalid cmd!");
    }
  }

  // Format the output.
  char buff[800];
  snprintf(buff, sizeof(buff) - 1, "{\"collection\": \"refrigerator\", \"api_key\": \"%s\", \"keg\": %f}", API_KEY, raw_value);
  Serial.println(buff);

  // Send.
  post_status(buff);

  // Wait a few seconds (so people can read the display) and then go into deep sleep.
  delay(10000);
  asm volatile("wfi");
}

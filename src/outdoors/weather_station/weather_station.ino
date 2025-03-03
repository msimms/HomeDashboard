// Created by Michael Simms

#include <AM2315C.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 

#define REF_VOLTAGE 3.3
#define ADC_RESOLUTION 4095

// Anemometer (https://www.adafruit.com/product/1733)
#define MIN_ANEMOMETER_VOLTAGE 0.4
#define MAX_ANEMOMETER_VOLTAGE 2.0
#define MIN_WIND_SPEED 0.2  // meters per sec
#define MAX_WIND_SPEED 32.4 // meters per sec

#define MOISTURE_SENSOR_1 A1
#define MOISTURE_SENSOR_2 A2

// Wifi
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)

// The "pin" for the onboard LED.
int LED = 13;

// Temperature and humidity sensor.
AM2315C DHT;

// RED    -------- | VDD             |
// YELLOW -------- | SDA    AM2315C  |
// BLACK  -------- | GND             |
// WHITE  -------- | SCL             |

/// @function mapfloat
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/// @function read_anemometer
float read_anemometer() {
  float sensor_value = analogRead(A0);
  float voltage = (sensor_value * REF_VOLTAGE) / ADC_RESOLUTION;
  float wind_speed_ms = 0.0;

  if (voltage >= MIN_ANEMOMETER_VOLTAGE) {
    wind_speed_ms = mapfloat(voltage, MIN_ANEMOMETER_VOLTAGE, MAX_ANEMOMETER_VOLTAGE, MIN_WIND_SPEED, MAX_WIND_SPEED);
  }
  return wind_speed_ms;
}

/// @function read_temperature_and_humidity_from_am2315
void read_temperature_and_humidity_from_am2315c(float* temp_c, float* humidity) {
  int status = DHT.read();
  *temp_c = DHT.getTemperature();
  *humidity = DHT.getHumidity();
}

/// @function read_soil_moisture_sensor
float read_soil_moisture_sensor(pin_size_t pin) {
  float sensor_value = analogRead(pin);
  float percent_dry = sensor_value / 1023;
  return percent_dry;
}

/// @function read_soil_moisture_sensor_1
float read_soil_moisture_sensor_1() {
  return read_soil_moisture_sensor(MOISTURE_SENSOR_1);
}

/// @function read_soil_moisture_sensor_2
float read_soil_moisture_sensor_2() {
  return read_soil_moisture_sensor(MOISTURE_SENSOR_2);
}

/// @function setup_wifi
void setup_wifi() {
  Serial.println("Setting up Wifi...");

  // Attempt to connect to Wi-Fi network:
  int wifi_status = WL_IDLE_STATUS;
  while (wifi_status != WL_CONNECTED) {
    Serial.print("Attempting to connect to the network: ");
    Serial.println(ssid);
    wifi_status = WiFi.begin(ssid, pass);

    // Wait a few seconds for connection.
    delay(5000);
  }

  // You're connected now, so print out the data.
  Serial.println("Wifi connected!");
}

/// @function print_wifi_status
void print_wifi_status() {
  // Print the SSID of the attached network.
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print the board's IP address.
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength.
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/// @function post_status
void post_status(String str) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

    Serial.println("Sending status...");
    if (client.connect(STATUS_URL, STATUS_PORT)) {
      client.println(str);
      Serial.println("Status sent!");
    } else {
      Serial.println("Error connecting to the server!");
      print_wifi_status();
    }
    client.stop();
  }
  else {
    Serial.println("Not connected to Wifi!");
    print_wifi_status();
  }
}

/// @function setup_anemometer
void setup_anemometer() {
  Serial.println("Setting up the anemometer...");
  pinMode(A0, INPUT_PULLUP); // Enable internal pull-up resistor on pin A0
  Serial.println("Done setting up the anemometer...");
}

/// @function setup_am2315
void setup_am2315() {
  Serial.println("Setting up the AM2315...");

  Wire.begin();
  DHT.begin();

  delay(1000);

  Serial.println("Done setting up the AM2315...");
}

/// @function setup
void setup() {
  // Initialize serial and wait for port to open.
  Serial.begin(9600);

  // Set the LED as output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize wifi.
  setup_wifi();

  // Initialize the anemometer.
  setup_anemometer();

  // Initialize the AM2315.
  setup_am2315();
}

/// @function loop
void loop() {
  // Turn the LED on.
  digitalWrite(LED, HIGH);

  // Read wind speed.
  float wind_speed_ms = read_anemometer();

  // Read temperature and humidity.
  float temp_c = 0.0;
  float humidity = 0.0;
  read_temperature_and_humidity_from_am2315c(&temp_c, &humidity);

  // Read soil moisture sensor.
  float moisture1 = read_soil_moisture_sensor_1();
  float moisture2 = read_soil_moisture_sensor_2();

  // Turn the LED off.
  digitalWrite(LED, LOW);

  // Format the output.
  char buff[256];
  snprintf(buff, sizeof(buff) - 1, "{\"wind speed ms\": %f, \"temperature\": %f, \"humidity\": %f, \"moisture_sensor_1\": %f, \"moisture_sensor_2\": %f}", wind_speed_ms, temp_c, humidity, moisture1, moisture2);
  Serial.println(buff);

  // Send.
  post_status(buff);

  delay(600000);
}

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
  WiFiSSLClient client;

  // Connect to the relay client.
  Serial.println("[INFO] Sending status...");
  if (client.connectSSL(STATUS_URL, STATUS_PORT)) {
    Serial.println("[INFO] Connected!");

    // Send the HTTP header
    client.print(String("POST https://") + STATUS_URL + ("/api/1.0/update_status HTTP/1.1\r\n"));
    client.print("Host: mikesimms.info\r\n");
    client.print("User-Agent: Nano33IoT/1.0\r\n");
    client.print("Content-Type: application/json; charset=utf-8\r\n");
    client.print(String("Content-Length: ") + str.length() + "\r\n");
    client.print("Connection: close\r\n");
    client.print("\r\n"); // end of headers

    // Send the payload.
    client.println(str);

    // Make sure it's sent.
    client.flush();
    Serial.println("[INFO] Status sent!");

    // Read the response.
    Serial.println("[INFO] Reading the response...");
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 5000) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }

    Serial.println("[INFO] Done sending status!");
  } else {
    Serial.println("[INFO] Error connecting to the server!");
    print_wifi_status();
  }
  client.stop();

  WiFi.end();
}

/// @function setup_anemometer
void setup_anemometer() {
  Serial.println("[INFO] Setting up the anemometer...");
  pinMode(A0, INPUT_PULLUP); // Enable internal pull-up resistor on pin A0
  Serial.println("[INFO] Done setting up the anemometer...");
}

/// @function setup_am2315
void setup_am2315() {
  Serial.println("[INFO] Setting up the AM2315...");

  Wire.begin();
  DHT.begin();

  delay(1000);

  Serial.println("[INFO] Done setting up the AM2315...");
}

/// @function setup
void setup() {

  // Initialize serial and wait for port to open.
  Serial.begin(9600);

  // Set the LED as output.
  pinMode(LED_BUILTIN, OUTPUT);

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
  char buff[800];
  snprintf(buff, sizeof(buff) - 1, "{\"collection\": \"patio_monitor\", \"api_key\": \"%s\", \"wind speed ms\": %f, \"temperature\": %f, \"humidity\": %f, \"moisture_sensor_1\": %f, \"moisture_sensor_2\": %f}", API_KEY, wind_speed_ms, temp_c, humidity, moisture1, moisture2);
  Serial.println(buff);

  // Send.
  post_status(buff);

  delay(600000);
}

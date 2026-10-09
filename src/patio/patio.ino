// Created by Michael Simms

#include <AM2315C.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 

#define REF_VOLTAGE 3.3
#define ADC_RESOLUTION 4095

// Anemometer (https://www.adafruit.com/product/1733)
#define MIN_ANEMOMETER_VOLTAGE 0.10 // based on experimentation
#define MAX_ANEMOMETER_VOLTAGE 2.0
#define MIN_WIND_SPEED 0.2  // meters per sec
#define MAX_WIND_SPEED 32.4 // meters per sec

#define MOISTURE_SENSOR_1 A1
#define MOISTURE_SENSOR_2 A2

// The "pin" for the onboard LED.
int LED = 13;

// The number of samples that will be averaged together for each measurement.
#define NUM_SAMPLES 10

// Function prototypes
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
float avg_buffer(float* buf, size_t len);
float read_anemometer();
void read_temperature_and_humidity_from_am2315c(float* temp_c, float* humidity);
float read_soil_moisture_sensor(pin_size_t pin);
float read_soil_moisture_sensor_1();
float read_soil_moisture_sensor_2();
void print_wifi_status();
void post_status(String str);
void setup_anemometer();
void setup_am2315c();
void dht_reinit();
void blink(int num_blinks);
void setup();

// Temperature and humidity sensor.
AM2315C DHT(&Wire);

// RED    -------- | VDD             |
// YELLOW -------- | SDA    AM2315C  |
// BLACK  -------- | GND             |
// WHITE  -------- | SCL             |

/// @function mapfloat
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/// @function avg_buffer
float avg_buffer(float* buf, size_t len) {
  float total = 0.0;
  for (int i = 0; i < len; ++i) {
    total += buf[i];
  }
  return total / (float)len;
}

/// @function read_anemometer
float read_anemometer() {
  float sensor_value = analogRead(A0);
  float voltage = (sensor_value * REF_VOLTAGE) / ADC_RESOLUTION;
  float wind_speed_ms = 0.0;

  if (voltage > MIN_ANEMOMETER_VOLTAGE) {
    wind_speed_ms = mapfloat(voltage, MIN_ANEMOMETER_VOLTAGE, MAX_ANEMOMETER_VOLTAGE, MIN_WIND_SPEED, MAX_WIND_SPEED);
  }
  return wind_speed_ms;
}

/// @function read_temperature_and_humidity_from_am2315c
void read_temperature_and_humidity_from_am2315c(float* temp_c, float* humidity) {
  int status = DHT.read();
  if (status == 0) {
    (*temp_c) = DHT.getTemperature();
    (*humidity) = DHT.getHumidity();
  }
  else {
    Serial.print("[ERROR] DHT.read() returned ");
    Serial.println(status);
  }
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

  // Connect to the client.
  Serial.println("[INFO] Establishing an SSL connection...");
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
  pinMode(A0, INPUT_PULLDOWN); // Enable internal pull-down resistor on pin A0
  Serial.println("[INFO] Done setting up the anemometer...");
}

/// @function setup_am2315c
void setup_am2315c() {
  Serial.println("[INFO] Setting up the AM2315...");

  Wire.begin();
  Wire.setClock(100000);  // 100 kHz
#if defined(WIRE_HAS_TIMEOUT)
  Wire.setWireTimeout(2500, true);
#endif

  // Use external resistors instead of the internal pullups.
  //pinMode(A4, INPUT_PULLUP);
  //pinMode(A5, INPUT_PULLUP);

  delay(5);

  if (DHT.begin()) {
    delay(1000);
  }
  else {
    Serial.println("[ERROR] Sensor not found. Check wiring!");
  }

  Serial.println("[INFO] Done setting up the AM2315...");
}

/// @function dht_reinit
void dht_reinit() {
  Wire.end();
  delay(5);
  setup_am2315c();
}

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

/// @function setup
void setup() {

  // Initialize serial and wait for port to open.
  Serial.begin(9600);
  delay(100);
  Serial.println("[INFO] Initializing....");

  // Set the LED as output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize the anemometer.
  setup_anemometer();

  // Initialize the AM2315C.
  setup_am2315c();
}

/// @function loop
void loop() {

  float buf_wind_speed_ms[NUM_SAMPLES] = {0};
  float buf_temp_c[NUM_SAMPLES] = {0};
  float buf_humidity[NUM_SAMPLES] = {0};
  float buf_moisture1[NUM_SAMPLES] = {0};
  float buf_moisture2[NUM_SAMPLES] = {0};

  for (int i = 0; i < NUM_SAMPLES; ++i) {

    // Turn the LED on.
    digitalWrite(LED, HIGH);

    // Read wind speed.
    Serial.println("[INFO] Reading wind speed...");
    float wind_speed_ms = read_anemometer();
    buf_wind_speed_ms[i] = wind_speed_ms;

    // Read temperature and humidity.
    float temp_c = 0.0;
    float humidity = 0.0;
    if (DHT.isConnected()) {
      Serial.println("[INFO] Reading temperature and humidity...");
      read_temperature_and_humidity_from_am2315c(&temp_c, &humidity);
      buf_temp_c[i] = temp_c;
      buf_humidity[i] = humidity;
    }
    else {
      Serial.println("[ERROR] The temperature and humidity sensor is not connected!");
    }

    // Read soil moisture sensor.
    Serial.println("[INFO] Reading soil moisture...");
    float moisture1 = read_soil_moisture_sensor_1();
    float moisture2 = read_soil_moisture_sensor_2();
    buf_moisture1[i] = moisture1;
    buf_moisture2[i] = moisture2;

    // Turn the LED off.
    digitalWrite(LED, LOW);

    // Wait a second.
    delay(1000);
  }

  // Calculate the averages.
  wind_speed_ms = avg_buffer(buf_wind_speed_ms, NUM_SAMPLES);
  temp_c = avg_buffer(buf_temp_c, NUM_SAMPLES);
  humidity = avg_buffer(buf_humidity, NUM_SAMPLES);
  moisture1 = avg_buffer(buf_moisture1, NUM_SAMPLES);
  moisture2 = avg_buffer(buf_moisture2, NUM_SAMPLES);

  // Success! Blink the LED to show that we're happy.
  blink(3);

  // Format the output.
  char buff[800];
  if (DHT.isConnected()) {
    snprintf(buff, sizeof(buff) - 1, "{\"collection\": \"patio_monitor\", \"api_key\": \"%s\", \"wind speed ms\": %f, \"temperature\": %f, \"humidity\": %f, \"moisture_sensor_1\": %f, \"moisture_sensor_2\": %f}", API_KEY, wind_speed_ms, temp_c, humidity, moisture1, moisture2);
  }
  else {
    snprintf(buff, sizeof(buff) - 1, "{\"collection\": \"patio_monitor\", \"api_key\": \"%s\", \"wind speed ms\": %f, \"moisture_sensor_1\": %f, \"moisture_sensor_2\": %f}", API_KEY, wind_speed_ms, moisture1, moisture2);
  }
  Serial.println(buff);

  // Send.
  post_status(buff);

  // Success! Blink the LED to show that we're happy.
  blink(5);

  // Re-init I2C on wake from sleep.
  //dht_reinit();
}

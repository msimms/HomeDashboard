// Created by Michael Simms

#include <AM2315C.h>
#include <SPI.h>
#include <NTPClient.h>
#include <RTC.h>
#include <Wire.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include "Arduino_LED_Matrix.h" // Include the LED_Matrix library
#include "arduino_secrets.h" 

#define REF_VOLTAGE 5.0
#define ADC_RESOLUTION 16383

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

// Temperature and humidity sensor.
AM2315C* g_DHT = NULL;

// Create an instance of the ArduinoLEDMatrix class.
ArduinoLEDMatrix g_matrix;

// SSL
WiFiSSLClient g_ssl;
HttpClient g_client(g_ssl, STATUS_URL, STATUS_PORT);

// NTP
WiFiUDP g_udp;
NTPClient g_time_client(g_udp, "pool.ntp.org", 0, 60000); // Update every 60s

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
  if (g_DHT) {
    int status = g_DHT->read();
    if (status == 0) {
      (*temp_c) = g_DHT->getTemperature();
      (*humidity) = g_DHT->getHumidity();
    }
    else {
      Serial.print("[ERROR] g_DHT->read() returned ");
      Serial.println(status);
    }
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

/// @function setup_wifi
void setup_wifi() {

  // Check the firmware version.
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("[INFO] Upgrade the firmware!");
  }

  // Make sure we were given a network to connect to.
  if (strlen(SECRET_SSID) == 0) {
    Serial.println("[ERROR] SSID not specified!");
    return;
  }

  // (Re)connect to Wifi.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[INFO] Connecting to WiFi...");
    Serial.print("[INFO] SSID: ");
    Serial.println(SECRET_SSID);

    WiFi.begin(SECRET_SSID, SECRET_PASS);

    // Wait until we're connected.
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - t0) < 15000) {
      delay(250);
      Serial.print(".");
    }
    Serial.println();

    // Sync RTC.
    Serial.println("[INFO] Syncing time...");
    g_time_client.begin();
    g_time_client.update();
    unsigned long epoch_time = g_time_client.getEpochTime();
    RTCTime rtc_time(epoch_time);
    RTC.setTime(rtc_time);

    Serial.println("[INFO] RTC synced with NTP!");
  }
  else {
    Serial.println("[INFO] Already connected to WiFi!");
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
  Serial.print("[INFO] Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}


/// @function post_status
void post_status(String body) {

  // (Re)connect to Wifi.
  setup_wifi();

  // Are we connected?
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] WiFi not connected.");
    Serial.println(WiFi.status());
    return;
  }

  // Did we get an IP address from the DHCP server?
  Serial.println("[INFO] WiFi connected!");
  Serial.print("[INFO] IP: ");
  Serial.println(WiFi.localIP());

  // Print the time.
  RTCTime current_time;
  RTC.getTime(current_time);
  Serial.print("[INFO] Current time: ");
  Serial.println(String(current_time.getHour()) + ":" + String(current_time.getMinutes()) + ":" + String(current_time.getSeconds()));

  // Start an HTTP session.
  g_client.setHttpResponseTimeout(60000);
  Serial.println("[INFO] Sending HTTPS POST...");
  int err = g_client.post("/api/1.0/update_status", "application/json", body);
  Serial.print("post() returned: ");
  Serial.println(err);

  int statusCode = g_client.responseStatusCode();
  Serial.print("[INFO] HTTP Status Code: ");
  Serial.println(statusCode);

  String response = g_client.responseBody();
  Serial.print("[INFO] HTTP Response Body: ");
  Serial.println(response);

  g_client.stop();
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

  // Use external resistors instead of the internal pullups.
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);

  delay(5);

  g_DHT = new AM2315C(&Wire);
  if (g_DHT) {
    //g_DHT->resetSensor();

    if (g_DHT->begin()) {
      Serial.println("[INFO] AM2315C sensor started!");
      delay(1000);
    }
    else {
      Serial.println("[ERROR] AM2315C sensor not found. Check wiring!");
    }
  }

  Serial.println("[INFO] Done setting up the AM2315C...");
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
  // Wait for the serial interface.
  while (!Serial) {
    delay(10);
  }
  Serial.println("[INFO] Initializing....");

  // Set the LED as output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize the I2C interface.
  Wire.begin();

  // Start the real time clock.
  RTC.begin();

  // Initialize the anemometer.
  setup_anemometer();

  // Initialize the AM2315C.
  setup_am2315c();

  // Initialize the UNO R4's built-in display.
  g_matrix.begin();

  // Connect to the wifi network.
  setup_wifi();

  // Make the onboard LED panel display a happy face.
  g_matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
}

/// @function loop
void loop() {

  // Average several samples together.
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
    if (g_DHT && g_DHT->isConnected()) {
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
  float wind_speed_ms = avg_buffer(buf_wind_speed_ms, NUM_SAMPLES);
  float temp_c = avg_buffer(buf_temp_c, NUM_SAMPLES);
  float humidity = avg_buffer(buf_humidity, NUM_SAMPLES);
  float moisture1 = avg_buffer(buf_moisture1, NUM_SAMPLES);
  float moisture2 = avg_buffer(buf_moisture2, NUM_SAMPLES);

  // Success! Blink the LED to show that we're happy.
  blink(3);

  // Format the output.
  char buff[800];
  if (g_DHT->isConnected()) {
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
}

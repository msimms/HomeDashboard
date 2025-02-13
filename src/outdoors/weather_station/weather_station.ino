#include <AM2315C.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 

#define MIN_VOLTAGE 0.4

// Wifi
char ssid[] = SECRET_SSID;            // your network SSID (name)
char pass[] = SECRET_PASS;            // your network password (use for WPA, or use as key for WEP)
int wifiStatus = WL_IDLE_STATUS;      // the Wi-Fi radio's status

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
void read_anemometer() {
  Serial.println("Reading wind speed...");

  float sensorValue = analogRead(A0);
  Serial.print("Analog Value: ");
  Serial.println(sensorValue);

  float voltage = (sensorValue / 1024) * 5;
  Serial.print(voltage);
  Serial.println(" V");

  float wind_speed = 0.0;
  if (voltage >= MIN_VOLTAGE) {
    wind_speed = mapfloat(voltage, 0.4, 2, 0, 32.4);
  }

  float speed_mph = ((wind_speed * 3600) / 1609.344);
  Serial.print("Wind Speed: ");
  Serial.print(wind_speed);
  Serial.print(" m/s (");
  Serial.print(speed_mph);
  Serial.println(" mph)");
}

/// @function read_temperature_and_humidity_from_am2315
void read_temperature_and_humidity_from_am2315c() {
  Serial.println("Reading temperature and humidity...");

  int status = DHT.read();
  float temperature = DHT.getTemperature();
  float humidity = DHT.getHumidity();

  // Format the output.
  char buff[32];
  snprintf(buff, sizeof(buff) - 1, "%f\t%f", temperature, humidity);

  Serial.println(buff);
}

/// @function setup_wifi
void setup_wifi() {
  Serial.println("Setting up Wifi...");

  // Set the LED as output.
  pinMode(LED_BUILTIN, OUTPUT);

  // attempt to connect to Wi-Fi network:
  while (wifiStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    wifiStatus = WiFi.begin(ssid, pass);

    // Wait 10 seconds for connection.
    delay(10000);
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
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize the anemometer.
  //setup_anemometer();

  // Initialize the AM2315.
  setup_am2315();
}

/// @function loop
void loop() {
  // Turn the LED on.
  digitalWrite(LED, HIGH);

  //read_anemometer();
  read_temperature_and_humidity_from_am2315c();

  // Turn the LED off.
  digitalWrite(LED, LOW);

  delay(10000);
}

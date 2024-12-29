#include <DHT11.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 

#define DHT11_PIN 7
#define MIN_VOLTAGE 0.4
#define AM2315_I2CADDR 0x5C
#define AM2315_READREG 0x03

char ssid[] = SECRET_SSID;            // your network SSID (name)
char pass[] = SECRET_PASS;            // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;          // the Wi-Fi radio's status
int ledState = LOW;                   // ledState used to set the LED
unsigned long previousMillisInfo = 0; // will store last time Wi-Fi information was updated
unsigned long previousMillisLED = 0;  // will store the last time LED was updated
const int intervalInfo = 5000;        // interval at which to update the board information

DHT11 dht11(DHT11_PIN);

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

/// @function read_temperature_and_humidity_from_dht11
void read_temperature_and_humidity_from_dht11() {
  Serial.println("Reading temperature and humidity...");

  int temperature = 0;
  int humidity = 0;

  // Attempt to read the temperature and humidity values from the DHT11 sensor.
  int result = dht11.readTemperatureHumidity(temperature, humidity);

  // Check the results of the readings.
  // If the reading is successful, print the temperature and humidity values.
  // If there are errors, print the appropriate error messages.
  if (result == 0) {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C\tHumidity: ");
      Serial.print(humidity);
      Serial.println(" %");
  } else {
      // Print error message based on the error code.
      Serial.println(DHT11::getErrorString(result));
  }
}

/// @function read_temperature_and_humidity_from_am2315
void read_temperature_and_humidity_from_am2315() {
  Serial.println("Reading temperature and humidity...");

  float temperature, humidity;

  uint8_t reply[10];
  uint8_t sreply[10];

  // Wake up the sensor.
  Serial.println("A");
  Wire.beginTransmission(AM2315_I2CADDR);
  delay(2);
  uint8_t end1 = Wire.endTransmission();

  // Send the read command.
  Wire.beginTransmission(AM2315_I2CADDR);
  uint8_t write1 = Wire.write(AM2315_READREG);
  uint8_t write2 = Wire.write(0x00);  // start at address 0x0
  uint8_t write3 = Wire.write(4);  // request 4 bytes data
  uint8_t end2 = Wire.endTransmission();
  
  // Add delay between request and actual read.
  delay(10);

  // Read the response.
  uint8_t request1 = Wire.requestFrom(AM2315_I2CADDR, 8);
  for (uint8_t i=0; i<8; i++) {
    reply[i] = Wire.read();
    sreply[i] = reply[i];
  }
  
  if (reply[0] != AM2315_READREG && reply[1] != 4) {
  }
}

/// @function setup_wifi
void setup_wifi() {
  Serial.println("Setting up Wifi...");

  // Set the LED as output.
  pinMode(LED_BUILTIN, OUTPUT);

  // attempt to connect to Wi-Fi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

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

  // Configure the internal pullup resistors.
  pinMode(A4, INPUT_PULLUP); // Enable internal pull-up resistor on pin A4
  pinMode(A5, INPUT_PULLUP); // Enable internal pull-up resistor on pin A5

  // Wake up the AM2315.
  Wire.beginTransmission(AM2315_I2CADDR);
  Wire.write(AM2315_READREG);
  Serial.println("0");
  Wire.endTransmission();
  Serial.println("0");

  // Add delay between request and actual read.
  delay(50);
  Serial.println("0");

  Wire.beginTransmission(AM2315_I2CADDR);
  Wire.write(AM2315_READREG);
  Wire.write(0x00); // start at address 0x0
  Wire.write(4); // request 4 bytes data
  Wire.endTransmission();
  Serial.println("0");

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
  setup_anemometer();

  // Initialize the AM2315.
  //setup_am2315();
}

/// @function loop
void loop() {
  read_anemometer();
  //read_temperature_and_humidity_from_am2315();
  delay(5000);
}

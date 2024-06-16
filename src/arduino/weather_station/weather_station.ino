#include <DHT11.h>
#include <SPI.h>
#include <WiFi.h>

#define HAS_WIFI 0
#define DHT11_PIN 7
#define MIN_VOLTAGE 0.4

char ssid[] = "yourNetwork"; //  your network SSID (name)
char pass[] = "secretPassword"; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0; // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
char server[] = "www.google.com";    // name address for Google (using DNS)

WiFiClient client;
DHT11 dht11(DHT11_PIN);

void setup() {
  Serial.println("Initializing...");
  Serial.begin(9600);

  // Wait for the serial port to connect.
  while (!Serial) {
    ;
  }

  if (HAS_WIFI) {
    // Is the Wifi shield available?
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      while (true);
    }

    // Check the Wifi firmware version.
    String fv = WiFi.firmwareVersion();
    if (fv != "1.1.0") {
      Serial.println("Please upgrade the firmware");
    }

    // If not connected to Wifi, then attempt to connect.
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);

      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);

      // Wait 10 seconds for connection.
      delay(10000);
    }

    Serial.println("Connected to wifi");
    printWifiStatus();
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

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

  float speed_mph = ((wind_speed * 3600)/1609.344);
  Serial.print("Wind Speed: ");
  Serial.print(wind_speed);
  Serial.println("m/s");
  Serial.print(speed_mph);
  Serial.println("mph");
}

void read_temperature() {
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

void post_to_web() {
  Serial.println("Posting...");

}

void loop() {
  read_anemometer();
  read_temperature();
  post_to_web();
  delay(5000);
}


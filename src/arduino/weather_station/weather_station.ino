#include <DHT11.h>

#define DHT11_PIN 7

DHT11 dht11(DHT11_PIN);

void setup() {
  Serial.println("Initializing...");
  Serial.begin(9600);
  delay(1000); // Wait before accessing sensors
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void read_anemometer() {
  Serial.println("Reading wind speed...");

  float sensorValue = analogRead(A0);
  Serial.print("Analog Value: ");
  Serial.println(sensorValue);
 
  float voltage = (sensorValue / 1024) * 5;
  Serial.print(voltage);
  Serial.println(" V");
 
  float wind_speed = mapfloat(voltage, 0.4, 2, 0, 32.4);
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
  Serial.println("Starting...");
  read_anemometer();
  read_temperature();
  post_to_web();
  delay(1000);
}

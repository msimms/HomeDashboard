#define DHT11_PIN 7

dht DHT;

void setup() {
  Serial.begin(9600);
  delay(1000); // Wait before accessing sensors
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void read_anemometer() {
  float sensorValue = analogRead(A0);
  Serial.print("Analog Value =");
  Serial.println(sensorValue);
 
  float voltage = (sensorValue / 1024) * 5;
  Serial.print(voltage);
  Serial.println(" V");
 
  float wind_speed = mapfloat(voltage, 0.4, 2, 0, 32.4);
  float speed_mph = ((wind_speed *3600)/1609.344);
  Serial.print("Wind Speed =");
  Serial.print(wind_speed);
  Serial.println("m/s");
  Serial.print(speed_mph);
  Serial.println("mph");
}

void read_temperature() {
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);
}

void post_to_web() {

}

void loop() {
  read_anemometer();
  read_temperature();
  post_to_web();
  delay(1000);
}

# Home Dashboard

This is a collection of projects for monitoring things around the house. In other words, a smart home that lives on your own infrastructure. The motivation for sharing this is so that anyone that wants to build their own home dashboard (web app and sensors) can use this to get started. Nothing here currently has the ability to integrate with Home Assistant, or anything similar. That may change in the future.

## Indoor Air Quality Microcontroller

This measures CO<sup>2</sup>, temperature, humidity, VOC, and VOC index and logs it to a database.

### Hardware

* Arudino Nano (ATMega328P)
* Adafruit SCD-41 - True CO2 Temperature and Humidity Sensor
* Adafruit SGP30 Air Quality Sensor - https://www.adafruit.com/product/3709
* MakerFocus 0.91 Inch I2C IIC OLED Display Module 128 x 32 Pixel - https://www.amazon.com/dp/B079BN2J8V

### Wiring

Pin  |
---- | ----
GND  | Sensor 1 GND -> Sensor 2 GND -> Screen GND
5VDC | Sensor 1 Power -> Sensor 2 Power -> Screen Power
A4   | Sensor 1 SDA -> Sensor 2 SDA -> Screen SDA
A5   | Sensor 1 SCL -> Sensor 2 SCL -> Screen SCL

### Software

Build and upload using the Arduino IDE. The target microprocessor is an Arudino Nano (ATMega328P).

Install the required libraries from within the Arduino IDE by going to Sketch > Include Library > Manage Libraries.
* Search for "SensirionI2CScd4x" and install the library.

## Patio Microcontroller (Arduino Nano Implementation)

This measures wind speed, temperature, humidity, and soil moisture. Measurements are sent to the web app so they can be logged to the database and viewed later.

### Hardware 

* Arduino Nano 33 IoT - https://www.amazon.com/dp/B07WPFQZQ1
* Anemometer - https://www.adafruit.com/product/1733
* Temperature & Humidity - https://www.adafruit.com/product/5182
* Moisture Sensors - https://www.amazon.com/dp/B0C6XGRKS5

### Wiring

Pin  |
---- | ----
A0   | Anemometer voltage
A1   | Soil sensor 1 voltage
A2   | Soil sensor 2 voltage
A4   | AM2315C SDA
A5   | AM2315C SCL
GND  | Tie all grounds together

### Software

Copy arduino_secrets_example.h to arduino_secrets.h and fill in the values for your setup.

Build and upload using the Arduino IDE. The target board is the Arduino Nano 33 IoT.

Install the required libraries from within the Arduino IDE by going to Sketch > Include Library > Manage Libraries.
* Search for "AM2315C" and install the library.

## Patio Microcontroller (Arduino UNO R4 WiFi)

This measures wind speed, temperature, humidity, and soil moisture. Measurements are sent to the web app so they can be logged to the database and viewed later.

### Hardware 

* Arduino UNO R4 WiFi - https://www.amazon.com/Arduino-UNO-WiFi-ABX00087-Bluetooth/dp/B0C8V88Z9D
* Anemometer - https://www.adafruit.com/product/1733
* Temperature & Humidity - https://www.adafruit.com/product/5182
* Moisture Sensors - https://www.amazon.com/dp/B0C6XGRKS5

### Wiring

Pin  |
---- | ----
A0   | Anemometer voltage
A1   | Soil sensor 1 voltage
A2   | Soil sensor 2 voltage
SDA  | AM2315C SDA
SDC  | AM2315C SCL
GND  | Tie all grounds together

### Software

Copy arduino_secrets_example.h to arduino_secrets.h and fill in the values for your setup.

Build and upload using the Arduino IDE. The target board is the Arduino Nano 33 IoT.

Install the required libraries from within the Arduino IDE by going to Sketch > Include Library > Manage Libraries.
* Search for "AM2315C" and install the library.
* Search for "NTPClient" and install the library.
* Search for "ArduinoHttpClient" and install the library.

## Scale

### Hardware

* Arduino UNO R4 WiFi - https://www.amazon.com/Arduino-UNO-WiFi-ABX00087-Bluetooth/dp/B0C8V88Z9D
* 4x HX711

### Wiring

Pin    |
------ | ------
GND    | Sensor GND
D4     | DT1
D5     | SCK1
D6     | DT2
D7     | SCK2
D8     | DT3
D9     | SCK3
D10    | DT4
D11    | SCK4
GND    | Screen GND
3.3VDC | Screen Power
A4     | Screen SDA
A5     | Screen SCL

### Enclosure

TODO

### Software

Copy arduino_secrets_example.h to arduino_secrets.h and fill in the values for your setup.

Install the required libraries from within the Arduino IDE by going to Sketch > Include Library > Manage Libraries.
* Search for "HX711" and install the library.
* Search for "ArduinoHttpClient" and install the library.

## Thermocouple

### Hardware

* ESP8266 ESP-12 ESP-12F
* Kiro&Seeu DC 3-5V MAX6675 Module + K Type Thermocouple

### Wiring

ESP8266 Pin | MAX6675 Pin
----------- | -----------
3.3VDC      | VCC
GND         | GND
D6          | 12
D1          | 4
D5          | 14

### Software

Copy arduino_secrets_example.h to arduino_secrets.h and fill in the values for your setup.

## Dashboard App

This exists so for integration with the phone's notification, so the user can get alerts about out-of-range values, etc.

### Building

This app is built using Apple XCode. Every attempt is made to stay up-to-date with the latest version of XCode and the latest versions of iOS and macOS. In theory, if you have cloned the source code repository and initialized the submodules, then you should be able to open the project in XCode, build, and deploy.

## Dashboard Website

Intended to be the primary method for viewing data, and also managing the API keys used by the sensors.

### Building

An alternative to the app is a dashboard website, built using python and flask.

## Version History

None - this is a perpetual work in progress

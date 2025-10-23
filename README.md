# Home Dashboard

A collection of projects for monitoring things around the house. In other words, a smart home that lives on your own infrastructure. The motivation for sharing this is so that someone that wants to build their own home dashboard (web app and sensors) can use this to get started.

## Indoor Air Quality Microcontroller

This measures CO<sup>2</sup>, temperature, humidity, VOC, and VOC index and reports it to the web app.

### Hardware

* Arudino Nano (ATMega328P)
* Adafruit SCD-41 - True CO2 Temperature and Humidity Sensor
* Adafruit SGP30 Air Quality Sensor - https://www.adafruit.com/product/3709
* MakerFocus 0.91 Inch I2C IIC OLED Display Module 128 x 32 Pixel - https://www.amazon.com/dp/B079BN2J8V

### Wiring

Pin | |
-- | -- |
GND | Sensor 1 GND -> Sensor 2 GND -> Screen GND
5VDC | Sensor 1 Power -> Sensor 2 Power -> Screen Power
A4 | Sensor 1 SDA -> Sensor 2 SDA -> Screen SDA
A5 | Sensor 1 SCL -> Sensor 2 SCL -> Screen SCL

### Software

Build and upload using the Arduino IDE. The target microprocessor is an Arudino Nano (ATMega328P).

Install the required libraries from within the Arduino IDE by going to Sketch > Include Library > Manage Libraries.
* Search for "SensirionI2CScd4x" and install the library.

## Patio Microcontroller

This measures CO<sup>2</sup>, temperature, humidity, and soil moisture. Measurements are sent to the web app.

### Hardware

* Arduino Nano 33 IoT - https://www.amazon.com/dp/B07WPFQZQ1
* Adafruit SCD-41 - True CO2 Temperature and Humidity Sensor
* Moisture Sensors - https://www.amazon.com/dp/B0C6XGRKS5

### Wiring

Pin | |
-- | -- |

### Software

Copy arduino_secrets_example.h to arduino_secrets.h and fill in the values for your setup.

Build and upload using the Arduino IDE. The target board is the Arduino Nano 33 IoT.

## Scale

### Hardware

* Arudino Uno R4 WiFi
* 4x HX711

### Wiring

Pin | |
-- | -- |
GND | Sensor GND
D4 | DT1
D5 | SCK1
D6 | DT2
D7 | SCK2
D8 | DT3
D9 | SCK3
D10 | DT4
D11 | SCK4
GND | Screen GND
3.3VDC | Screen Power
A4 | Screen SDA
A5 | Screen SCL

### Enclosure

TODO

### Software

Copy arduino_secrets_example.h to arduino_secrets.h and fill in the values for your setup.

## Thermocouple

### Hardware

* ESP8266 ESP-12 ESP-12F
* Kiro&Seeu DC 3-5V MAX6675 Module + K Type Thermocouple

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

None - still in development

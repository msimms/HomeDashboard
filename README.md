# Home Dashboard

A collection of projects for monitoring things around the house. The motivation for sharing this is so that someone that wants to build their own home dashboard (web app and sensors) can use this to get started.

## Indoor Air Quality Microcontroller Code

This measures CO2, temperature, humidity, VOC, and VOC index.

### Building

Build and upload using the Arduino IDE. The target microprocessor is an Arudino Nano (ATMega328P).

Install the required libraries from within the Arduino IDE by going to Sketch > Include Library > Manage Libraries.
* Search for "SensirionI2CScd4x" and install the library.

## Patio Microcontroller Code

This measures CO2, temperature, humidity, and soil moisture.

### Building

Build and upload using the Arduino IDE. The target board is the Arduino Nano 33 IoT.

## Dashboard App

### Building

This app is built using Apple XCode. Every attempt is made to stay up-to-date with the latest version of XCode and the latest versions of iOS and macOS. In theory, if you have cloned the source code repository and initialized the submodules, then you should be able to open the project in XCode, build, and deploy.

## Dashboard Website

### Building

An alternative to the app is a dashboard website, built using python and flask.

## Version History

None - still in development

# Scale

This is a work-in-progress. Just a simple system for measuring the contents of my homebrew keg, by weight, and reporting it to the dashboard via the REST API.

## Build the Scale

### Print the Scale Enclosure

The CAD file is provided in the repository. It should be printed using a material strong enough to hold the weight of the keg.

### Scale Hardware

Load Cells: https://www.adafruit.com/product/4543

HX711 Load Cell Amps: https://www.amazon.com/dp/B07SGPX7ZH

### Mount the load cells and load cell amplifiers

Attach the load cell wires to the terminal block.
TODO - add pic

## Build the Scale Firmware (Arduino Nano 33 IoT with Headers)

### Hardware

https://www.amazon.com/Arduino-UNO-WiFi-ABX00087-Bluetooth/dp/B0C8V88Z9D

### Add support for the board to the Arduino IDE:

Tools -> Board -> Booard Manager -> Install Arduino SAMD Boards

### Add supporting libraries to the Arduino IDE:

Tools -> Manager Libraries
Install the following libraries:
* Adafruit SSD1306 (for the screen)
* HX711 Arduno Library by Bogdan Necula

### Build and Upload the Firmware

Sketch -> Upload

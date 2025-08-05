// Created by Michael Simms

#include <Wire.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduCAM.h>
#include "arduino_secrets.h"
#include "memorysaver.h"
#include "OV5642_regs.h"

#define CS_PIN 2
ArduCAM myCAM(OV5642, CS_PIN);

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Wait for serial.
  while (!Serial)
    ;

  // Initialize control pins.
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  delay(100);

  // Initialize buses.
  Wire.begin(11, 12);  // SDA = 11, SCL = 12
  SPI.begin();  // SPI

  // Scan I2C.
  Serial.println("I2C scanner running...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Device found at 0x");
      Serial.println(addr, HEX);
    }
  }

  // Initialize the camera.
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
  delay(1000);

  // Read and verify the sensor ID.
  Serial.println(F("Starting ArduCAM OV5642 Test..."));
  uint8_t vid = 0, pid = 0;
  bool success = true;

  // Switch to register bank 1.
  success &= (myCAM.wrSensorReg8_8(0xFF, 0x01) == 0);

  // Read the sensor ID registers.
  success &= (myCAM.rdSensorReg8_8(0x0A, &vid) == 0);
  success &= (myCAM.rdSensorReg8_8(0x0B, &pid) == 0);

  Serial.print(F("Camera VID: 0x")); Serial.println(vid, HEX);
  Serial.print(F("Camera PID: 0x")); Serial.println(pid, HEX);

  if (!success) {
    Serial.println(F("❌ I2C communication failed (check SDA/SCL and pullups)"));
    while (1)
      ;
  }

  if (vid == 0x56 && pid == 0x42) {
    Serial.println(F("✅ OV5642 camera detected successfully!"));
  } else {
    Serial.println(F("❌ OV5642 camera NOT detected. Check wiring, power, or sensor damage."));
    while (1)
      ;
  }

  // Connect to WiFi.
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        Serial.println("Reading from client...");
        String req = client.readStringUntil('\r');
        client.read(); // Consume \n

        if (req.indexOf("GET /") != -1) {

          // Take a photo.
          Serial.println("Taking photo...");
          myCAM.flush_fifo();
          myCAM.clear_fifo_flag();
          myCAM.start_capture();
          while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
            ;

          uint32_t length = myCAM.read_fifo_length();
          if (length >= MAX_FIFO_SIZE) {
            Serial.println("MAX_FIFO_SIZE exceeded!");
            Serial.println(length);
            Serial.println(MAX_FIFO_SIZE);
            client.println("HTTP/1.1 500 OK");
            client.println("Connection: close");
            client.println();
          }
          else {
            Serial.println("Returning response...");
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: image/jpeg");
            client.println("Connection: close");
            client.println();

            myCAM.CS_LOW();
            myCAM.set_fifo_burst();
            for (uint32_t i = 0; i < length; i++) {
              client.write(SPI.transfer(0x00));
            }
            myCAM.CS_HIGH();
          }
          myCAM.clear_fifo_flag();
          Serial.println("Done.");
        }

        break;
      }
    }
    delay(1);
    client.stop();
  }
}

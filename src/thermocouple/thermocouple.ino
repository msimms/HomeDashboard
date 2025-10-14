// MIT License
//
// Copyright (c) 2025 Mike Simms
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <max6675.h>

// Pin mapping (change if you wired differently)
const int PIN_SO   = 12; // GPIO12 (D6)
const int PIN_CS   = 5;  // GPIO5  (D1)
const int PIN_SCK  = 14; // GPIO14 (D5)

MAX6675 tc(PIN_SCK, PIN_CS, PIN_SO);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("MAX6675 + ESP8266 ready");
}

void loop() {
  double c = tc.readCelsius();   // ~220 ms conversion latency internally
  if (isnan(c)) {
    Serial.println("Thermocouple missing/open!");
  } else {
    double f = c * 9.0 / 5.0 + 32.0;
    Serial.printf("Temp: %.2f °C (%.2f °F)\n", c, f);
  }
  delay(500); // ~2 Hz updates (safe for MAX6675)
}

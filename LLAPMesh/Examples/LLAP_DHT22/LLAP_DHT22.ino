/*

  LLAP Protocol Implementation Library for Arduino
  Copyright (C) 2013-2014 by Xose Pérez <xose dot perez at gmail dot com>
  http://tinkerman.eldiariblau.net

  Based on the same name library by Ciseco (https://github.com/CisecoPlc/LLAPSerial)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <LLAPSerial.h>
#include <DHT22.h>

#define DEVICE_ID "WS"
#define DHT_PIN 2

// DHT22 connections:
// Connect pin 1 (on the left) of the sensor to 3.3V
// Connect pin 2 of the sensor to whatever your DHT_PIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT22 dht(DHT_PIN);
LLAPSerial LLAP(Serial);

void setup() {

    // This should match your radio baud rate
    Serial.begin(115200);

    // This device has a static ID
	// Make sure this ID is not already in use,
	// or if you are using the same LLAP library for the coordinator
	// use an ID outside the dynamic range it manages
    LLAP.begin(DEVICE_ID);

}

void loop() {

    static unsigned long lastTime = millis();

    if (millis() - lastTime >= 10000) {

        lastTime = millis();

        DHT22_ERROR_t errorCode = dht.readData();
        if (errorCode == DHT_ERROR_NONE) {
            float t = dht.getTemperatureC();
            float h = dht.getHumidity();
            LLAP.sendMessage("HUM", h, 1);
            LLAP.sendMessage("TMP", t, 1);
        } else {
            LLAP.sendMessage("ERROR", (int) errorCode);
        }

    }

}

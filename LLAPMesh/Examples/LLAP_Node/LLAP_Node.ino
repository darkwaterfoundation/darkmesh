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

LLAPSerial LLAP(Serial);

#define INTERVAL_MESSAGES 3000

void setup() {

    Serial.begin(9600);

    // begin() accepts a boolean or a char* as a parameter.
    // The default value is boolean false, which means that we want to reuse the previous ID, which is stored in EEPROM
    // begin(true) means we want the coordinator to assign this node a new value
    // Finally, begin("ZZ") means we want to assing this node the static ID "ZZ"
    LLAP.begin();

}

void loop() {

    static int i = 0;
    static unsigned long timeout = millis() + INTERVAL_MESSAGES;

    // You will need this if AUTO_SERIAL_EVENT is not defined
    // and your node needs to receive messages or uses dynamic addressing (like this one)
    LLAP.SerialEvent();

	// Avoid using "delay" as it will interfere with serial data
    if (timeout < millis()) {
        LLAP.sendMessage("VAL", i++);
        timeout = millis() + INTERVAL_MESSAGES;
    }

}


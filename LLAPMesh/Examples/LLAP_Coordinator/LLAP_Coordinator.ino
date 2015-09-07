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

void setup() {

    Serial.begin(9600);

    // Set the node as coordinator
    LLAP.setCoordinator(true);

    // This is unnecessary, default coordinator ID is AA
    LLAP.begin("AA");

}

void loop() {

    // You will need this if AUTO_SERIAL_EVENT is not defined
    // and your node needs to receive messages (like this one) or uses dynamic addressing
    LLAP.SerialEvent();

    if (LLAP.available()) {
        Serial.print("Message from ");
        Serial.print(LLAP.getRemoteID());
        Serial.print(": ");
        Serial.println(LLAP.getBody());
    }

}





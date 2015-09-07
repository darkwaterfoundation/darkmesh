/*
  LLAP Mesh implementation library for Arduino
  By @shrkey for Dark Water Foundation

  Based on:

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

#ifndef _LLAPMESH_H

    #define _LLAPMESH_H

    #include <Arduino.h>
    #include <Stream.h>

    class LLAPMesh {

        private:

            Stream & _Serial;

            boolean _available;
            boolean _acknowledged;
            boolean _coordinator;
            boolean _rebootable;

            char _raw[13];
            char _localID[3];
            char _remoteID[3];
            char _body[10];

            void processMessage();
            void setLocalID(char *);
            void saveAddress(char *);
            void loadAddress();
            char * getNewAddress();

        public:

            LLAPMesh(Stream & serial);

            boolean available();
            boolean acknowledged();

            void SerialEvent();

            void setUART(Stream & serial);
            void reset();
            void begin(char * id);
            void begin(boolean newAddress = false);
            void setCoordinator(boolean value);
            char * getLocalID();
            char * getRemoteID();
            char * getBody();

            void sendMessage(char * key, boolean broadcast = false);
            void sendMessage(char * key, char * remoteID);
            void sendMessage(char * key, char * value, boolean broadcast = false);
            void sendMessage(char * key, char * value, char * remoteID);
            void sendMessage(const __FlashStringHelper * key, char * value, char * remoteID);
            void sendMessage(const __FlashStringHelper * key, char * value, boolean broadcast = false);
            void sendMessage(const __FlashStringHelper * key, boolean broadcast = false);
            void sendMessage(char * key, int value, boolean broadcast = false);
            void sendMessage(char * key, int value, char * remoteID);
            void sendMessage(char * key, float value, byte precision, boolean broadcast = false);
            void sendMessage(char * key, float value, byte precision, char * remoteID);

    };

    extern LLAPMesh LLAP;

#endif

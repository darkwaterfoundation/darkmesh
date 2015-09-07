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
#include <LLAPMesh.h>
#include <EEPROM.h>

#define EEPROM_ID_ADDRESS 500
#define DYNAMIC_ADDRESSES_START 'B'
#define DYNAMIC_ADDRESSES 104 // 26 address per character, from BA to EZ
//#define AUTO_SERIAL_EVENT

void(* reboot) (void) = 0;

/**
 * LLAPSerial constructor
 *
 * @param Stream & serial Serial object to use to send messages
 * @return LLAPSerial
 */
LLAPMesh::LLAPMesh(Stream & serial): _Serial(serial) {
    _available = false;
    _acknowledged = false;
    _coordinator = false;
    _rebootable = false;
    _localID[0] = _localID[1] = '-';
}

/**
 * setLocalID
 * Sets a new device ID.
 *
 * @param char * id 2 byte string [A-Z each]
 */
void LLAPMesh::setLocalID(char * id) {
    _localID[0] = id[0];
    _localID[1] = id[1];
    _localID[2] = 0;
    saveAddress(_localID);
}

/**
 * loadAddress
 * Recovers address from EEPROM
 */
void LLAPMesh::loadAddress() {
    _localID[0] = EEPROM.read(EEPROM_ID_ADDRESS);
    _localID[1] = EEPROM.read(EEPROM_ID_ADDRESS + 1);
    if (_localID[0] < 'A' || 'Z' < _localID[0] ||
        _localID[1] < 'A' || 'Z' < _localID[1]) {
        _localID[0] = _localID[1] = '-';
        saveAddress(_localID);
    }
}

/**
 * saveAddress
 * Stores current address into EEPROM
 */
void LLAPMesh::saveAddress(char * id) {
    EEPROM.write(EEPROM_ID_ADDRESS, id[0]);
    EEPROM.write(EEPROM_ID_ADDRESS + 1, id[1]);
}

/**
 * getNewAddress
 * Gets a free address for a client and updates pointer
 *
 * @return char * 2 byte string [A-Z each]
 */
char * LLAPMesh::getNewAddress() {
    uint8_t id = EEPROM.read(EEPROM_ID_ADDRESS + 2);
    char address[3]  = {0};
    address[0] = (char) (id / ('Z' - 'A' + 1)) + DYNAMIC_ADDRESSES_START;
    address[1] = (char) (id % ('Z' - 'A' + 1)) + 'A';
    EEPROM.write(EEPROM_ID_ADDRESS + 2, (id + 1) % DYNAMIC_ADDRESSES);
    return address;
}

/**
 * reset
 * Reset address pool
 */
void LLAPMesh::reset() {
    EEPROM.write(EEPROM_ID_ADDRESS + 2, 0);
}

/**
 * begin
 * Gets the device ID and sends STARTED message.
 *
 * @param boolean newAddress wether to load address from EEPROM or not
 */
void LLAPMesh::begin(boolean newAddress) {
    if (!newAddress) {
        _rebootable = true;
        loadAddress();
    }
    if (_localID[0] == '-' && _localID[1] == '-') {
        uint8_t counter = 0;
        _acknowledged = false;
        while (!_acknowledged && counter<5) {
            sendMessage(F("STARTED"));
            delay(200);
            SerialEvent();
            counter++;
        }
    } else {
        sendMessage(F("STARTED"));
    }
}

/**
 * begin
 * Sets a new device ID and sends STARTED message.
 *
 * @param char * id 2 byte string [A-Z each]
 */
void LLAPMesh::begin(char * id) {
    setLocalID(id);
    begin(true);
}

/**
 * getLocalID
 * Retrieves a new device ID.
 *
 * @return char * 2 byte string [A-Z each]
 */
char * LLAPMesh::getLocalID() {
    return _localID;
}

/**
 * setCoordinator
 * Flags the device as coordinator.
 * This means it will process all messages.
 *
 * @param boolean value
 */
void LLAPMesh::setCoordinator(boolean value) {
    _coordinator = value;
    if (_coordinator) {
        setLocalID("AA");
    }
}

/**
 * available
 * Returns wether there is a message awaiting or not
 * @return boolean
 */
boolean LLAPMesh::available() {
    return _available;
}

/**
 * acknowledged
 * Returns wether the last message has recive a ACK response
 * @return boolean
 */
boolean LLAPMesh::acknowledged() {
    return _acknowledged;
}

/**
 * getBody
 * Gets the body of the last processed message
 *
 * @return char * an up to 9 byte string
 */
char * LLAPMesh::getBody() {
    _available = false;
    return _body;
}

/**
 * getRemoteID
 * Gets the ID of the last processed message.
 *
 * @return char * a 2 byte string
 */
char * LLAPMesh::getRemoteID() {
    return _remoteID;
}

/**
 * processMessage
 * Filters messages and processes them
 * The message structure is:
 * byte 0       : Message header ('a')
 * bytes 1-2    : Source address for responses, destination address for requests
 *                The special address '..' indicates broadcast message
 * bytes 3-11   : Message body (with a key-value format)
 */
void LLAPMesh::processMessage() {

    // A coordinator processes all messages
    if (!_coordinator) {

        // A message to '..' will be processed by all nodes
        if (_raw[1] != '.' || _raw[2] != '.') {

            // Process messages aimed to this node
            if (_raw[1] != _localID[0] || _raw[2] != _localID[1]) return;

        }

    }

    // The main program will receive all the messages
    _remoteID[0] = _raw[1];
    _remoteID[1] = _raw[2];
    _remoteID[2] = 0;

    // Response to HELLO requests (set device ID in case the request was broadcasted)
    if (0 == strncmp_P(&_raw[3], PSTR("HELLO----"), 9)) {
        _raw[1] = _localID[0];
        _raw[2] = _localID[1];
        _Serial.print(_raw);

    // Response to STARTED requests (if coordinator)
    } else if (0 == strncmp_P(&_raw[3], PSTR("STARTED--"), 9)) {
        if (_coordinator) {
            sendMessage("ACK", "", _remoteID);
            if (_remoteID[0] == '-' && _remoteID[1] == '-') {
                char * address = getNewAddress();
                sendMessage(F("CHDEVID"), address, _remoteID);
                sendMessage(F("REBOOT"), "", _remoteID);
            }
        }

    // Change device ID (if not coordinator)
    } else if (0 == strncmp_P(&_raw[3], PSTR("CHDEVID"), 7)) {
        if (!_coordinator) {
            _Serial.print(_raw);
            char _newID[3] = {0};
            _newID[0] = _raw[10];
            _newID[1] = _raw[11];
            saveAddress(_newID);
        }

    // Reboot to apply CHDEVID (if not coordinator)
    } else if (0 == strncmp_P(&_raw[3], PSTR("REBOOT"), 6)) {
        if (!_coordinator) {
            if (_rebootable) {
                reboot();
            } else {
                loadAddress();
            }
        }

    // Acknoledgement
    } else if (0 == strncmp_P(&_raw[3], PSTR("ACK"), 3)) {
        _acknowledged = true;

    } else {
        strcpy(_body, &_raw[3]);
        _available = true;
    }

}

/**
 * SerialEvent
 * Checks for any incomming message.
 * This method has to be called in the loop.
 */
void LLAPMesh::SerialEvent() {

    // Leave if there is a previous unprocessed message
    if (_available) return;

    if (_Serial.available() >= 12) {
        char inChar = (char)_Serial.peek();
        if (inChar == 'a') {
            for (byte i = 0; i<12; i++) {
                inChar = (char)_Serial.read();
                _raw[i] = inChar;
                if (i < 11 && _Serial.peek() == 'a') {
                    // If the next char is the escape 'a' then we are out of sync
                    // so current message is not valid
                    return;
                }
            }
            _raw[12]=0;
            processMessage();
        } else {
            // This is not the char we expected, so we throw it away
            _Serial.read();
        }
    }
}

/**
 * sendMessage
 * Builds and sends a new LLAP message
 *
 * @param char * key Key of the message
 * @param char * value Alphanumeric value
 * #param char * remoteID 2 byte char array with the destination ID
 */
void LLAPMesh::sendMessage(char * key, char * value, char * remoteID) {

    _raw[0] = 'a';
    _raw[1] = remoteID[0];
    _raw[2] = remoteID[1];

    byte lenkey = strlen(key);
    byte lenvalue = strlen(value);

    for (byte i=0; i<9; i++) {
        if (i < lenkey) {
            _raw[i+3] = key[i];
        } else if (i < lenkey + lenvalue) {
            _raw[i+3] = value[i - lenkey];
        } else {
            _raw[i+3] = '-';
        }
    }

    _Serial.print(_raw);
    _Serial.flush();
    _acknowledged = false;

}

void LLAPMesh::sendMessage(const __FlashStringHelper * key, char * value, char * remoteID)
{
	const char PROGMEM *p = (const char PROGMEM *)key;
	byte eos = 0;
    _raw[0] = 'a';
    _raw[1] = remoteID[0];
    _raw[2] = remoteID[1];

    //strlen_P
    byte lenkey = strlen_P(p);
    byte lenvalue = strlen(value);

    for (byte i = 0; i<9; i++) {

        if (i < lenkey) {
            _raw[i+3] = pgm_read_byte(p++);
        } else if (i < lenkey + lenvalue) {
            _raw[i+3] = value[i - lenkey];
        } else {
            _raw[i+3] = '-';
        }

    }

    _Serial.print(_raw);
    _Serial.flush();
    _acknowledged = false;
}

void LLAPMesh::sendMessage(const __FlashStringHelper * key, char * value, boolean broadcast)
{
    if (broadcast) {
        sendMessage(key, value, "..");
    } else {
        sendMessage(key, value, _localID);
    }
}

void LLAPMesh::sendMessage(const __FlashStringHelper * key, boolean broadcast)
{
    if (broadcast) {
        sendMessage(key, "", "..");
    } else {
        sendMessage(key, "", _localID);
    }
}

/**
 * sendMessage
 * Builds and sends a new LLAP message
 *
 * @param char * key Key of the message
 * @param boolean broadcast Weather to broadcast the message or not, defaults to false,
 *    if false the message with be sent with the localID in the address field
 */
void LLAPMesh::sendMessage(char * key, boolean broadcast) {
    if (broadcast) {
        sendMessage(key, "", "..");
    } else {
        sendMessage(key, "", _localID);
    }
}

/**
 * sendMessage
 * Builds and sends a new LLAP message
 *
 * @param char * key Key of the message
 * @param char * value Alphanumeric value
 * @param boolean broadcast Weather to broadcast the message or not, defaults to false,
 *    if false the message with be sent with the localID in the address field
 */
void LLAPMesh::sendMessage(char * key, char * value, boolean broadcast) {
    if (broadcast) {
        sendMessage(key, value, "..");
    } else {
        sendMessage(key, value, _localID);
    }
}

/**
 * sendMessage
 * Builds and sends a new LLAP message
 *
 * @param char * key Key of the message
 * @param int value Integer value
 * @param boolean broadcast Weather to broadcast the message or not, defaults to false,
 *    if false the message with be sent with the localID in the address field
 */
void LLAPMesh::sendMessage(char * key, int value, boolean broadcast) {
    if (broadcast) {
        sendMessage(key, value, "..");
    } else {
        sendMessage(key, value, _localID);
    }
}

/**
 * sendMessage
 * Builds and sends a new LLAP message
 *
 * @param char * key Key of the message
 * @param int value Integer value
 * #param char * remoteID 2 byte char array with the destination ID
 */
void LLAPMesh::sendMessage(char * key, int value, char * remoteID) {
    char _value[10];
    itoa(value, _value, 10);
    sendMessage(key, _value, remoteID);
}

/**
 * sendMessage
 * Builds and sends a new LLAP message
 *
 * @param char * key Key of the message
 * @param float value Decimal value
 * @param byte precision Number of decimals of value to send
 * @param boolean broadcast Weather to broadcast the message or not, defaults to false,
 *    if false the message with be sent with the localID in the address field
 */
void LLAPMesh::sendMessage(char * key, float value, byte precision, boolean broadcast) {
    if (precision == 0) {
        sendMessage(key, (int) value, broadcast);
    } else if (broadcast) {
        sendMessage(key, value, precision, "..");
    } else {
        sendMessage(key, value, precision, _localID);
    }
}

/**
 * sendMessage
 * Builds and sends a new LLAP message
 *
 * @param char * key Key of the message
 * @param float value Decimal value
 * @param byte precision Number of decimals of value to send
 * #param char * remoteID 2 byte char array with the destination ID
 */
void LLAPMesh::sendMessage(char * key, float value, byte precision, char * remoteID) {
    if (precision == 0) {
        sendMessage(key, (int) value, remoteID);
    } else {
        float max = 100000000;
        for (byte i=0; i<precision; i++) max /= 10;
        if (abs(value) < max) {
            char _value[10] = {0};
            dtostrf(value, precision + 3, precision, _value);
            sendMessage(key, _value, remoteID);
        }
    }
}

/*
 * SerialEvent occurs whenever a new data comes in the
 * hardware serial RX.  This routine is run between each
 * time loop() runs, so using delay inside loop can delay
 * response.  Multiple bytes of data may be available.
 **/
#ifdef AUTO_SERIAL_EVENT
    void serialEvent() {
        LLAP.SerialEvent();
    }
#endif

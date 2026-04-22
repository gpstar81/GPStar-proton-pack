/**
 *   Infrared - IR communication methods for GPStar devices.
 *   Handles all sending/receiving logic for Infrared devices.
 *   Copyright (C) 2023-2026 Michael Rajotte, Dustin Grau, Nomake Wan
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

// Only compile this code for ESP32 or ATTINY1616 platforms
#if (defined(ESP32) || defined(__AVR_ATtiny1616__))

#include "InfraredManager.h"

/**
 * Static IR Data Definitions
 */

// Defines an IR command as captured from the PKE device at full power.
// This does not follow a standard protocol, so we use raw timings.
static const uint16_t ir_GhostInTrap[] = {
  1770, 1200, 600, 600, 600, 600, 580, 1200, 600, 600,
  580, 1200, 600, 1200, 580, 600, 580, 1200, 600
};

/**
 * Infrared Manager Class Interface
 *
 * NOTE: This class requires WirelessManager to be initialized first to provide the deviceID,
 * which is derived from the WiFi adapter's MAC address for unique device identification.
 */

InfraredManager::InfraredManager(uint8_t deviceType, uint16_t deviceID)
  : m_deviceType(deviceType), m_deviceID(deviceID) {
  // Initialize IR LED pin
  pinMode(IR_LED_PIN, OUTPUT);
  digitalWrite(IR_LED_PIN, LOW);
  IrSender.begin(IR_LED_PIN);

  #ifndef GPSTAR_IR_TX_ONLY
  // If not in TX-only mode, also set up the receiver pin and timer.
  IrReceiver.begin(IR_RECEIVER_PIN);
  rx_timer.start(0);
  #endif

  // Initialize and start the timer with 0 delay
  tx_timer.start(0);
}

void InfraredManager::setDeviceID(uint16_t deviceID) {
  m_deviceID = deviceID;
}

uint16_t InfraredManager::getDeviceID() const {
  return m_deviceID;
}

uint8_t InfraredManager::getDeviceType() const {
  return m_deviceType;
}

void InfraredManager::startTXTimer(uint16_t delay) {
  tx_timer.start(delay);
}

void InfraredManager::restartTXTimer() {
  tx_timer.repeat();
}

void InfraredManager::stopTXTimer() {
  tx_timer.stop();
}

bool InfraredManager::isTXTimerExpired() {
  return tx_timer.justFinished();
}

#ifndef GPSTAR_IR_TX_ONLY
void InfraredManager::startRXTimer(uint16_t delay) {
  rx_timer.start(delay);
}

void InfraredManager::restartRXTimer() {
  rx_timer.repeat();
}

void InfraredManager::stopRXTimer() {
  rx_timer.stop();
}

bool InfraredManager::isRXTimerExpired() {
  return rx_timer.justFinished();
}

bool InfraredManager::decodeData() {
  return IrReceiver.decode();
}

void InfraredManager::resumeData() {
  IrReceiver.resume();
}

bool InfraredManager::dataIsNEC() {
  return IrReceiver.decodedIRData.protocol == NEC;
}

uint8_t InfraredManager::dataPreamble() {
  uint16_t address = IrReceiver.decodedIRData.address;
  return ((address >> 14) & 0x03);
}

uint8_t InfraredManager::dataDeviceType() {
  uint16_t address = IrReceiver.decodedIRData.address;
  return ((address >> 12) & 0x03);
}

uint16_t InfraredManager::dataDeviceID() {
  uint16_t address = IrReceiver.decodedIRData.address;
  return (address & 0x0FFF);
}

uint8_t InfraredManager::dataCommand() {
  return IrReceiver.decodedIRData.command;
}
#endif

// Construct 16-bit NEC address from device type and device ID
// Format: [2-bit preamble][2-bit device type][12-bit device ID]
uint16_t InfraredManager::buildIRAddress(uint8_t deviceType, uint16_t deviceId) {
  // Ensure deviceType is only 2 bits (0-3)
  deviceType &= 0x03;

  // Ensure deviceId is only 12 bits (0-4095)
  deviceId &= 0x0FFF;

  // Construct address: preamble(2) + deviceType(2) + deviceId(12)
  return (IR_GPSTAR_PREAMBLE << 14) | (deviceType << 12) | deviceId;
}

#ifdef ESP32
// Send an infrared command using the device's configured type and ID.
// Returns a JSON document with details about the command sent and its status.
JsonDocument InfraredManager::sendCommand(IR_COMMAND_TYPE commandType, uint8_t streamMode, uint8_t powerLevel, uint8_t streamInfo) {
  JsonDocument jsonResult;

  if(commandType == IR_CMD_GHOST_TRAP) {
    // Send the standard Ghost Trap (PKE) IR signal.
    jsonResult["command"] = "ghostintrap";
    jsonResult["description"] = "Ghost Trap raw IR signal (PKE format)";
    jsonResult["status"] = "success";
    IrSender.sendRaw(ir_GhostInTrap, sizeof(ir_GhostInTrap) / sizeof(ir_GhostInTrap[0]), CARRIER_KHZ);
  }
  else if(commandType == IR_CMD_TEST) {
    // Send a special command to test the NEC-based IR signal.
    uint16_t myAddress = buildIRAddress(m_deviceType, m_deviceID);
    uint8_t command = IR_COMMAND_TEST; // Command 0xFF (255) = Testing

    jsonResult["command"] = "gpstartest";
    jsonResult["description"] = "GPStar NEC-based IR signal test command";
    jsonResult["preamble"] = "0x" + String(IR_GPSTAR_PREAMBLE, HEX);
    jsonResult["deviceType"] = "0x" + String(m_deviceType, HEX);
    jsonResult["deviceID"] = "0x" + String(m_deviceID, HEX);
    jsonResult["address"] = "0x" + String(myAddress, HEX);
    jsonResult["command"] = "0x" + String(command, HEX);

    // Show what bytes should be transmitted in standard NEC.
    uint8_t addrLSB = myAddress & 0xFF;
    uint8_t addrMSB = (myAddress >> 8) & 0xFF;
    jsonResult["expectedNEC"] = String(addrLSB, HEX) + " " + String(addrMSB, HEX) + " " + String(command, HEX) + " " + String((uint8_t)~command, HEX);

    // Compute what raw data the library will generate.
    uint32_t rawData = IrSender.computeNECRawDataAndChecksum(myAddress, command);
    jsonResult["rawData"] = "0x" + String(rawData, HEX);

    // Break down the raw data into bytes.
    uint8_t byte0 = rawData & 0xFF;
    uint8_t byte1 = (rawData >> 8) & 0xFF;
    uint8_t byte2 = (rawData >> 16) & 0xFF;
    uint8_t byte3 = (rawData >> 24) & 0xFF;
    jsonResult["rawBytes"] = String(byte0, HEX) + " "
                            + String(byte1, HEX) + " "
                            + String(byte2, HEX) + " "
                            + String(byte3, HEX);
    jsonResult["status"] = "success";

    // Send as raw NEC data (without repeats) as we constructed the data exactly as we want it.
    IrSender.sendNECRaw(rawData, 0);
  }
  else if(commandType == IR_CMD_FIRING) {
    // Send firing command with both stream type and power level embedded
    uint16_t myAddress = buildIRAddress(m_deviceType, m_deviceID);

    // Calculate command based on stream type and power level
    uint8_t baseCommand = IR_COMMAND_PROTON; // Initialize to Proton stream.
    switch(streamMode) {
      case IR_STREAM_PROTON:
      default:
        if(streamInfo == IR_CMD_STREAM) {
          baseCommand = IR_COMMAND_PROTON; // Proton: 0x20-0x24 (power levels 1-5)
        }
        else if(streamInfo == IR_CMD_BLAST) {
          baseCommand = IR_COMMAND_BOSON_DART; // Boson Dart: 0x25-0x29 (power levels 1-5)
        }
        else if(streamInfo == IR_CMD_CTS) {
          baseCommand = IR_COMMAND_CTS; // Crossing the Streams: 0x60-0x64 (power levels 1-5)
        }
      break;
      case IR_STREAM_SLIME:
        if(streamInfo == IR_CMD_STREAM) {
          baseCommand = IR_COMMAND_SLIME; // Slime: 0x30-0x34 (power levels 1-5)
        }
        else if(streamInfo == IR_CMD_BLAST) {
          baseCommand = IR_COMMAND_SLIME_BLAST; // Slime Blast: 0x35-0x39 (power levels 1-5)
        }
      break;
      case IR_STREAM_STASIS:
        if(streamInfo == IR_CMD_STREAM) {
          baseCommand = IR_COMMAND_STASIS; // Stasis: 0x40-0x44 (power levels 1-5)
        }
        else if(streamInfo == IR_CMD_BLAST) {
          baseCommand = IR_COMMAND_SHOCK_BLAST; // Shock Blast: 0x45-0x49 (power levels 1-5)
        }
      break;
      case IR_STREAM_MESON:
        if(streamInfo == IR_CMD_STREAM) {
          baseCommand = IR_COMMAND_MESON; // Meson: 0x50-0x54 (power levels 1-5)
        }
        else if(streamInfo == IR_CMD_BLAST) {
          baseCommand = IR_COMMAND_OVERLOAD_PULSE; // Overload Pulse: 0x55-0x59 (power levels 1-5)
        }
      break;
    }
    uint8_t command = baseCommand + (powerLevel - 1); // Add power level offset

    jsonResult["command"] = "firing";
    jsonResult["description"] = "GPStar NEC-based IR signal firing command with stream type and power level";
    jsonResult["streamMode"] = streamMode;
    jsonResult["powerLevel"] = powerLevel;
    jsonResult["streamInfo"] = streamInfo;
    jsonResult["preamble"] = "0x" + String(IR_GPSTAR_PREAMBLE, HEX);
    jsonResult["deviceType"] = "0x" + String(m_deviceType, HEX);
    jsonResult["deviceID"] = "0x" + String(m_deviceID, HEX);
    jsonResult["address"] = "0x" + String(myAddress, HEX);
    jsonResult["command"] = "0x" + String(command, HEX);

    // Show what bytes should be transmitted in standard NEC.
    uint8_t addrLSB = myAddress & 0xFF;
    uint8_t addrMSB = (myAddress >> 8) & 0xFF;
    jsonResult["expectedNEC"] = String(addrLSB, HEX) + " " + String(addrMSB, HEX) + " " + String(command, HEX) + " " + String((uint8_t)~command, HEX);

    // Compute what raw data the library will generate.
    uint32_t rawData = IrSender.computeNECRawDataAndChecksum(myAddress, command);
    jsonResult["rawData"] = "0x" + String(rawData, HEX);

    // Break down the raw data into bytes.
    uint8_t byte0 = rawData & 0xFF;
    uint8_t byte1 = (rawData >> 8) & 0xFF;
    uint8_t byte2 = (rawData >> 16) & 0xFF;
    uint8_t byte3 = (rawData >> 24) & 0xFF;
    jsonResult["rawBytes"] = String(byte0, HEX) + " "
                           + String(byte1, HEX) + " "
                           + String(byte2, HEX) + " "
                           + String(byte3, HEX);
    jsonResult["status"] = "success";

    // Send as raw NEC data (without repeats) as we constructed the data exactly as we want it.
    IrSender.sendNECRaw(rawData, 0);
  }
  else {
    jsonResult["command"] = "unknown";
    jsonResult["status"] = "failure";
    jsonResult["error"] = "Unknown IR Command Type";
  }

  return jsonResult;
}

// Convert string command type to IR_COMMAND_TYPE enum
bool InfraredManager::stringToCommandType(const String& commandStr, IR_COMMAND_TYPE& commandType) {
  if(commandStr.equals("ghostintrap")) {
    commandType = IR_CMD_GHOST_TRAP;
    return true;
  }
  else if(commandStr.equals("firing")) {
    commandType = IR_CMD_FIRING;
    return true;
  }
  else if(commandStr.equals("gpstartest")) {
    commandType = IR_CMD_TEST;
    return true;
  }
  return false;
}
#endif // ESP32
#endif // (defined(ESP32) || defined(__AVR_ATtiny1616__))

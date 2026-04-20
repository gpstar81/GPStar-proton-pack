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

#include <Arduino.h>
#ifdef ESP32
#include <ArduinoJson.h>
#endif
#include <millisDelay.h>

// Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data.
// Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#ifdef GPSTAR_IR_TX_ONLY // Only disable the receiver if the device requests it explicitly.
#define DISABLE_CODE_FOR_RECEIVER
#else
#ifndef IR_RECEIVER_PIN
#define IR_RECEIVER_PIN 12
#endif // ifndef IR_RECEIVER_PIN
#endif // ifdef GPSTAR_IR_TX_ONLY

// Do not use a feedback LED for the IR signal.
#define NO_LED_FEEDBACK_CODE

/**
 * IR Protocol Constants
 */
#define EXCLUDE_UNIVERSAL_PROTOCOLS // Disable universal protocols; we only use NEC.
#define EXCLUDE_EXOTIC_PROTOCOLS // Disable exotic protocols; we only use NEC.
#define CARRIER_KHZ 38 // Defines the standard IR carrier frequency in kHz.
#ifndef IR_LED_PIN // Only set a default pin number if not already set.
  #define IR_LED_PIN 17 // Standard IR LED pin for GPStar devices
#endif

/**
 * GPStar devices will use a modified address space around the NEC Extended protocol
 * which requires full construction and sending via IrSender.sendNECRaw(addr, cmd)
 *
 * The address is a 16-bit value with the command as an 8-bit value, where the type
 * of device and a unique 12-bit ID from the chip is custom-built for the address.
 *
 * This allows easy identification of the type of device sending the command and can
 * identify a sender uniquely, making the chance of overlap 1:4096 per device type.
 * - Address: 2-bit preamble (0b11) + 2-bit device code + 12-bit unique device ID.
 * - Command: 8-bit command value (providing up to 256 commands).
 */
#define IR_GPSTAR_PREAMBLE      0b11   // 0x3

// Device type codes (2 bits)
#define IR_DEVICE_NEUTRONA_WAND 0b00   // 0x0
#define IR_DEVICE_SINGLE_SHOT   0b01   // 0x1
#define IR_DEVICE_GHOST_TRAP    0b10   // 0x2
#define IR_DEVICE_PSTT          0b11   // 0x3

// Command base values for firing commands
#define IR_COMMAND_PROTON         0x20   // 0x20, Proton Stream
#define IR_COMMAND_BOSON_DART     0x25   // 0x25, Boson Dart -> Proton Stream (Alt)
#define IR_COMMAND_SLIME          0x30   // 0x30, Slime Blower
#define IR_COMMAND_SLIME_BLAST    0x35   // 0x35, Slime Blast -> Slime Blower (Alt)
#define IR_COMMAND_STASIS         0x40   // 0x40, Dark Matter Generator
#define IR_COMMAND_SHOCK_BLAST    0x45   // 0x45, Shock Blast -> Dark Matter Generator (Alt)
#define IR_COMMAND_MESON          0x50   // 0x50, Composite Particle System
#define IR_COMMAND_OVERLOAD_PULSE 0x55   // 0x55, Overload Pulse -> Composite Particle System (Alt)
#define IR_COMMAND_CTS            0x60   // 0x60, Crossing The Streams -> Proton Stream (Alt *)
#define IR_COMMAND_TEST           0xFF   // 0xFF, Test Command

// Stream mode constants for firing commands (should match STREAM_MODES enum in Streams.h)
// Note: All "Spectral" stream modes are treated as cosmetically custom Proton streams.
#define IR_STREAM_PROTON        0      // 0, Proton Stream
#define IR_STREAM_STASIS        1      // 1, Dark Matter Generator
#define IR_STREAM_SLIME         2      // 2, Slime Blower
#define IR_STREAM_MESON         3      // 3, Composite Particle System

// IR Command Type Enum - Defines the types of infrared commands that can be sent
enum IR_COMMAND_TYPE {
  IR_CMD_GHOST_TRAP,  // Ghost Trap (PKE) raw IR signal
  IR_CMD_FIRING,      // Firing command with stream type/power level
  IR_CMD_TEST         // Test command for IR signal verification
};

// IR Stream Type. Sending a stream, a crossing the streams or a blast.
enum IR_STREAM_INFO {
  IR_CMD_STREAM,   // Regular firing strea
  IR_CMD_CTS,      // Crossing the streams
  IR_CMD_BLAST     // Alt firing blasts such as: Boson Dart, Shock Blast, Overload Pulse and Slime Blast
};

/**
 * Infrared Communication Manager Class
 *
 * Handles all IR transmission for GPStar devices with device-specific configuration.
 * Each device creates an instance with its specific device type during initialization.
 */
#include <IRremote.hpp>

class InfraredManager {
public:
  /**
   * Constructor - Initialize the IR handler for a specific device type
   * @param deviceType The device type constant (IR_DEVICE_NEUTRONA_WAND, IR_DEVICE_SINGLE_SHOT, etc.)
   * @param deviceID The unique device identifier (typically from wireless manager)
   *
   * NOTE: This class requires WirelessManager to be initialized first to provide the deviceID,
   * which is derived from the WiFi adapter's MAC address for unique device identification.
   */
  InfraredManager(uint8_t deviceType, uint16_t deviceID);

#ifdef ESP32
  /**
   * Send an infrared command
   * @param commandType Command type from IR_COMMAND_TYPE enum (IR_CMD_GHOST_TRAP, IR_CMD_FIRING, IR_CMD_TEST)
   * @param streamMode Stream mode for firing commands (0=PROTON, 1=SLIME, 2=STASIS, 3=MESON)
   * @param powerLevel Power level for firing commands (1-5)
   * @param streamInfo Firing a stream, crossing the streams or a blast (alt firing) (0-1-2)
   * @return JsonDocument with command result and debugging information
   */
  JsonDocument sendCommand(IR_COMMAND_TYPE commandType, uint8_t streamMode = IR_STREAM_PROTON, uint8_t powerLevel = 5, uint8_t streamInfo = 0);

  /**
   * Convert string command type to IR_COMMAND_TYPE enum
   * @param commandStr Command type string ("ghostintrap", "firing", "gpstartest")
   * @param commandType Output parameter for the enum value
   * @return True if conversion successful, false if unknown command string
   */
  static bool stringToCommandType(const String& commandStr, IR_COMMAND_TYPE& commandType);
#endif

  /**
   * Update the device ID (useful when wireless manager reconnects)
   * @param deviceID The new device identifier
   */
  void setDeviceID(uint16_t deviceID);

  /**
   * Get the current device ID
   * @return The current device identifier
   */
  uint16_t getDeviceID() const;

  /**
   * Get the device type
   * @return The device type constant
   */
  uint8_t getDeviceType() const;

  /**
   * Start the infrared transmit timer (for automatic firing signals)
   * @param delay Timer delay in milliseconds (default: 1000ms)
   */
  void startTXTimer(uint16_t delay = 1000);

  /**
   * Restart the infrared transmit timer (for automatic firing signals)
   * Uses the last duration value from startTXTimer.
   */
  void restartTXTimer();

  /**
  * Stop the infrared transmit timer (for automatic firing signals)
  */
  void stopTXTimer();

  /**
   * Check if the infrared transmit timer has finished
   * @return True if timer has finished and can send signal
   */
  bool isTXTimerExpired();

#ifndef GPSTAR_IR_TX_ONLY
  /**
   * Start the infrared receive timer (for automatic firing signals)
   * @param delay Timer delay in milliseconds (default: 500ms)
   */
  void startRXTimer(uint16_t delay = 500);

  /**
   * Restart the infrared receive timer (for automatic firing signals)
   * Uses the last duration value from startRXTimer.
   */
  void restartRXTimer();

  /**
  * Stop the infrared receive timer (for automatic firing signals)
  */
  void stopRXTimer();

  /**
   * Check if the infrared receive timer has finished
   * @return True if timer has finished and can receive signal
   */
  bool isRXTimerExpired();

  /**
  * Decode any data we have received
  * @return True if we have valid data to decode
  */
  bool decodeData();

  /**
  * Commands IR receiver to resume receiving new data
  */
  void resumeData();

  /**
  * Check if the received data protocol is NEC
  * @return True if the data protocol is NEC
  */
  bool dataIsNEC();

  /**
  * Get the preamble of the decoded data
  * @return The IR data preamble
  */
  uint8_t dataPreamble();

  /**
  * Get the device type of the decoded data
  * @return The IR data device type
  */
  uint8_t dataDeviceType();

  /**
  * Get the device ID of the decoded data
  * @return The IR data device ID
  */
  uint16_t dataDeviceID();

  /**
  * Gets the command from the decoded data
  * @return The IR command received
  */
  uint8_t dataCommand();
#endif

private:
  uint8_t m_deviceType;     // Device type constant
  uint16_t m_deviceID;      // Unique device identifier
  millisDelay tx_timer;      // Timer for transmit signals
#ifndef GPSTAR_IR_TX_ONLY
  millisDelay rx_timer;      // Timer for receive signals
#endif
  /**
   * Build a 16-bit NEC address from device type and device ID
   * Format: [2-bit preamble][2-bit device type][12-bit device ID]
   * @param deviceType The 2-bit device type
   * @param deviceId The 12-bit device identifier
   * @return 16-bit NEC address
   */
  uint16_t buildIRAddress(uint8_t deviceType, uint16_t deviceId);
};

#endif // (defined(ESP32) || defined(__AVR_ATtiny1616__))

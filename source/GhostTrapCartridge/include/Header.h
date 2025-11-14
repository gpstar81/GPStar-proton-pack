/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <nomake_wan@yahoo.co.jp>
 *                    & Dustin Grau <dustin.grau@gmail.com>
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

// Pin definitions for door motor functionality
const uint8_t DOOR_MOTOR_CLOSE_PIN = 16; // PA3
const uint8_t DOOR_MOTOR_OPEN_PIN = 15; // PA2
const uint8_t DOOR_CLOSED_SWITCH_PIN = 0; // PA4
const uint8_t DOOR_OPENED_SWITCH_PIN = 1; // PA5
const uint8_t MOTOR_CONTROL_SLEEP_PIN = 14; // PA1 (LOW == SLEEP)

// Pin definitions for control switches
const uint8_t NOGHOST_SWITCH_PIN = 2; // PA6
const uint8_t PATTERN_SWITCH_PIN = 3; // PA7

// Pin definition for the IR receiver
const uint8_t IR_RECEIVER_PIN = 4; // PB5

// Pin definition for the yellow bargraph LED
const uint8_t YELLOW_LED_PIN = 10; // PC0

// Pin definition for triggering the 6 white interior LEDs
const uint8_t STOCK_TOP_LED_PIN = 12; // PC2

// Pin definition for addressable LEDs
const uint8_t FASTLED_PIN = 13; // PC3

// Pin definitions for smoke machine
const uint8_t SMOKE_PIN = 5; // PB4
const uint8_t BLOWER_PIN = 11; // PC1

// Pin definitions for I2C (28-segment bargraph)
const uint8_t SDA_PIN = 8; // PB1
const uint8_t SCL_PIN = 9; // PB0

// Pin definitions for serial communication
const uint8_t TX_PIN = 7; // PB2
const uint8_t RX_PIN = 6; // PB3

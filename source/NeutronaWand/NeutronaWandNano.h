/**
 *   gpstar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gmail.com>
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

/* 
 *  AltSoftSerial uses: Pin 9 = TX & Pin 8 = RX. 
 *  So Pin 9 goes to the RX of the WavTrigger and Pin 8 goes to the TX of the WavTrigger. 
 *  IMPORTANT: Do not forget to unplug the TX1/RX1 cables from Serial1 while you are uploading code to your Nano.
 *  You want to use: #define __WT_USE_ALTSOFTSERIAL__
 */
#include <AltSoftSerial.h>

const bool b_pcb = false;
const uint8_t led_slo_blo = 5; // There are 2 LED's attached to this pin. The slo-blo and the light on the front of the wand body. You can drive up to 2 led's from 1 pin on a arduino.
const uint8_t led_front_left = NULL;


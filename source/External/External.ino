/**
 *   gpstar External - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

// Suppress warning about SPI hardware pins
#define FASTLED_INTERNAL

// 3rd-Party Libraries
#include <FastLED.h>
#include <millisDelay.h>
#include <ezButton.h>

// Local Files
#include "Header.h"
#include "Colours.h"
#include "Wireless.h"

void setup(){
  Serial.begin(115200);

  // RGB LEDs for use when needed.
  FastLED.addLeds<NEOPIXEL, DEVICE_LED_PIN>(device_leds, DEVICE_NUM_LEDS);

  // Change the addressable LED to black by default.
  device_leds[PRIMARY_LED] = getHueAsRGB(PRIMARY_LED, C_BLACK);

  // Begin waiting, so we can keep moving in the main loop.
  ms_wifiretry.start(i_wifi_retry_wait);

  // Set digital pins for LED's
  pinMode(BUILT_IN_LED, OUTPUT);
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);

  // Set default state for LED's.
  digitalWrite(BUILT_IN_LED, LOW);
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);

  // Setup WiFi connection to controller device
  startWiFi();
  Serial.println("Waiting for WiFi connection...");

  // Initialize critical timers.
  ms_fast_led.start(1);
}

void loop(){
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }

  if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.println("WiFi Connection Lost");

    // If wifi has dropped, clear some flags.
    b_wifi_connected = false;
    b_socket_config= false;

    // Try to reconnect then check status.
    WiFi.reconnect();
    ms_wifiretry.start(i_wifi_retry_wait);
  }

  if (!b_wifi_connected && ms_wifiretry.remaining() < 1) {
    // Timeout expired, check if we have a connection yet.
    if (WiFi.status() == WL_CONNECTED) {
      // On first connection, output some status indicating as such.
      Serial.println("WiFi Connected");
      Serial.println(WiFi.localIP());
      b_wifi_connected = true;
    }
    else {
      // When not connected, could be any number of status possible.
      Serial.println("."); // Note that we're still waiting.
      ms_wifiretry.start(i_wifi_retry_wait);
      b_wifi_connected = false;
      b_socket_config= false;
    }
  }

  // When the wifi connection is established, proceed with websocket.
  if (b_wifi_connected) {
    if (!b_socket_config) {
      // Connect to the Attenuator device which is at a known IP address.
      webSocket.begin("192.168.1.2", 80, "/ws");

      webSocket.onEvent(webSocketEvent); // WebSocket event handler

      // If connection broken/failed then retry every X seconds.
      webSocket.setReconnectInterval(i_websocket_retry_wait);

      // Denote that we configured the websocket connection.
      b_socket_config = true;
    }

    webSocket.loop(); // Keep the socket alive.
    blinkLights(); // Update blinking lights.
  }
}

void blinkLights() {
  if(b_firing) {
    // Only begin blinking if firing.
    if(ms_blink.remaining() < 1) {
      b_blink = !b_blink; // Flip the flag.

      if(i_power > 0) {
        // Speed up the blink with the power level.
        ms_blink.start(i_blink_delay / i_power);
      }
      else {
        // Handle case where power is unset.
        ms_blink.start(i_blink_delay);
      }
    }

    if(b_blink) {
      ledsOff(); // Turn off LED's
    }
    else {
      // Turn on LED's according to the firing mode.
      switch(FIRING_MODE) {
        case PROTON:
          digitalWrite(LED_R_PIN, HIGH);
        break;
        case SLIME:
          digitalWrite(LED_G_PIN, HIGH);
        break;
        case STASIS:
          digitalWrite(LED_B_PIN, HIGH);
        break;
        case MESON:
          digitalWrite(LED_R_PIN, HIGH);
          digitalWrite(LED_G_PIN, HIGH);
        break;
        default:
          digitalWrite(LED_R_PIN, HIGH);
          digitalWrite(LED_G_PIN, HIGH);
          digitalWrite(LED_B_PIN, HIGH);
        break;
      }
    }
  }
  else {
    ledsOff(); // Turn off the RGB LED's
    b_blink = true; // Set to the blink (off) state.
  }
}

void ledsOff() {
  // Turn off the RGB pins.
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
}
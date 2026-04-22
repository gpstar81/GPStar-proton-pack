/**
 *   GPStar Proton Stream Target Trainer
 *   Copyright (C) 2023-2026 GPStar Technologies <contact@gpstartechnologies.com>
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

// Required for PlatformIO
#include <Arduino.h>

// Set to 1 to enable built-in debug messages via Serial device output.
// Use with DEBUG_SEND_TO_CONSOLE and other DEBUG_'s in Configuration.h
#define GPSTAR_DEBUG 0

// Debug macros
#if GPSTAR_DEBUG == 1
  #define debug(...) Serial.print(__VA_ARGS__)
  #define debugf(...) Serial.printf(__VA_ARGS__)
  #define debugln(...) Serial.println(__VA_ARGS__)
#else
  #define debug(...)
  #define debugf(...)
  #define debugln(...)
#endif

// PROGMEM macros
#define PROGMEM_READU32(x) pgm_read_dword_near(&(x))
#define PROGMEM_READU16(x) pgm_read_word_near(&(x))
#define PROGMEM_READU8(x) pgm_read_byte_near(&(x))

// 3rd-Party Libraries
#include <CRC32.h>
#include <digitalWriteFast.h>
#include <millisDelay.h>
#include <ESP32Servo.h>
#include <FastLED.h>
#include <ezButton.h>

// Forward declaration for use in all includes.
void sendDebug(const String& message);

// Shared Libraries
#include <WirelessManager.h>
#include <WebRouter.h>

// Define the WirelessManager pointer globally (initialized to nullptr).
// This matches the extern declaration in Wireless.h
WirelessManager* wirelessMgr = nullptr;

// Include the InfraredManager class and define a global pointer for it.
#define IR_LED_PIN 17
#define IR_RECEIVER_PIN 12
#include <InfraredManager.hpp>

// Define the InfraredManager pointer globally (initialized to nullptr).
// This matches the extern declaration in InfraredManager.h
InfraredManager* irManager = nullptr;

// Local Files
#include "Configuration.h"
#include "Header.h"
#include "Colours.h"
#include "PreferencesESP.h"
#include "GPStarServo.h"
#include "Wireless.h"
#include "Webhandler.h"
#include "Webrouting.h"
#include "System.h"

// Writes a debug message to the serial console or sends to the WebSocket or Events stream.
void sendDebug(const String& message) {
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET) and defined(ESP32)
    if(b_httpd_started) {
      ws.textAll(message); // Send a copy to the WebSocket.
    }
  #endif
  #if defined(DEBUG_SEND_TO_EVENTS) and defined(ESP32)
    sendDebugEvent(message.c_str()); // Send message to the events stream.
  #endif
}

void setup() {
  // Reduce CPU frequency to 160 MHz to save ~33% power compared to 240 MHz.
  // Alternatively set CPU to 80 MHz to save ~50% power compared to 240 MHz.
  // Do not set below 80 MHz as it will affect WiFi and other peripherals.
  setCpuFrequencyMhz(80);

  // This is required in order to make sure the board boots successfully.
  Serial.begin(115200);

  // When debugging is enabled, wait for Serial to be ready (max 3 seconds).
  unsigned long startMillis = millis();
  while (!Serial && millis() - startMillis < 3000) {
    delay(10);
  }
  Serial.flush(); // Ensure buffer is clear.
  Serial.setTxTimeoutMs(0); // Optional: reduce USB-CDC transmission delay.
  Serial.println(F("Serial is Ready")); // Should appear after Serial is ready.

  /* This loop changes GPIO40~GPIO42 to Function 1, which is GPIO.
   * PIN_FUNC_SELECT sets the IOMUX function register appropriately.
   * IO_MUX_GPIO0_REG is the register for GPIO0, which we then seek from.
   * PIN_FUNC_GPIO is a define for Function 1, which sets the pins to GPIO mode.
   */
  for(uint8_t gpio_pin = 40; gpio_pin < 43; gpio_pin++) {
    PIN_FUNC_SELECT(IO_MUX_GPIO0_REG + (gpio_pin * 4), PIN_FUNC_GPIO);
  }


  // Get all special device preferences from NVS which may be needed for sensors.
  // This also loads the target configuration settings.
  getSpecialPreferences();

  // Define the WirelessManager object only after NVS/Preferences are initialized.
  if(wirelessMgr == nullptr) {
    wirelessMgr = new WirelessManager(WirelessDeviceType::PSTT, "192.168.2.2");

    // Initialize the Infrared handler with the device type and ID.
    if(irManager == nullptr) {
      irManager = new InfraredManager(IR_DEVICE_PSTT, wirelessMgr->getDeviceID());
    }

    #if defined(RESET_AP_SETTINGS)
      // Reset the WiFi password to the expected default on every startup.
      wirelessMgr->resetWifiPassword();
      debugln(F("WARNING: Firmware forced a reset of the local WiFi password!"));
    #endif
  }

  // Attach the servo motor.
  pstt_servo.attach(PSTT_SERVO_PIN);

  // The status indicator LED on the Proton Stream Target Trainer board.
  pinModeFast(PSTT_STATUS_LED_PIN, OUTPUT);
  digitalWriteFast(PSTT_STATUS_LED_PIN, HIGH);

  FastLED.addLeds<NEOPIXEL, PSTT_JEWEL_LED_PIN>(pstt_jewel_leds, JEWEL_LED_MAX).setCorrection(TypicalLEDStrip);
  FastLED.setMaxRefreshRate(0); // Disable FastLED's blocking 2.5ms delay.

  // Update all addressable LEDs to prevent stale LED states.
  FastLED.show();

  // Initialise the fastLED state update timer.
  ms_fast_led.start(i_fast_led_delay);

  // Set target to ready state automatically on boot.
  setTargetAsReady();
}

// Loop logic dedicated to this device which handles all of the standard operations.
void mainLoop() {
  // Get the current state of any input devices (toggles, buttons, and switches).

  switch_pstt.loop();

  // Check the target health and do appropriate actions if required.
  checkTargetHealth();

  // Update the colour of the LED indicators.
  updateHealthIndicators();

  // Manual button press detection for ezButton (similar to Attenuator pattern).
  static bool was_pressed = false;
  static unsigned long press_start = 0;
  const unsigned long LONG_PRESS_TIME = 600; // 600ms for long press

  if(switch_pstt.isPressed()) {
    was_pressed = true;
    press_start = millis();
  }

  if(switch_pstt.isReleased() && was_pressed) {
    unsigned long press_duration = millis() - press_start;

    if(press_duration >= LONG_PRESS_TIME) {
      // Long press - reset target
      setTargetAsReady();
    }
    else {
      // Short press - retract target
      setTargetDefeated();
    }

    was_pressed = false;
  }

  checkInfraredData(); // Check for the latest IR data and handle as needed.
}


// The main loop of the program which manages all system operations which must occur on every loop.
void loop() {
  // The ESP32 uses a dual-core CPU with the loop() executing in Core0 by default.
  // Using vTaskDelay even without core-pinning will allow other tasks to run on Core1.
  // Features such as networking, WiFi, and OTA updates can benefit from this brief delay.
  vTaskDelay(pdMS_TO_TICKS(1)); // Translate 1 ms to ticks for delay.

  // Run checks on web-related tasks.
  webLoops();
  // (Re-)Start WiFi if the web server is not running.
  if(!b_httpd_started) {
    restartWireless();
  }

  mainLoop(); // Continue on to the main loop.

  // Update the addressable LEDs and restart the timer.
  if(ms_fast_led.justFinished()) {
    FastLED.show();

    ms_fast_led.start(i_fast_led_delay);
  }
}

/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

// Required for PlatformIO
#include <Arduino.h>

// Specify all #define statements for task scheduler first
// See: https://github.com/arkhipenko/TaskScheduler/tree/master/examples
#define _TASK_SCHEDULING_OPTIONS
#ifndef ESP32
  // This only works on ATMEGA; it will crash wifi on ESP32
  #define _TASK_SLEEP_ON_IDLE_RUN
#endif
#define _TASK_TIMECRITICAL

// See: https://github.com/arkhipenko/TaskScheduler/wiki/API-Documentation
#include <TaskScheduler.h>

// Set to 1 to enable built-in debug messages via Serial device output.
#define DEBUG 0

// Debug macros
#if DEBUG == 1
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

#ifdef ESP32
  // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data.
  // Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
  #define DISABLE_CODE_FOR_RECEIVER

  // Disable carrier PWM generation in software and use (restricted) hardware PWM.
  // This is the default for ESP32 and by defining here avoids a compiler warning.
  #define SEND_PWM_BY_TIMER

  // Do not use a feedback LED for the IR signal.
  #define NO_LED_FEEDBACK_CODE
#endif

// 3rd-Party Libraries
#include <CRC32.h>
#include <millisDelay.h>
#include <FastLED.h>
#include <avdweb_Switch.h>
#include <ht16k33.h>
#include <Wire.h>
#ifdef ESP32
  #include <HardwareSerial.h>
  #include <IRremote.hpp>
#else
  #include <EEPROM.h>
#endif

// Forward declaration for use in all includes.
void sendDebug(const String message);

// Forward declaration of scheduler task callback(s).
void animateTaskCallback();
void inputTaskCallback();
#ifdef ESP32
void motionTaskCallback();
void wifiSetupTaskCallback();
#endif

// Create the primary task scheduler.
Scheduler schedule;

// Shared Libraries
#ifdef ESP32
  #include <MagCalibration.h>
  MagCalibration magCal;

  #include <WirelessManager.h>
  // Define the WirelessManager pointer globally (initialized to nullptr).
  // This matches the extern declaration in Wireless.h
  WirelessManager* wirelessMgr = nullptr;
#endif

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Header.h"
#include "Delay.h"
#include "Colours.h"
#include "Bargraph.h"
#include "Cyclotron.h"
#include "Audio.h"
#ifdef ESP32
  #include "Motion.h"
  #include "PreferencesESP.h"
#else
  #include "PreferencesATMega.h"
#endif
#include "System.h"
#include "Actions.h"
#ifdef ESP32
  #include "Wireless.h"
  #include "Webhandler.h"
#endif

// Writes a debug message to the serial console or sends to the WebSocket.
void sendDebug(const String message) {
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET) and defined(ESP32)
    if(b_httpd_started) {
      ws.textAll(message); // Send a copy to the WebSocket.
    }
  #endif
}

// Create a task to handle all updates for LED/Bargraph animations.
// 33ms reflects a refresh rate equivalent to 30fps.
// 25ms reflects a refresh rate equivalent to 40fps.
// 20ms reflects a refresh rate equivalent to 50fps.
// 16ms reflects a refresh rate equivalent to 60fps.
Task animateTask(20, TASK_FOREVER, &animateTaskCallback);

// Create a task to check for user inputs via switches/encoders.
// Average visual reaction time to changes is 13-20ms.
Task inputsTask(14, TASK_FOREVER, &inputTaskCallback);

#ifdef ESP32
  // Create a task to check for motion via IMU/magnetometer.
  // We only need to update every 50ms (20Hz).
  Task motionTask(50, TASK_FOREVER, &motionTaskCallback);

  // Create a task for WiFi setup (single-run).
  Task wifiSetupTask(0, TASK_ONCE, &wifiSetupTaskCallback);
#endif

void setup() {
#ifdef ESP32
  // Reduce CPU frequency to 160 MHz to save ~33% power compared to 240 MHz.
  // Alternatively set CPU to 80 MHz to save ~50% power compared to 240 MHz.
  // Do not set below 80 MHz as it will affect WiFi and other peripherals.
  setCpuFrequencyMhz(160);

  // This is required in order to make sure the board boots successfully.
  Serial.begin(115200);

#if DEBUG == 1
  // When debugging is enabled, wait for Serial to be ready (max 3 seconds).
  unsigned long startMillis = millis();
  while (!Serial && millis() - startMillis < 3000) {
    delay(10);
  }
  Serial.flush(); // Ensure buffer is clear.
  Serial.setTxTimeoutMs(0); // Optional: reduce USB-CDC transmission delay.
  Serial.println(F("Serial is Ready")); // Should appear after Serial is ready.
#endif

  // Serial0 (UART0) is enabled by default; end() sets GPIO43 & GPIO44 to GPIO.
  Serial0.end();

  /* This loop changes GPIO39~GPIO42 to Function 1, which is GPIO.
   * PIN_FUNC_SELECT sets the IOMUX function register appropriately.
   * IO_MUX_GPIO0_REG is the register for GPIO0, which we then seek from.
   * PIN_FUNC_GPIO is a define for Function 1, which sets the pins to GPIO mode.
   */
  for(uint8_t gpio_pin = 39; gpio_pin < 43; gpio_pin++) {
    PIN_FUNC_SELECT(IO_MUX_GPIO0_REG + (gpio_pin * 4), PIN_FUNC_GPIO);
  }

  // Define the WirelessManager object only after NVS/Preferences are initialized.
  if(wirelessMgr == nullptr) {
    wirelessMgr = new WirelessManager("Blaster", "192.168.1.8");

    #if defined(RESET_AP_SETTINGS)
      // Reset the WiFi password to the expected default on every startup.
      wirelessMgr->resetWifiPassword();
      debugln(F("WARNING: Firmware forced a reset of the local WiFi password!"));
    #endif
  }
#else
  Serial.begin(9600); // Standard HW serial (USB) console.
#endif

  // Setup the audio device for this controller.
  setupAudioDevice();

#ifdef ESP32
  // Use of the register is not needed by ESP32, as it uses a different method for PWM.
#else
  // Change PWM frequency for the vibration motor, we do not want it high pitched.
  // For ATmega2560, we set the PWM frequency for pin 11 (TCCR5B) to 122.55 Hz.
  TCCR1B = (TCCR1B & B11111000) | B00000100;
  pinMode(VIBRATION_PIN, OUTPUT); // Vibration motor is PWM, so fallback to default pinMode just to be safe.
#endif

  // System LEDs - Consists of the chain of cyclotron and barrel LEDs
  FastLED.addLeds<NEOPIXEL, SYSTEM_LED_PIN>(system_leds, CYCLOTRON_LED_COUNT + BARREL_LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setMaxRefreshRate(0); // Disable FastLED's blocking 2.5ms delay.

  // RGB Vent Light.
  FastLED.addLeds<NEOPIXEL, TOP_LED_PIN>(vent_leds, VENT_LEDS_MAX).setCorrection(TypicalLEDStrip);
  for(uint8_t i = 0; i < VENT_LEDS_MAX; i++) {
    // Initialize all vent_leds to white initially.
    vent_leds[i] = getHueAsRGB(C_WHITE);
  }

  // Rotary encoder on the top of the device.
  encoder.initialize();

#ifdef ESP32
  // Get all special device preferences from NVS which may be needed for sensors.
  getSpecialPreferences();

  // ESP32-S3 requires manually specifying SDA and SCL pins first.
  // This is the i2c bus to be used solely for the bargraph.
  Wire.begin(I2C_SDA, I2C_SCL, 400000UL);

  // Attempt to start the sensors.
  Wire1.begin(IMU_SDA, IMU_SCL, 400000UL);
  uint8_t i_retries = 0;
  while(i_retries < 250) {
    if(!initializeSensors()) {
      debugln("Failed to find sensors, retrying");
      i_retries++;
      delay(10);
    }
    else {
      break;
    }
  }

  if(b_mag_found && b_imu_found) {
    delay(40); // Pause briefly for the devices to start.
    configureSensors(); // Set sensor ranges and defaults.
    readRawSensorData(); // Perform an initial sensor read.
    resetAllMotionData(true); // Reset and calibrate.
  }
  else {
    // Sensor malfunction detected, so disconnect Wire1.
    Wire1.end();
  }
#else
  Wire.begin();
  Wire.setClock(400000UL); // Sets the i2c bus to 400kHz
#endif

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();

  // Initialize all non-addressable LEDs
  led_Status.initialize();
  led_SloBlo.initialize();
  led_Clippard.initialize();
#ifndef ESP32
  led_TopWhite.initialize();
  led_Vent.initialize();
#endif
  led_Hat1.initialize();
  led_Hat2.initialize();
  led_Tip.initialize();

  // Device status.
  DEVICE_STATUS = MODE_OFF;
  DEVICE_ACTION_STATUS = ACTION_IDLE;

  // We bootup the device in the classic proton mode.
  STREAM_MODE = PROTON;

  // Load any saved settings stored in the EEPROM memory of the GPStar Single-Shot Blaster.
  if(b_eeprom) {
    readEEPROM();
  }

  // Reset the master volume. Important to keep this as we startup the system at the lowest volume.
  // Then the EEPROM reads any settings if required, then we reset the volume.
  updateMasterVolume(true);

  // Start up some timers for MODE_ORIGINAL.
  ms_slo_blo_blink.start(i_slo_blo_blink_delay);

  // Starts music track completion check timer.
  ms_check_music.start(i_music_check_delay);

  // Make sure lights are off, including the bargraph.
  allLightsOff();

  // Execute the System POST (Power On Self Test)
  systemPOST();

  // Set the options for the tasks so that it "catches up" if there is a delay.
  animateTask.setSchedulingOption(TASK_SCHEDULE);
  inputsTask.setSchedulingOption(TASK_SCHEDULE);

  // Initialize the task scheduler and enable the core tasks.
  schedule.init();
  schedule.addTask(animateTask);
  schedule.addTask(inputsTask);
#ifdef ESP32
  schedule.addTask(motionTask);
  schedule.addTask(wifiSetupTask);
  motionTask.enable();
  wifiSetupTask.enable();
#endif
  animateTask.enable();
  inputsTask.enable();
}

// Task callback for handling animations.
void animateTaskCallback() {
  // Update bargraph with latest state and pattern changes.
  if(ms_firing_pulse.isRunning()) {
    // Increase the speed for updates while this timer is still running.
    bargraphUpdate(POWER_LEVEL - 1);
  }
  else {
    // Otherwise run with the standard timing.
    bargraphUpdate();
  }

  // Keep the cyclotron spinning as necessary.
  checkCyclotron();

  // Update all addressable LEDs to reflect any changes.
  FastLED[0].showLeds(255);

  // Update the vent/top LEDs.
  if(b_vent_lights_changed) {
    if(b_rgb_vent_light) {
      // Only commit an update if the addressable LED panel is installed.
      FastLED[1].showLeds(255);
    }

    b_vent_lights_changed = false;
  }
}

// Task callback for handling user inputs.
void inputTaskCallback() {
#ifdef ESP32
  webLoops(); // Handle web server loops, including WebSocket events and OTA updates.
#endif

  updateAudio(); // Update the state of the available sound board.

  checkMusic(); // Perform music control here as this is a standalone device.

  switchLoops(); // Standard polling for switch/button changes via user inputs.

  // Get the current state of any input devices (toggles, buttons, and switches).
  checkRotaryEncoder();
  checkMenuVibration();

  // Handle button press events based on current device state and menu level (for config/EEPROM purposes).
  checkDeviceAction();

  // Perform updates/actions based on timer events.
  checkGeneralTimers();
}

#ifdef ESP32
// Task callback for handling motion detection.
void motionTaskCallback() {
  if(b_mag_found && b_imu_found) {
    checkMotionSensors();
  }
}

// Task callback for WiFi setup (single-run).
void wifiSetupTaskCallback() {
  debugln(F("Starting WiFi setup task..."));
  
  // Begin by setting up WiFi as a prerequisite to all else.
  if(startWiFi()) {
    // Start the local web server.
    startWebServer();
    debugln(F("WiFi and web server started successfully"));
  }
  else {
    debugln(F("Failed to start WiFi"));
  }
  
  // Disable this task after it runs once.
  wifiSetupTask.disable();
}
#endif

void loop() {
  // Task execution via the scheduler.
  schedule.execute();
}

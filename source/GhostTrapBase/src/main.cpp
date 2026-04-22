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
#include <millisDelay.h>
#include <ezButton.h>
#include <SerialTransfer.h>
#include <esp_system.h>
#include <nvs_flash.h>

// Serial comms definitions (move to Serial.h)
#define TX_PIN 43 // Pin to transmit serial data to trap cartridge
#define RX_PIN 44 // Pin to receive serial data from trap cartridge
HardwareSerial CartridgeSerial(0); // Associate AttenuatorSerial with UART0
SerialTransfer cartridgeComs;

// Writes a debug message to the serial console or sends to the WebSocket or Events stream.
void sendDebug(const String& message) {
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET)
    ws.textAll(message); // Send a copy to the WebSocket.
  #endif
  #if defined(DEBUG_SEND_TO_EVENTS) and defined(ESP32)
    sendDebugEvent(message.c_str()); // Send message to the events stream.
  #endif
}

// Shared Libraries
#include <DeviceState.h>
#include <Communication.h>
#include <WirelessManager.h>
#include <WebRouter.h>

// Global instance of DeviceState class for the Ghost Trap.
DeviceState gpstarTrap;

// Local Files
#include "Configuration.h"
#include "Header.h"
#include "MusicSounds.h"
#include "Audio.h"
#include "Wireless.h"
#include "Webhandler.h"
#include "Webrouting.h"
#include "System.h"

// Define the WirelessManager pointer globally (initialized to nullptr).
// This matches the extern declaration in Wireless.h
WirelessManager* wirelessMgr = nullptr;

// Task Handles
TaskHandle_t AnimationTaskHandle = NULL;
TaskHandle_t PreferencesTaskHandle = NULL;
TaskHandle_t UserInputTaskHandle = NULL;
TaskHandle_t WiFiManagementTaskHandle = NULL;
TaskHandle_t WiFiSetupTaskHandle = NULL;

// Variables for approximating CPU load
// https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/volatile/
volatile uint32_t idleTimeCore0 = 0;
volatile uint32_t idleTimeCore1 = 0;

// Idle task for Core 0
#if defined(DEBUG_PERFORMANCE)
void idleTaskCore0(void * parameter) {
  while(true) {
    idleTimeCore0 = idleTimeCore0 + 1;
    vTaskDelay(1);
  }
}
#endif

// Idle task for Core 1
#if defined(DEBUG_PERFORMANCE)
void idleTaskCore1(void * parameter) {
  while(true) {
    idleTimeCore1 = idleTimeCore1 + 1;
    vTaskDelay(1);
  }
}
#endif

// Animation Task (Loop)
void AnimationTask(void *parameter) {
  while(true) {
    #if defined(DEBUG_TASK_TO_CONSOLE)
      // Confirm the core in use for this task, and when it runs.
      debug(F("Executing AnimationTask in core"));
      debug(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      debug(F(" | Stack HWM: "));
      debugln(uxTaskGetStackHighWaterMark(NULL));
    #endif

    // Update LEDs using appropriate colour scheme and environment vars.
    updateLEDs();

    vTaskDelay(8 / portTICK_PERIOD_MS); // 8ms delay
  }
}

// Preferences Task (Single-Run)
void PreferencesTask(void *parameter) {
  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Confirm the core in use for this task, and when it runs.
    debug(F("Executing PreferencesTask in core"));
    debugln(xPortGetCoreID());
  #endif

  // Print partition information to verify NVS availability
  #if defined(DEBUG_SEND_TO_CONSOLE)
  printPartitions();
  #endif

  // Initialize the NVS flash partition and throw any errors as necessary.
  esp_err_t err = nvs_flash_init();
  if(err != ESP_OK) {
    #if defined(DEBUG_SEND_TO_CONSOLE)
    debug(F("NVS initialization failed with error: "));
    debugln(esp_err_to_name(err));
    #endif

    // If initialization fails, erase and reinitialize NVS.
    debugln(F("Erasing and reinitializing NVS..."));
    nvs_flash_erase();

    err = nvs_flash_init();
    if(err != ESP_OK) {
      #if defined(DEBUG_SEND_TO_CONSOLE)
      debug(F("Failed to reinitialize NVS: "));
      debugln(esp_err_to_name(err));
      #endif
    }
    else {
      debugln(F("NVS reinitialized successfully"));
    }
  }
  else {
    debugln(F("NVS initialized successfully"));
  }

  /*
   * Get Local Device Preferences
   * Accesses the "device" namespace in read-only mode under the "nvs" partition.
   */
  Preferences preferences;
  if(preferences.begin("device", true)) {
    switch(preferences.getUChar("display_type", STATUS_GRAPHIC)) {
      case 0:
        DISPLAY_TYPE = STATUS_TEXT;
      break;
      case 1:
        DISPLAY_TYPE = STATUS_GRAPHIC;
      break;
      case 2:
        DISPLAY_TYPE = STATUS_BOTH;
      break;
    }

    // Preferences for smoke (enabled, duration) on doors opened/closed.
    b_smoke_opened_enabled = preferences.getBool("smoke_opened", false);
    b_smoke_closed_enabled = preferences.getBool("smoke_closed", false);
    i_smoke_opened_duration = preferences.getUShort("smoke_op_dur", 2000);
    i_smoke_closed_duration = preferences.getUShort("smoke_cl_dur", 3000);

    preferences.end();
  }
  else {
    // If namespace is not initialized, open in read/write mode and set defaults.
    if(preferences.begin("device", false)) {
      preferences.putUChar("display_type", DISPLAY_TYPE);
      preferences.putBool("smoke_opened", b_smoke_opened_enabled);
      preferences.putBool("smoke_closed", b_smoke_closed_enabled);
      preferences.putUShort("smoke_op_dur", i_smoke_opened_duration);
      preferences.putUShort("smoke_cl_dur", i_smoke_closed_duration);
      preferences.end();
    }
  }

  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Get the stack high water mark for optimizing bytes allocated.
    debug(F("PreferencesTask Stack HWM: "));
    debugln(uxTaskGetStackHighWaterMark(NULL));
  #endif

  // Task ends after setup is complete and MUST be removed from scheduling.
  // Failure to do this can cause an error within the watchdog timer!
  vTaskDelete(NULL);
}

// User Input Task (Loop)
void UserInputTask(void *parameter) {
  while(true) {
    #if defined(DEBUG_TASK_TO_CONSOLE)
      // Confirm the core in use for this task, and when it runs.
      Serial.print(F("Executing UserInputTask in core"));
      Serial.print(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      Serial.print(F(" | Stack HWM: "));
      Serial.println(uxTaskGetStackHighWaterMark(NULL));
    #endif

    checkPedal(); // Check the state of the foot pedal or trap triggers.
    checkDoors(); // Check for door state (open/close).

    // Trigger an update to the user that the doors have changed state.
    if(LAST_DOOR_STATE != DOORS_UNKNOWN && LAST_DOOR_STATE != DOOR_STATE) {
      notifyWSClients(); // Alert connected clients that the doors changed.

      if(DOOR_STATE == DOORS_OPENED && b_smoke_opened_enabled) {
        startSmoke(i_smoke_opened_duration); // Run smoke for X seconds after doors open.
        ms_light.start(i_smoke_opened_duration * 2); // Override light timer to double duration.
      }
      if(DOOR_STATE == DOORS_CLOSED && b_smoke_closed_enabled) {
        startSmoke(i_smoke_closed_duration); // Run smoke for X seconds after doors close.
      }
    }
    LAST_DOOR_STATE = DOOR_STATE; // Remember the latest door state.

    vTaskDelay(14 / portTICK_PERIOD_MS); // 14ms delay
  }
}

// WiFi Management Task (Loop)
void WiFiManagementTask(void *parameter) {
  while(true) {
    #if defined(DEBUG_TASK_TO_CONSOLE)
      // Confirm the core in use for this task, and when it runs.
      debug(F("Executing WiFiManagementTask in core"));
      debug(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      debug(F(" | Stack HWM: "));
      debugln(uxTaskGetStackHighWaterMark(NULL));
    #endif

    // Perform periodic checks for WiFi clients and OTA updates.
    webLoops();

    vTaskDelay(100 / portTICK_PERIOD_MS); // 100ms delay
  }
}

// WiFi Setup Task (Single-Run)
void WiFiSetupTask(void *parameter) {
  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Confirm the core in use for this task, and when it runs.
    debug(F("Executing WiFiSetupTask in core"));
    debugln(xPortGetCoreID());
  #endif

  // Define the WirelessManager object only after NVS/Preferences are initialized.
  if(wirelessMgr == nullptr) {
    wirelessMgr = new WirelessManager(WirelessDeviceType::GHOST_TRAP, "192.168.1.10");

    #if defined(RESET_AP_SETTINGS)
      // Reset the WiFi password to the expected default on every startup.
      wirelessMgr->resetWifiPassword();
      debugln(F("WARNING: Firmware forced a reset of the local WiFi password!"));
    #endif
  }

  // Begin by setting up WiFi as a prerequisite to all else.
  if(startWiFi()) {
    // Start the local web server.
    startWebServer();

    // Begin timer for remote client events.
    ms_cleanup.start(i_websocketCleanup);
    ms_apclient.start(i_apClientCount);
    ms_otacheck.start(i_otaCheck);
  }

  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Get the stack high water mark for optimizing bytes allocated.
    debug(F("WiFiSetupTask Stack HWM: "));
    debugln(uxTaskGetStackHighWaterMark(NULL));
  #endif

  // Task ends after setup is complete and MUST be removed from scheduling.
  // Failure to do this can cause an error within the watchdog timer!
  vTaskDelete(NULL);
}

void setup() {
  // Provide an opportunity to set the CPU Frequency MHz: 80, 160, 240 [Default = 240]
  // Lower frequency means less power consumption, but slower performance (obviously).
  setCpuFrequencyMhz(160);

  Serial.begin(115200); // Serial monitor via USB connection.
  delay(1000); // Provide a delay to allow serial output.

  #if defined(DEBUG_SEND_TO_CONSOLE)
    debug(F("CPU Freq (MHz): "));
    debugln(getCpuFrequencyMhz());
  #endif

  CartridgeSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  cartridgeComs.begin(CartridgeSerial, false);

  // Setup the audio device for this controller.
  setupAudioDevice();

  // Get initial switch/button states.
  switchLoops();

  // Delay before configuring and running tasks.
  delay(200);

  // Set up for reading the switches to determine door state.
  DOOR_STATE = DOORS_UNKNOWN; // Default until we first read the pins.
  LAST_DOOR_STATE = DOOR_STATE; // Keep setting in sync until read.

  // Set the trap state.
  TRAP_STATE = TRAP_IDLE;

  /**
   * By default the WiFi will run on core0, while the standard loop() runs on core1.
   * We can make efficient use of the available cores by "pinning" a task to a core.
   * The ESP32 platform comes with FreeRTOS implemented internally and exposed even
   * to the Arduino platform (meaning: no need for using the ESP-IDF exclusively).
   * In theory this allows for improved parallel processing with prioritization and
   * granting of dedicated memory stacks to each task (which can be monitored).
   *
   * Parameters:
   *  Task Function Name,
   *  User-Friendly Task Name,
   *  Stack Size (in bytes),
   *  Input Parameter,
   *  Priority (use higher #),
   *  Task Handle Reference,
   *  Pinned Core (0 or 1)
   */

  // Create a single-run setup task with the highest priority for WiFi/WebServer startup.
  xTaskCreatePinnedToCore(PreferencesTask, "PreferencesTask", 4096, NULL, 5, &PreferencesTaskHandle, 1);

  // Delay all lower priority tasks until Preferences are loaded.
  vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for 100ms to avoid competition.

  // Create a single-run setup task with the highest priority for WiFi/WebServer startup.
  xTaskCreatePinnedToCore(WiFiSetupTask, "WiFiSetupTask", 4096, NULL, 4, &WiFiSetupTaskHandle, 1);

  // Delay all lower priority tasks until WiFi and WebServer setup is done.
  vTaskDelay(200 / portTICK_PERIOD_MS); // Delay for 200ms to avoid competition.

  // Create tasks which utilize a loop for continuous operation (prioritized highest to lowest).
  xTaskCreatePinnedToCore(UserInputTask, "UserInputTask", 4096, NULL, 3, &UserInputTaskHandle, 1);
  xTaskCreatePinnedToCore(AnimationTask, "AnimationTask", 2048, NULL, 2, &AnimationTaskHandle, 1);
  xTaskCreatePinnedToCore(WiFiManagementTask, "WiFiManagementTask", 2048, NULL, 1, &WiFiManagementTaskHandle, 1);

  // Create idle tasks for each core, used to estimate % busy for core.
  #if defined(DEBUG_PERFORMANCE)
  xTaskCreatePinnedToCore(idleTaskCore0, "Idle Task Core 0", 1000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(idleTaskCore1, "Idle Task Core 1", 1000, NULL, 1, NULL, 1);
  #endif
}

// Helper function to format bytes with a comma separator
String formatBytesWithCommas(uint32_t bytes) {
    String result = String(bytes);
    int insertPosition = result.length() - 3;
    while(insertPosition > 0) {
        result = result.substring(0, insertPosition) + "," + result.substring(insertPosition);
        insertPosition -= 3;
    }
    return result;
}

// Function to calculate and print CPU load
void printCPULoad() {
  uint32_t idle0 = idleTimeCore0;
  uint32_t idle1 = idleTimeCore1;

  // Calculate CPU load as (total time - idle time) / total time
  float cpuLoadCore0 = 100.0 - ((float)idle0 / (float)(idle0 + idle1)) * 100.0;
  float cpuLoadCore1 = 100.0 - ((float)idle1 / (float)(idle0 + idle1)) * 100.0;

  debug(F("CPU Load Core0: "));
  debug(cpuLoadCore0);
  debugln(F("%"));

  debug(F("CPU Load Core1: "));
  debug(cpuLoadCore1);
  debugln(F("%"));

  // Reset idle times after calculation
  idleTimeCore0 = 0;
  idleTimeCore1 = 0;
}

void printMemoryStats() {
  debugln(F("Memory Usage Stats:"));

  // Heap memory
  debug(F("|-Total Free Heap: "));
  debug(formatBytesWithCommas(esp_get_free_heap_size()));
  debugln(F(" bytes"));

  debug(F("|-Minimum Free Heap Ever: "));
  debug(formatBytesWithCommas(esp_get_minimum_free_heap_size()));
  debugln(F(" bytes"));

  debug(F("|-Maximum Allocatable Block: "));
  debug(formatBytesWithCommas(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)));
  debugln(F(" bytes"));

  // Stack memory (for the main task)
  debug(F("|-Tasks Stack High Water Mark:"));
  debug(F("|--Main Task: "));
  debug(formatBytesWithCommas(uxTaskGetStackHighWaterMark(NULL)));
  debugln(F(" bytes"));

  // Stack memory (for other tasks)
  if(AnimationTaskHandle != NULL) {
    debug(F("|--Animation: "));
    debug(formatBytesWithCommas(uxTaskGetStackHighWaterMark(AnimationTaskHandle)));
    debugln(F(" / 2,048 bytes"));
  }
  if(UserInputTaskHandle != NULL) {
    debug(F("|--User Input: "));
    debug(formatBytesWithCommas(uxTaskGetStackHighWaterMark(UserInputTaskHandle)));
    debugln(F(" / 4,096 bytes"));
  }
  if(WiFiManagementTaskHandle != NULL) {
    debug(F("|--WiFi Mgmt.: "));
    debug(formatBytesWithCommas(uxTaskGetStackHighWaterMark(WiFiManagementTaskHandle)));
    debugln(F(" / 2,048 bytes"));
  }
}

void loop() {
  // No work done here, only in the tasks!

  #if defined(DEBUG_PERFORMANCE)
  debugln(F("=================================================="));
  printCPULoad();      // Print CPU load
  printMemoryStats();  // Print memory usage
  delay(3000);         // Wait 5 seconds before printing again
  #endif
}

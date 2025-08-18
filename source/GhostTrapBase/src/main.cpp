/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <-redacted->
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

// Suppress warning about SPI hardware pins
// Define this before including <FastLED.h>
#define FASTLED_INTERNAL

// PROGMEM macro
#define PROGMEM_READU32(x) pgm_read_dword_near(&(x))
#define PROGMEM_READU16(x) pgm_read_word_near(&(x))
#define PROGMEM_READU8(x) pgm_read_byte_near(&(x))

// 3rd-Party Libraries
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <esp_system.h>
#include <nvs_flash.h>

// Local Files
#include "Configuration.h"
#include "Header.h"
#include "Colours.h"
#include "MusicSounds.h"
#include "Audio.h"
#include "Wireless.h"
#include "System.h"

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
    idleTimeCore0++;
    vTaskDelay(1);
  }
}
#endif

// Idle task for Core 1
#if defined(DEBUG_PERFORMANCE)
void idleTaskCore1(void * parameter) {
  while(true) {
    idleTimeCore1++;
    vTaskDelay(1);
  }
}
#endif

// Animation Task (Loop)
void AnimationTask(void *parameter) {
  while(true) {
    #if defined(DEBUG_TASK_TO_CONSOLE)
      // Confirm the core in use for this task, and when it runs.
      Serial.print(F("Executing AnimationTask in core"));
      Serial.print(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      Serial.print(F(" | Stack HWM: "));
      Serial.println(uxTaskGetStackHighWaterMark(NULL));
    #endif

    // Update LEDs using appropriate colour scheme and environment vars.
    updateLEDs();

    // Update the state of any LEDs.
    FastLED.show();

    // Verify the state of any other devices which need updating.
    checkBlower();
    checkSmoke();

    vTaskDelay(8 / portTICK_PERIOD_MS); // 8ms delay
  }
}

// Preferences Task (Single-Run)
void PreferencesTask(void *parameter) {
  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Confirm the core in use for this task, and when it runs.
    Serial.print(F("Executing PreferencesTask in core"));
    Serial.println(xPortGetCoreID());
  #endif

  // Print partition information to verify NVS availability
  #if defined(DEBUG_SEND_TO_CONSOLE)
  printPartitions();
  #endif

  // Initialize the NVS flash partition and throw any errors as necessary.
  esp_err_t err = nvs_flash_init();
  if(err != ESP_OK) {
    #if defined(DEBUG_SEND_TO_CONSOLE)
    Serial.printf("NVS initialization failed with error: %s\n", esp_err_to_name(err));
    #endif

    // If initialization fails, erase and reinitialize NVS.
    debug(F("Erasing and reinitializing NVS..."));
    nvs_flash_erase();

    err = nvs_flash_init();
    if(err != ESP_OK) {
      #if defined(DEBUG_SEND_TO_CONSOLE)
      Serial.printf("Failed to reinitialize NVS: %s\n", esp_err_to_name(err));
      #endif
    }
    else {
      debug(F("NVS reinitialized successfully"));
    }
  }
  else {
    debug(F("NVS initialized successfully"));
  }

  /*
   * Get Local Device Preferences
   * Accesses the "device" namespace in read-only mode under the "nvs" partition.
   */
  bool b_namespace_opened = preferences.begin("device", true);
  if(b_namespace_opened) {
    switch(preferences.getShort("display_type", 0)) {
      case 0:
        DISPLAY_TYPE = STATUS_TEXT;
      break;
      case 1:
        DISPLAY_TYPE = STATUS_GRAPHIC;
      break;
      case 2:
      default:
        DISPLAY_TYPE = STATUS_BOTH;
      break;
    }

    // Preferences for smoke (enabled, duration) on doors opened/closed.
    b_smoke_opened_enabled = preferences.getBool("smoke_opened", false);
    b_smoke_closed_enabled = preferences.getBool("smoke_closed", false);
    i_smoke_opened_duration = preferences.getShort("smoke_op_dur", 2000);
    i_smoke_closed_duration = preferences.getShort("smoke_cl_dur", 3000);

    preferences.end();
  }
  else {
    // If namespace is not initialized, open in read/write mode and set defaults.
    if(preferences.begin("device", false)) {
      preferences.putShort("display_type", DISPLAY_TYPE);
      preferences.putBool("smoke_opened", b_smoke_opened_enabled);
      preferences.putBool("smoke_closed", b_smoke_closed_enabled);
      preferences.putShort("smoke_op_dur", i_smoke_opened_duration);
      preferences.putShort("smoke_cl_dur", i_smoke_closed_duration);
      preferences.end();
    }
  }

  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Get the stack high water mark for optimizing bytes allocated.
    Serial.print(F("PreferencesTask Stack HWM: "));
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
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

    checkDoors(); // Check for door state (open/close).

    // Trigger an update to the user that the doors have changed state.
    if (LAST_DOOR_STATE != DOORS_UNKNOWN && LAST_DOOR_STATE != DOOR_STATE) {
      notifyWSClients(); // Alert connected clients that the doors changed.

      if (DOOR_STATE == DOORS_OPENED && b_smoke_opened_enabled) {
        startSmoke(i_smoke_opened_duration); // Run smoke for X seconds after doors open.
        ms_light.start(i_smoke_opened_duration * 2); // Override light timer to double duration.
      }
      if (DOOR_STATE == DOORS_CLOSED && b_smoke_closed_enabled) {
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
      Serial.print(F("Executing WiFiManagementTask in core"));
      Serial.print(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      Serial.print(F(" | Stack HWM: "));
      Serial.println(uxTaskGetStackHighWaterMark(NULL));
    #endif

    // Proceed with management if the AP and web server are started.
    if(b_ap_started && b_ws_started) {
      if(ms_cleanup.remaining() < 1) {
        // Clean up oldest WebSocket connections.
        ws.cleanupClients();

        // Restart timer for next cleanup action.
        ms_cleanup.start(i_websocketCleanup);
      }

      if(ms_apclient.remaining() < 1) {
        // Update the current count of AP clients.
        i_ap_client_count = WiFi.softAPgetStationNum();

        // Restart timer for next count.
        ms_apclient.start(i_apClientCount);
      }

      if(ms_otacheck.remaining() < 1) {
        // Handles device reboot after an OTA update.
        ElegantOTA.loop();

        // Restart timer for next check.
        ms_otacheck.start(i_otaCheck);
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS); // 100ms delay
  }
}

// WiFi Setup Task (Single-Run)
void WiFiSetupTask(void *parameter) {
  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Confirm the core in use for this task, and when it runs.
    Serial.print(F("Executing WiFiSetupTask in core"));
    Serial.println(xPortGetCoreID());
  #endif

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
    Serial.print(F("WiFiSetupTask Stack HWM: "));
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
  #endif

  // Task ends after setup is complete and MUST be removed from scheduling.
  // Failure to do this can cause an error within the watchdog timer!
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200); // Serial monitor via USB connection.
  delay(1000); // Provide a delay to allow serial output.

  // Provide an opportunity to set the CPU Frequency MHz: 80, 160, 240 [Default = 240]
  // Lower frequency means less power consumption, but slower performance (obviously).
  setCpuFrequencyMhz(160);
  #if defined(DEBUG_SEND_TO_CONSOLE)
    Serial.print(F("CPU Freq (MHz): "));
    Serial.println(getCpuFrequencyMhz());
  #endif

  #if defined(USE_ESP32_S3)
    // RGB LED on the ESP32-S3 device itself.
    FastLED.addLeds<WS2812, BUILT_IN_LED>(device_leds, DEVICE_NUM_LEDS);
  #endif

  // RGB LEDs on the top of the trap (Frutto Technology).
  FastLED.addLeds<WS2812, TOP_PIXELS>(top_leds, NUM_TOP_PIXELS);

  // Get initial switch/button states.
  switchLoops();

  // Delay before configuring and running tasks.
  delay(200);

  // Configure the blower fan.
  pinMode(BLOWER_PIN, OUTPUT);
  digitalWrite(BLOWER_PIN, LOW); // Set to LOW (off)

  // Configure the smoke (coil + pump) device.
  pinMode(SMOKE_PIN, OUTPUT);
  digitalWrite(SMOKE_PIN, LOW); // Set to LOW (off)

  // Configure the the top 2 white lights.
  pinMode(TOP_2WHITE, OUTPUT);
  digitalWrite(TOP_2WHITE, LOW); // Set to LOW (off)

  // Set up for reading the switches to determine door state.
  pinMode(DOOR_CLOSED_PIN, INPUT);
  pinMode(DOOR_OPENED_PIN, INPUT);
  DOOR_STATE = DOORS_UNKNOWN; // Default until we first read the pins.
  LAST_DOOR_STATE = DOOR_STATE; // Keep setting in sync until read.

  // Prepare the on-board (non-power) LED to be used as an output pin for indication.
  pinMode(BUILT_IN_LED, OUTPUT);
  digitalWrite(BUILT_IN_LED, LOW);

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

  Serial.print(F("CPU Load Core0: "));
  Serial.print(cpuLoadCore0);
  Serial.println(F("%"));

  Serial.print(F("CPU Load Core1: "));
  Serial.print(cpuLoadCore1);
  Serial.println(F("%"));

  // Reset idle times after calculation
  idleTimeCore0 = 0;
  idleTimeCore1 = 0;
}

void printMemoryStats() {
  Serial.println(F("Memory Usage Stats:"));

  // Heap memory
  Serial.print(F("|-Total Free Heap: "));
  Serial.print(formatBytesWithCommas(esp_get_free_heap_size()));
  Serial.println(F(" bytes"));

  Serial.print(F("|-Minimum Free Heap Ever: "));
  Serial.print(formatBytesWithCommas(esp_get_minimum_free_heap_size()));
  Serial.println(F(" bytes"));

  Serial.print(F("|-Maximum Allocatable Block: "));
  Serial.print(formatBytesWithCommas(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)));
  Serial.println(F(" bytes"));

  // Stack memory (for the main task)
  Serial.println(F("|-Tasks Stack High Water Mark:"));
  Serial.print(F("|--Main Task: "));
  Serial.print(formatBytesWithCommas(uxTaskGetStackHighWaterMark(NULL)));
  Serial.println(F(" bytes"));

  // Stack memory (for other tasks)
  if (AnimationTaskHandle != NULL) {
    Serial.print(F("|--Animation: "));
    Serial.print(formatBytesWithCommas(uxTaskGetStackHighWaterMark(AnimationTaskHandle)));
    Serial.println(F(" / 2,048 bytes"));
  }
  if (UserInputTaskHandle != NULL) {
    Serial.print(F("|--User Input: "));
    Serial.print(formatBytesWithCommas(uxTaskGetStackHighWaterMark(UserInputTaskHandle)));
    Serial.println(F(" / 4,096 bytes"));
  }
  if (WiFiManagementTaskHandle != NULL) {
    Serial.print(F("|--WiFi Mgmt.: "));
    Serial.print(formatBytesWithCommas(uxTaskGetStackHighWaterMark(WiFiManagementTaskHandle)));
    Serial.println(F(" / 2,048 bytes"));
  }
}

void loop() {
  // No work done here, only in the tasks!

  #if defined(DEBUG_PERFORMANCE)
  Serial.println(F("=================================================="));
  printCPULoad();      // Print CPU load
  printMemoryStats();  // Print memory usage
  delay(3000);         // Wait 5 seconds before printing again
  #endif
}

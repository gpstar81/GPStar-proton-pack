/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2025 Dustin Grau <dustin.grau@gmail.com>
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
#include <esp_system.h>
#include <nvs_flash.h>

// Local Files
#include "Configuration.h"
#include "Header.h"
#include "Colours.h"
#include "Wireless.h"
#include "System.h"

// Task Handles
TaskHandle_t AnimationTaskHandle = NULL;
TaskHandle_t PreferencesTaskHandle = NULL;
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
      Serial.print(F("Executing AnimationTask in core"));
      Serial.print(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      Serial.print(F(" | Stack HWM: "));
      Serial.println(uxTaskGetStackHighWaterMark(NULL));
    #endif

    // Update light animation based on websocket data.
    if(b_firing) {
      animateLights();
    }
    else {
      if(b_ext_wifi_started) {
        ledsOff();
      }
      else {
        device_leds[0] = getHueAsRGB(PRIMARY_LED, C_PURPLE, 255);
      }
    }

    // Update the device LEDs and restart the timer.
    FastLED.show();

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

  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Get the stack high water mark for optimizing bytes allocated.
    Serial.print(F("PreferencesTask Stack HWM: "));
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
  #endif

  // Task ends after setup is complete and MUST be removed from scheduling.
  // Failure to do this can cause an error within the watchdog timer!
  vTaskDelete(NULL);
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

    // Handle reconnection to external WiFi when necessary.
    if(b_ap_started) {
      if(b_ws_started && ms_cleanup.remaining() < 1) {
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

      if (WiFi.status() == WL_CONNECTED && b_ext_wifi_started && !b_socket_ready) {
        debug(F("WiFi Connected, Socket Not Configured"));
        b_ext_wifi_paused = false; // Resume retries when needed.
        setupWebSocketClient(); // Restore the WebSocket connection.
      }
    }

    // Proceed with management if the AP and web server are started.
    if(b_ap_started) {
      if(ms_otacheck.remaining() < 1) {
        // Handles device reboot after an OTA update.
        ElegantOTA.loop();

        // Restart timer for next check.
        ms_otacheck.start(i_otaCheck);
      }

      // Try to start the external WiFi.
      if(!b_ext_wifi_started && !b_ext_wifi_paused) {
        b_ext_wifi_started = startExternalWifi();
      }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1000ms delay
  }
}

// WiFi Setup Task (Single-Run)
void WiFiSetupTask(void *parameter) {
  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Confirm the core in use for this task, and when it runs.
    Serial.print(F("Executing WiFiSetupTask in core"));
    Serial.println(xPortGetCoreID());
  #endif

  // Set a visual indicator that WiFi is being configured.
  device_leds[0] = getHueAsRGB(PRIMARY_LED, C_RED, 255);
  FastLED.show();

  // Begin by setting up WiFi as a prerequisite to all else.
  if(startWiFi()) {
    if(b_ap_started) {
      // Indicate we've established the private network.
      device_leds[0] = getHueAsRGB(PRIMARY_LED, C_BLUE, 255);
      FastLED.show();
    }

    // Start the local web server.
    startWebServer();

    // Begin timer for remote client events.
    ms_cleanup.start(i_websocketCleanup);
    ms_apclient.start(i_apClientCount);
    ms_otacheck.start(i_otaCheck);
  }

  vTaskDelay(200 / portTICK_PERIOD_MS); // 200ms delay

  // Clear LED once we have the AP and web server started.
  device_leds[0] = CRGB::Black;
  FastLED.show();

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

  // Boot into proton mode at level 1 by default.
  STREAM_MODE = PROTON;
  POWER_LEVEL = LEVEL_1;

  // Device RGB LEDs for use when needed.
  FastLED.addLeds<NEOPIXEL, DEVICE_LED_PIN>(device_leds, DEVICE_NUM_LEDS);
  ms_anim_change.start(i_animation_time); // Default animation time.

  // Set palette by stream mode.
  updateStreamPalette();

  // Change the addressable LED to black by default.
  fill_solid(device_leds, DEVICE_NUM_LEDS, CRGB::Black);

  // Initialize palettes with custom color gradients
  paletteProton = CRGBPalette16(
    CRGB::Red, CRGB::Red, CRGB::Maroon, CRGB::Maroon,
    CRGB::Orange, CRGB::Red, CRGB::Red, CRGB::Black,
    CRGB::Red, CRGB::Red, CRGB::Maroon, CRGB::Maroon,
    CRGB::Orange, CRGB::Red, CRGB::Red, CRGB::Black
  );

  paletteSlime = CRGBPalette16(
    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,
    CRGB::LimeGreen, CRGB::LimeGreen, CRGB::Black, CRGB::Black,
    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,
    CRGB::LimeGreen, CRGB::LimeGreen, CRGB::Black, CRGB::Black
  );

  paletteStasis = CRGBPalette16(
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Indigo, CRGB::Indigo, CRGB::Black, CRGB::Black,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Indigo, CRGB::Indigo, CRGB::Black, CRGB::Black
  );

  paletteMeson = CRGBPalette16(
    CRGB::Yellow, CRGB::Yellow, CRGB::Orange, CRGB::Orange,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Yellow, CRGB::Yellow, CRGB::Orange, CRGB::Orange,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );

  paletteSpectral = CRGBPalette16(
    CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Indigo, CRGB::Violet, CRGB::Black,
    CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Indigo, CRGB::Violet, CRGB::Black
  );

  paletteHalloween = CRGBPalette16(
    CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Purple, CRGB::Purple, CRGB::Purple, CRGB::Purple,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );

  paletteChristmas = CRGBPalette16(
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );

  paletteWhite = CRGBPalette16(
    CRGB::GhostWhite, CRGB::GhostWhite, CRGB::Gainsboro, CRGB::Gainsboro,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::GhostWhite, CRGB::GhostWhite, CRGB::Gainsboro, CRGB::Gainsboro,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );

  // Delay before configuring and running tasks.
  delay(200);

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
  xTaskCreatePinnedToCore(PreferencesTask, "PreferencesTask", 4096, NULL, 4, &PreferencesTaskHandle, 1);

  // Delay all lower priority tasks until Preferences are loaded.
  vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for 100ms to avoid competition.

  // Create a single-run setup task with the highest priority for WiFi/WebServer startup.
  xTaskCreatePinnedToCore(WiFiSetupTask, "WiFiSetupTask", 4096, NULL, 3, &WiFiSetupTaskHandle, 1);

  // Delay all lower priority tasks until WiFi and WebServer setup is done.
  vTaskDelay(200 / portTICK_PERIOD_MS); // Delay for 200ms to avoid competition.

  // Create tasks which utilize a loop for continuous operation (prioritized highest to lowest).
  xTaskCreatePinnedToCore(AnimationTask, "AnimationTask", 2048, NULL, 2, &AnimationTaskHandle, 1);
  xTaskCreatePinnedToCore(WiFiManagementTask, "WiFiManagementTask", 4096, NULL, 1, &WiFiManagementTaskHandle, 0);

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

  // Exception: Run the WebSocket client loop if connected to WiFi.
  if (b_ext_wifi_started && b_socket_ready) {
    wsClient.loop();
  }
}
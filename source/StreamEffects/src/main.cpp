/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2026 Dustin Grau <dustin.grau@gmail.com>
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
#include <FastLED.h>
#include <esp_system.h>
#include <nvs_flash.h>

// Shared Libraries
#include <DeviceState.h>
#include <WirelessManager.h>
#include <WebRouter.h>

// Global instance of DeviceState class for the overall system.
DeviceState gpstarSystem;

// Local Files
#include "Configuration.h"
#include "Header.h"
#include "Colours.h"
#include "Wireless.h"
#include "Webhandler.h"
#include "Webrouting.h"
#include "System.h"

// Writes a debug message to the serial console or sends to the WebSocket.
void sendDebug(const String& message) {
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET)
    if(b_httpd_started) {
      ws.textAll(message); // Send a copy to the WebSocket.
    }
  #endif
}

// Define the WirelessManager pointer globally (initialized to nullptr).
// This matches the extern declaration in Wireless.h
WirelessManager* wirelessMgr = nullptr;

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
      debug(F("Executing AnimationTask in core"));
      debug(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      debug(F(" | Stack HWM: "));
      debugln(uxTaskGetStackHighWaterMark(NULL));
    #endif

    // Update light animation based on websocket data (or self-test mode).
    if(b_firing || gpstarSystem.inStreamMode(SELFTEST)) {
      updateStreamPalette();
      animateLights();
    }
    else {
      // Not firing and not testing, update LED[0] to indicate status.
      if(b_ext_wifi_started) {
        ledsOff();
      }
      else {
        switch(LED_COLOR_TYPE) {
          case LED_RGB:
          default:
            device_leds[0] = getHueAsRGB(PRIMARY_LED, C_PURPLE, 255);
          break;
          case LED_GRB:
            device_leds[0] = getHueAsGRB(PRIMARY_LED, C_PURPLE, 255);
          break;
          case LED_GBR:
            device_leds[0] = getHueAsGBR(PRIMARY_LED, C_PURPLE, 255);
          break;
        }
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

  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Get the stack high water mark for optimizing bytes allocated.
    debug(F("PreferencesTask Stack HWM: "));
    debugln(uxTaskGetStackHighWaterMark(NULL));
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
      debug(F("Executing WiFiManagementTask in core"));
      debug(xPortGetCoreID());
      // Get the stack high water mark for optimizing bytes allocated.
      debug(F(" | Stack HWM: "));
      debugln(uxTaskGetStackHighWaterMark(NULL));
    #endif

    // Handle reconnection to external WiFi when necessary.
    if(b_local_ap_started) {
      if(b_httpd_started && ms_cleanup.remaining() < 1) {
        // Clean up oldest WebSocket connections.
        ws.cleanupClients();

        // Restart timer for next cleanup action.
        ms_cleanup.start(i_websocketCleanup);
      }

      if(ms_apclient.remaining() < 1) {
        // Update the current count of AP clients.
        i_ap_client_count = WiFi.softAPgetStationNum();

        // Restart timer for next count.
        ms_apclient.start(i_apClientDelay);
      }

      if(WiFi.status() == WL_CONNECTED && b_ext_wifi_started) {
        b_ext_wifi_paused = false; // Resume WiFi retries when needed.
        checkWebSocketClient(); // Always check the WebSocket client.
      }
    }

    // Proceed with management if the AP and web server are started.
    if(b_local_ap_started) {
      // Perform periodic checks for WiFi clients and OTA updates.
      webLoops();

      // Try to start the external WiFi.
      if(!b_ext_wifi_started && !b_ext_wifi_paused) {
        if(!gpstarSystem.inStreamMode(SELFTEST)) {
          resetWebSocketData(); // Clear previous information sent from the pack.
        }
        notifyWSClients(); // Notify clients of this device of a change of data.
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
    debug(F("Executing WiFiSetupTask in core"));
    debugln(xPortGetCoreID());
  #endif

  // Define the WirelessManager object only after NVS/Preferences are initialized.
  if(wirelessMgr == nullptr) {
    wirelessMgr = new WirelessManager(WirelessDeviceType::STREAM_EFFECTS, "192.168.2.2");

    #if defined(RESET_AP_SETTINGS)
      // Reset the WiFi password to the expected default on every startup.
      wirelessMgr->resetWifiPassword();
      debugln(F("WARNING: Firmware forced a reset of the local WiFi password!"));
    #endif
  }

  // Set a visual indicator that WiFi is being configured.
  switch(LED_COLOR_TYPE) {
    case LED_RGB:
    default:
      device_leds[0] = getHueAsRGB(PRIMARY_LED, C_RED, 255);
    break;
    case LED_GRB:
      device_leds[0] = getHueAsGRB(PRIMARY_LED, C_RED, 255);
    break;
    case LED_GBR:
      device_leds[0] = getHueAsGBR(PRIMARY_LED, C_RED, 255);
    break;
  }
  FastLED.show();

  // Begin by setting up WiFi as a prerequisite to all else.
  if(startWiFi()) {
    if(b_local_ap_started) {
      // Indicate we've established the private network.
      switch(LED_COLOR_TYPE) {
        case LED_RGB:
        default:
          device_leds[0] = getHueAsRGB(PRIMARY_LED, C_BLUE, 255);
        break;
        case LED_GRB:
          device_leds[0] = getHueAsGRB(PRIMARY_LED, C_BLUE, 255);
        break;
        case LED_GBR:
          device_leds[0] = getHueAsGBR(PRIMARY_LED, C_BLUE, 255);
        break;
      }
      FastLED.show();
    }

    // Start the local web server.
    startWebServer();

    // Begin timer for remote client events.
    ms_cleanup.start(i_websocketCleanup);
    ms_apclient.start(i_apClientDelay);
    ms_otacheck.start(i_otaCheck);
  }

  vTaskDelay(200 / portTICK_PERIOD_MS); // 200ms delay

  // Clear LED once we have the AP and web server started.
  device_leds[0] = CRGB::Black;
  FastLED.show();

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
  // Device RGB LEDs for use when needed.
  FastLED.addLeds<NEOPIXEL, DEVICE_LED_PIN>(device_leds, DEVICE_MAX_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxRefreshRate(0); // Disable FastLED's blocking 2.5ms delay.
  FastLED.setBrightness(255); // Use a highest brightness for visibility.

  // Update all addressable LEDs to prevent stale LED states.
  FastLED.show();

  Serial.begin(115200); // Serial monitor via USB connection.

#if GPSTAR_DEBUG == 1
  // When debugging is enabled, wait for Serial to be ready (max 3 seconds).
  unsigned long startMillis = millis();
  while (!Serial && millis() - startMillis < 3000) {
    delay(10);
  }
  Serial.flush(); // Ensure buffer is clear.
  Serial.println(F("Serial is Ready")); // Should appear after Serial is ready.
#endif

  // Provide an opportunity to set the CPU Frequency MHz: 80, 160, 240 [Default = 240]
  // Lower frequency means less power consumption, but slower performance (obviously).
  setCpuFrequencyMhz(80);
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debug(F("CPU Freq (MHz): "));
    debugln(getCpuFrequencyMhz());
  #endif

  btStop(); // Disable Bluetooth which is not needed for this hardware.

  // Make sure all LEDs are off and set the default palette for stream mode.
  ledsOff();
  updateStreamPalette();

  // Change all possible addressable LEDs to black by default.
  fill_solid(device_leds, DEVICE_MAX_LEDS, CRGB::Black);

  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  // Accesses namespace in read-only mode.
  bool b_namespace_opened = preferences.begin("device", true);
  if(b_namespace_opened) {
    if(preferences.isKey("numLeds")) {
      i_num_leds = preferences.getUShort("numLeds", 250);
    }
    if(preferences.isKey("ledType")) {
      LED_COLOR_TYPE = (LED_COLOR_TYPES)preferences.getUChar("ledType", (uint8_t)LED_RGB);
    }
    preferences.end();
  }

  initializePalettes(); // Set all colour patterns by stream type.
  delay(200); // Delay before configuring and running tasks.

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
  xTaskCreatePinnedToCore(AnimationTask, "AnimationTask", 4096, NULL, 2, &AnimationTaskHandle, 1);
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
  delay(3000);         // Wait 3 seconds before printing again
  #endif

  // Exception: Run the WebSocket client loop if connected to WiFi
  // and the WebSocket client is either CONNECTING or CONNECTED.
  if(b_ext_wifi_started && (wsRemote.status == CONNECTING || wsRemote.status == CONNECTED)) {
    wsRemote.client.loop();
  }
}
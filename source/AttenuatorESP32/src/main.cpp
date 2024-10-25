/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                         & Dustin Grau <dustin.grau@gmail.com>
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
#include <ht16k33.h>
#include <Wire.h>
#include <SerialTransfer.h>
#include <esp_system.h>
#include <nvs_flash.h>

// Local Files
#include "Configuration.h"
#include "Communication.h"
#include "Header.h"
#include "Bargraph.h"
#include "Colours.h"
#include "Serial.h"
#include "Wireless.h"
#include "System.h"

// Task Handles
TaskHandle_t AnimationTaskHandle = NULL;
TaskHandle_t PreferencesTaskHandle = NULL;
TaskHandle_t SerialCommsTaskHandle = NULL;
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

// Obtain a list of partitions for this device.
void printPartitions() {
  const esp_partition_t *partition;
  esp_partition_iterator_t iterator = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if (iterator == nullptr) {
    Serial.println(F("No partitions found."));
    return;
  }

  Serial.println(F("Partitions:"));
  while (iterator != nullptr) {
    partition = esp_partition_get(iterator);
    Serial.printf("Label: %s, Size: %u bytes, Address: 0x%08X\n",
                  partition->label,
                  partition->size,
                  partition->address);
    iterator = esp_partition_next(iterator);
  }

  esp_partition_iterator_release(iterator);  // Release the iterator once done
}

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

    // Call this on each loop in case the user changed their preference.
    if(b_invert_leds) {
      // Flip the identification of the LEDs.
      i_device_led[0] = 2; // Top
      i_device_led[1] = 1; // Upper
      i_device_led[2] = 0; // Lower
    }
    else {
      // Use the expected order for the LEDs.
      // aka. Defaults for the Arduino Nano and ESP32.
      i_device_led[0] = 0; // Top
      i_device_led[1] = 1; // Upper
      i_device_led[2] = 2; // Lower
    }

    // Update LEDs using appropriate colour scheme and environment vars.
    updateLEDs();

    // Update bargraph elements, leveraging cyclotron speed modifier.
    // In reality this multiplier is a divisor to the standard delay.
    bargraphUpdate(i_speed_multiplier);

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

  /*
   * Get Local Device Preferences
   * Accesses the "device" namespace in read-only mode under the "nvs" partition.
   */
  bool b_namespace_opened = preferences.begin("device", true);
  if(b_namespace_opened) {
    // Return stored values if available, otherwise use a default value.
    b_invert_leds = preferences.getBool("invert_led", false);
    b_enable_buzzer = preferences.getBool("use_buzzer", true);
    b_enable_vibration = preferences.getBool("use_vibration", true);
    b_overheat_feedback = preferences.getBool("use_overheat", true);
    b_firing_feedback = preferences.getBool("fire_feedback", false);

    switch(preferences.getShort("radiation_idle", 0)) {
      case 0:
        RAD_LENS_IDLE = AMBER_PULSE;
      break;
      case 1:
        RAD_LENS_IDLE = ORANGE_FADE;
      break;
      case 2:
        RAD_LENS_IDLE = RED_FADE;
      break;
    }

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

    s_track_listing = preferences.getString("track_list", "");
    preferences.end();
  }
  else {
    // If namespace is not initialized, open in read/write mode and set defaults.
    if(preferences.begin("device", false)) {
      preferences.putBool("invert_led", b_invert_leds);
      preferences.putBool("use_buzzer", b_enable_buzzer);
      preferences.putBool("use_vibration", b_enable_vibration);
      preferences.putBool("use_overheat", b_overheat_feedback);
      preferences.putBool("fire_feedback", b_firing_feedback);
      preferences.putShort("radiation_idle", RAD_LENS_IDLE);
      preferences.putShort("display_type", DISPLAY_TYPE);
      preferences.putString("track_list", "");
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

// Serial Comms Task (Loop)
void SerialCommsTask(void *parameter) {
  #if defined(DEBUG_TASK_TO_CONSOLE)
    // Confirm the core in use for this task, and when it runs.
    Serial.print(F("Executing SerialCommsTask in core"));
    Serial.print(xPortGetCoreID());
    // Get the stack high water mark for optimizing bytes allocated.
    Serial.print(F(" | Stack HWM: "));
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
  #endif

  while(true) {
    if(b_wait_for_pack) {
      if(ms_packsync.justFinished()) {
        // Tell the pack we are trying to sync.
        attenuatorSerialSend(A_SYNC_START);

        // Keep the on-board LED dark until sync'd.
        digitalWrite(BUILT_IN_LED, LOW);

        // Pause and try again in a moment.
        ms_packsync.start(i_sync_initial_delay);
      }

      checkPack();

      if(!b_wait_for_pack) {
        // Indicate that we are no longer waiting on the pack.
        digitalWrite(BUILT_IN_LED, HIGH);
      }
    }
    else {
      bool b_notify = checkPack(); // Always updates on pack check.

      // If at any point this flag is true, we have comms open to the pack.
      // This gets reset upon every bootup (read: re-connection to a pack).
      if(b_notify) {
        b_comms_open = true;
      }

      if(ms_packsync.justFinished()) {
        // The pack just went missing, so treat as disconnected.
        b_wait_for_pack = true;
        ms_packsync.start(i_sync_initial_delay);
      }

      /**
       * Alert any WebSocket clients after an API call was received.
       *
       * Note: We only perform this action if we have data from the pack
       * which resulted in a significant state change--this prevents the
       * device from spamming any downstream clients with unchanged data.
       */
      if(b_notify) {
        notifyWSClients(); // Send latest status to the WebSocket.
      }
    }

    vTaskDelay(2 / portTICK_PERIOD_MS); // 2ms delay
  }
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

    if(!b_wait_for_pack) {
      // When not waiting for the pack go directly to checking user inputs.
      checkUserInputs();
    }

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

      if(ms_cleanup.remaining() < 1) {
        // Update the current count of AP clients.
        i_ap_client_count = WiFi.softAPgetStationNum();

        // Restart timer for next count.
        ms_apclient.start(i_apClientCount);
      }

      if(ms_cleanup.remaining() < 1) {
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

  // Expect a Serial2 connection with communication to a GPStar Proton Pack PCB.
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  packComs.begin(Serial2, false);

  // Prepare the on-board (non-power) LED to be used as an output pin for indication.
  pinMode(BUILT_IN_LED, OUTPUT);

  // Provide an opportunity to set the CPU Frequency MHz: 80, 160, 240 [Default = 240]
  // Lower frequency means less power consumption, but slower performance (obviously).
  setCpuFrequencyMhz(240);
  #if defined(DEBUG_SEND_TO_CONSOLE)
    Serial.print(F("CPU Freq (MHz): "));
    Serial.println(getCpuFrequencyMhz());
  #endif

  // Assume the Super Hero arming mode with Afterlife (default for Haslab).
  SYSTEM_MODE = MODE_SUPER_HERO;
  RED_SWITCH_MODE = SWITCH_OFF;
  SYSTEM_YEAR = SYSTEM_AFTERLIFE;

  // Boot into proton mode (default for pack and wand).
  STREAM_MODE = PROTON;

  // Set a default animation for the radiation indicator.
  RAD_LENS_IDLE = AMBER_PULSE;
  DISPLAY_TYPE = STATUS_TEXT;

  // Begin at menu level one. This affects the behavior of the rotary dial.
  MENU_LEVEL = MENU_1;

  if(!b_wait_for_pack) {
    // If not waiting for the pack set power level to 5.
    POWER_LEVEL = LEVEL_5;
  }
  else {
    // When waiting for the pack set power level to 1.
    POWER_LEVEL = LEVEL_1;
  }

  // RGB LEDs for effects (upper/lower) and user status (top).
  FastLED.addLeds<NEOPIXEL, DEVICE_LED_PIN>(device_leds, DEVICE_NUM_LEDS);

  // Set all LEDs as off (black) until the device is ready.
  device_leds[0] = getHueAsRGB(0, C_BLACK);
  device_leds[1] = getHueAsRGB(1, C_BLACK);
  device_leds[2] = getHueAsRGB(2, C_BLACK);

  // Debounce the toggle switches and encoder pushbutton.
  switch_left.setDebounceTime(switch_debounce_time);
  switch_right.setDebounceTime(switch_debounce_time);
  encoder_center.setDebounceTime(switch_debounce_time);

  // Rotary encoder on the top of the Attenuator.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(r_encoderA), readEncoder, CHANGE);

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();

  // Feedback devices (piezo buzzer and vibration motor)
  pinMode(BUZZER_PIN, OUTPUT);
  setToneChannel(0); // Forces Tone to use Channel 0.

  // Use the combined method for the arduino-esp32 platform, using the esp-idf v5.3+
  ledcAttachChannel(VIBRATION_PIN, 5000, 8, 5); // Uses 5 kHz frequency, 8-bit resolution, channel 5

  // Turn off any user feedback.
  buzzOff();
  vibrateOff();

  // Get initial switch/button states.
  switchLoops();

  // Delay before configuring and running tasks.
  delay(200);

  // Initialize a critical timer for serial comms.
  if(b_wait_for_pack) {
    ms_packsync.start(0);
  }

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
  xTaskCreatePinnedToCore(PreferencesTask, "PreferencesTask", 4096, NULL, 6, &PreferencesTaskHandle, 1);

  // Delay all lower priority tasks until Preferences are loaded.
  vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for 100ms to avoid competition.

  // Create a single-run setup task with the highest priority for WiFi/WebServer startup.
  xTaskCreatePinnedToCore(WiFiSetupTask, "WiFiSetupTask", 4096, NULL, 5, &WiFiSetupTaskHandle, 1);

  // Delay all lower priority tasks until WiFi and WebServer setup is done.
  vTaskDelay(200 / portTICK_PERIOD_MS); // Delay for 200ms to avoid competition.

  // Create tasks which utilize a loop for continuous operation (prioritized highest to lowest).
  xTaskCreatePinnedToCore(SerialCommsTask, "SerialCommsTask", 4096, NULL, 4, &SerialCommsTaskHandle, 1);
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
  if (SerialCommsTaskHandle != NULL) {
    Serial.print(F("|--Serial Comms: "));
    Serial.print(formatBytesWithCommas(uxTaskGetStackHighWaterMark(SerialCommsTaskHandle)));
    Serial.println(F(" / 4,096 bytes"));
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

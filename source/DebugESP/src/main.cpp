// Required for PlatformIO
#include <Arduino.h>

// Set to 1 to enable built-in debug messages
#define DEBUG 1

// Debug macros
#if DEBUG == 1
  #ifdef ESP32
    #define DEBUG_PORT USBSerial
  #else
    #define DEBUG_PORT Serial
  #endif

  #define debug(...) DEBUG_PORT.print(__VA_ARGS__)
  #define debugf(...) DEBUG_PORT.printf(__VA_ARGS__)
  #define debugln(...) DEBUG_PORT.println(__VA_ARGS__)
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
#include <SerialTransfer.h>
#include <Wire.h>
#include <HardwareSerial.h>

// Libraries
#include "esp_system.h"

// For the Serial1 add-on device
#ifndef SERIAL1_TX_PIN
  #define SERIAL1_TX_PIN 10
#endif
#ifndef SERIAL1_RX_PIN
  #define SERIAL1_RX_PIN 11
#endif

// For the Neutrona Wand
#ifndef SERIAL2_TX_PIN
  #define SERIAL2_TX_PIN 43 // TXD0 = 43
#endif
#ifndef SERIAL2_RX_PIN
  #define SERIAL2_RX_PIN 44 // RXD0 = 44
#endif

// For the audio device
#ifndef SERIAL3_RX_PIN
  #define SERIAL3_RX_PIN 15
#endif
#ifndef SERIAL3_TX_PIN
  #define SERIAL3_TX_PIN 16
#endif

// For the i2c Bus
#define I2C_SCL 39
#define I2C_SDA 40

/*
 * Define Serial Communication Buffers
 */
SerialTransfer serial1Coms;
SerialTransfer packComs;

const char* resetReasonToString(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:    return "Power-on reset";
    case ESP_RST_EXT:        return "External reset";
    case ESP_RST_SW:         return "Software reset";
    case ESP_RST_PANIC:      return "Panic reset";
    case ESP_RST_INT_WDT:    return "Interrupt watchdog";
    case ESP_RST_TASK_WDT:   return "Task watchdog";
    case ESP_RST_WDT:        return "Other watchdog reset";
    case ESP_RST_DEEPSLEEP:  return "Deep sleep reset";
    case ESP_RST_BROWNOUT:   return "Brownout reset";
    case ESP_RST_SDIO:       return "SDIO reset";
    default:                 return "Unknown reset reason";
  }
}

void setup() {
  // Standard serial (USB-CDC) console (technically 19/20 but not really Tx/Rx).
  USBSerial.begin(9600);
  while (!USBSerial) delay(10);

  esp_reset_reason_t reason = esp_reset_reason();
  debugf("Reset reason: %s (%d)\n", resetReasonToString(reason), reason);

  /* This loop changes GPIO39~GPIO44 to Function 1, which is GPIO.
   * PIN_FUNC_SELECT sets the IOMUX function register appropriately.
   * IO_MUX_GPIO0_REG is the register for GPIO0, which we then seek from.
   * PIN_FUNC_GPIO is a define for Function 1, which sets the pins to GPIO mode.
   */
  for(uint8_t gpio_pin = 39; gpio_pin < 45; gpio_pin++) {
    PIN_FUNC_SELECT(IO_MUX_GPIO0_REG + (gpio_pin * 4), PIN_FUNC_GPIO);
  }

  // Serial0.end(); // Detach UART0 as we will be reassigning it.

  // Assign Serial1 to UART1 (pins 11/10) for the "Serial1" communications (aka. serial1Coms).
  HardwareSerial Serial1(1);
  Serial1.begin(9600, SERIAL_8N1, SERIAL1_RX_PIN, SERIAL1_TX_PIN);

  // Assign Serial2 to UART0 (pins 44/43) for the Neutrona Wand communications (aka. packComs).
  HardwareSerial Serial2(0);
  Serial2.end(); // Detach UART0 first to avoid conflicts.
  Serial2.begin(9600, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN);

  // Connect the serial ports.
  debugln(F("Connecting serial ports..."));
  serial1Coms.begin(Serial1, false, DEBUG_PORT, 100); // Attenuator/Wireless
  packComs.begin(Serial2, false); // Neutrona Wand

  // Setup the audio device for this controller (configures UART2 as Serial3).
  debugln(F("Setting up audio device..."));

  // Create a HardwareSerial instance for UART2 (Serial3, pins 15/16)
  HardwareSerial Serial3(2); // 2 = UART2
  Serial3.begin(57600, SERIAL_8N1, SERIAL3_RX_PIN, SERIAL3_TX_PIN);

  // Setup the i2c bus using the Wire protocol.
  // ESP32-S3 requires manually specifying SDA and SCL pins first.
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
  Wire.setClock(400000UL); // Sets the i2c bus to 400kHz

  debugf("Setup complete, free heap: %u bytes\n", ESP.getFreeHeap());
}

void loop() {
  debugln(F("Main loop running..."));
  delay(1000); // Just a delay to prevent flooding the debug output
}

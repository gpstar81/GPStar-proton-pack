// Required for PlatformIO
#include <Arduino.h>

// Set to 1 to enable built-in debug messages
#define DEBUG 1

// Define how we get console output
#ifdef ESP32
  // Requires that we use the ESP32 in USB peripheral mode: ARDUINO_USB_MODE=0
  // Also requires enabling USB CDC interface on boot: ARDUINO_USB_CDC_ON_BOOT=1
  // Our intended serial console is technically pins 19/20 but not really Tx/Rx
  #define DEBUG_PORT USBSerial
#else
  #define DEBUG_PORT Serial
#endif

// Debug macros
#if DEBUG == 1
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
#include <INA219.h>

// Libraries
#ifdef ESP32
  #include "esp_system.h"
#endif

// Define the serial ports for use by hardware
#ifdef ESP32
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

  // In order to reassign and use UART0 we must burn eFuses:
  // UART_PRINT_CONTROL=3 and DIS_PAD_JTAG=1
  #define WAND_DEVICE Serial2
  #define SERIAL1_DEVICE Serial1
  #define AUDIO_UARTID 0
  HardwareSerial AUDIO_DEVICE(AUDIO_UARTID);
#else
  // Hardware serial ports are used as-is for ATMega2560
  #define WAND_DEVICE Serial2
  #define SERIAL1_DEVICE Serial1
  #define AUDIO_DEVICE Serial3
#endif

// For the i2c Bus
#define I2C_SCL 39
#define I2C_SDA 40

/*
 * Define Serial Communication Buffers
 */
SerialTransfer serial1Coms;
SerialTransfer packComs;

/*
 * Power monitor object on i2c bus using the INA219 chip.
 */
INA219 monitor;

#ifdef ESP32
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
#endif

void setup() {
  digitalWrite(LED_BUILTIN, false); // Turn off the built-in LED

  DEBUG_PORT.begin(9600);
  while (!DEBUG_PORT) {
    delay(10);
  }

#ifdef ESP32
  // On init of the ESP32 system we print the reset reason.
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
#endif

#ifdef ESP32
  // Assign Serial1 to UART (pins 11/10) for the "Serial1" communications (aka. serial1Coms).
  SERIAL1_DEVICE.begin(9600, SERIAL_8N1, SERIAL1_RX_PIN, SERIAL1_TX_PIN);

  // Assign Serial2 to UART (pins 44/43) for the Neutrona Wand communications (aka. packComs).
  WAND_DEVICE.begin(9600, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN);

  // Setup the audio device for this controller (configures UART0 as Serial3).
  // Create a HardwareSerial instance for UART2 (Serial3, pins 15/16)
  debugln(F("Setting up audio device..."));
  AUDIO_DEVICE.begin(57600, SERIAL_8N1, SERIAL3_RX_PIN, SERIAL3_TX_PIN);

  // Setup the i2c bus using the Wire protocol.
  // ESP32-S3 requires manually specifying SDA and SCL pins first.
  Wire.setPins(I2C_SDA, I2C_SCL);
#else
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(57600);
#endif

  // Connect the serial ports.
  debugln(F("Connecting serial ports..."));
  serial1Coms.begin(SERIAL1_DEVICE, false, DEBUG_PORT, 100); // Attenuator/Wireless
  packComs.begin(WAND_DEVICE, false); // Neutrona Wand

  debugln(F("Setting up i2c device..."));
  Wire.begin();
  Wire.setClock(400000UL); // Sets the i2c bus to 400kHz
  uint8_t i_monitor_status = monitor.begin();
  debug(F("Power Meter Result: "));
  debugln(i_monitor_status);

#ifdef ESP32
  debugf("Setup complete, free heap: %u bytes\n", ESP.getFreeHeap());
#endif
}

void loop() {
#ifdef ESP32
  digitalWrite(LED_BUILTIN, true); // Turn on the built-in LED
#endif
  debugln(F("Main loop running..."));
  delay(1000); // Just a delay to prevent flooding the debug output

  // Access the serial1Coms and packComs objects to confirm they are working.
  if (serial1Coms.available()) {
    // Process incoming data from serial1Coms
    debugln(F("Data received on serial1Coms"));
  }
  if (packComs.available()) {
    // Process incoming data from packComs
    debugln(F("Data received on packComs"));
  }
}

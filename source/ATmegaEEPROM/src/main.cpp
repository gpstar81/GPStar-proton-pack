// Required for PlatformIO
#include <Arduino.h>

// Set to 1 to enable built-in debug messages
#define DEBUG 0

// Debug macros
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// 3rd-Party Libraries
#include <EEPROM.h>

// Function: clearEEPROM
// Purpose: Clears the entire EEPROM by writing 0xFF to all addresses.
void clearEEPROM() {
  for (uint16_t i = 0; i < EEPROM.length(); i++) {
    EEPROM.update(i, 0xFF); // Write 0xFF to each address
  }
  debugln(F("EEPROM cleared."));
}

// Function: isEEPROMCleared
// Purpose: Checks if the entire EEPROM is cleared (all bytes are 0xFF).
// Returns: true if cleared, false otherwise.
bool isEEPROMCleared() {
  for (uint16_t i = 0; i < EEPROM.length(); i++) {
    if (EEPROM.read(i) != 0xFF) {
      return false; // Found a non-cleared byte
    }
  }
  return true; // All bytes are cleared
}

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  debugln(F("EEPROM check started."));
}

void loop() {
  // Check if EEPROM is cleared (read: all bytes are 0xFF)
  if (!isEEPROMCleared()) {
    debugln(F("EEPROM is not cleared. Clearing now..."));
    clearEEPROM(); // Clear the EEPROM
  } else {
    debugln(F("EEPROM is already cleared."));
  }

  delay(10000); // Wait for 10 seconds before checking again
}

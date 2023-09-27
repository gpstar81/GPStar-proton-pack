// 3rd-Party Libraries
#include <millisDelay.h>
#include <ht16k33.h>
#include <Wire.h>

// Local Files
#include "Header.h"
#include "Bargraph.h"

void setup() {
  Serial.begin(9600);

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();

  // Set defaults for testing.
  POWER_LEVEL = LEVEL_5;
  i_speed_multiplier = 1;
}

void loop() {
  mainLoop(); // Mimic normal pattern for other devices.
}

void mainLoop() {
  if(BARGRAPH_STATE == BG_OFF) {
    bargraphReset(); // Turns on bargraph, enabling animations.
  }
  //BARGRAPH_STATE = BG_OFF; // Disables all bargraph animations.

  bargraphPowerCheck(POWER_LEVEL); // Ramps bargraph up and down.
  //BARGRAPH_PATTERN = BG_OUTER_INNER; // Standard firing pattern.
  //BARGRAPH_PATTERN = BG_INNER_PULSE; // Modified firing pattern.

  // Update bargraph elements using some speed modifier.
  // In reality this multiplier is a divisor to the standard delay.
  bargraphUpdate(i_speed_multiplier);
}
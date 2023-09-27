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

  // Initialize critical timers.
  ms_bargraph.start(i_bargraph_delay);

  // Set defaults for testing.
  POWER_LEVEL = LEVEL_5;
  i_speed_multiplier = 1;
}

void loop() {
  mainLoop();
}

void mainLoop() {
  //BARGRAPH_STATE = BG_OFF; // Completely disables bargraph updates.

  bargraphPowerCheck(POWER_LEVEL); // Ramps bargraph up and down.
  //BARGRAPH_PATTERN = BG_OUTER_INNER; // Standard firing pattern.
  //BARGRAPH_PATTERN = BG_INNER_PULSE; // Modified firing pattern.

  // Update bargraph elements.
  bargraphUpdate(i_speed_multiplier);
}
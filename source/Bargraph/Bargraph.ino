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
}

void loop() {
  mainLoop();
}

void mainLoop() {
  // Set the bargraph pattern.
  if(BARGRAPH_STATE == BG_EMPTY) {
    BARGRAPH_PATTERN = BG_RAMP_UP;
  }
  else {
    BARGRAPH_PATTERN = BG_RAMP_DOWN;
  }

  // Update bargraph elements.
  bargraphUpdate();
}
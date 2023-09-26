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
  ms_bargraph.start(1);
}

void loop() {
  mainLoop();
}

void mainLoop() {
  BARGRAPH_PATTERN = BG_RAMP_UP; // Set the bargraph pattern

  if(ms_bargraph.justFinished()) {
    bargraphUpdate();
  }
}
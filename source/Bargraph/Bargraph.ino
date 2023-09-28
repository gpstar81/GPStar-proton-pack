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

  // Start the demo timer.
  ms_demo.start(1000);
}

void loop() {
  mainLoop(); // Mimic normal pattern for other devices.
}

void mainLoop() {
  if(ms_demo.remaining() == 0) {
    incrementDemo();
  }

  runDemo(); // Sets the demo to run on the bargraph.

  // Update bargraph elements using some speed modifier.
  // In reality this multiplier is a divisor to the standard delay.
  bargraphUpdate(i_speed_multiplier);
}

void incrementDemo() {
  // Increments through values 0-5, resets after that.
  i_demo = (i_demo + 1) % 6;
}

void runDemo() {
  switch(i_demo) {
    case 0:
      if(ms_demo.remaining() == 0) {
        bargraphOff();
        ms_demo.start(2000);
      }
    break;

    case 1:
      // Prepare bargraph for next pattern.
      if(ms_demo.remaining() == 0) {
        bargraphReset();
        i_speed_multiplier = 3;
        POWER_LEVEL = LEVEL_5;
        BARGRAPH_PATTERN = BG_POWER_RAMP; // Ramp up/down to set power level.
        ms_demo.start(7000);
        ms_power.start(4000);
        ms_speed.start(2000);
      }
      if(ms_power.justFinished()) {
        POWER_LEVEL = LEVEL_3;
      }
      if(ms_speed.justFinished()) {
        i_speed_multiplier = 1;
      }
    break;

    case 2:
      if(ms_demo.remaining() == 0) {
        bargraphReset();
        i_speed_multiplier = 1;
        BARGRAPH_PATTERN = BG_OUTER_INNER; // Standard firing pattern.
        ms_demo.start(4000);
        ms_speed.start(2000);
      }
      if(ms_speed.justFinished()) {
        i_speed_multiplier = 3;
      }
    break;

    case 3:
      if(ms_demo.remaining() == 0) {
        bargraphReset();
        i_speed_multiplier = 1;
        BARGRAPH_PATTERN = BG_INNER_PULSE; // Modified firing pattern.
        ms_demo.start(4000);
        ms_speed.start(2000);
      }
      if(ms_speed.justFinished()) {
        i_speed_multiplier = 3;
      }
    break;

    case 4:
      if(ms_demo.remaining() == 0) {
        bargraphClear();
        bargraphReset();
        i_speed_multiplier = 1;
        BARGRAPH_PATTERN = BG_RAMP_UP;
        ms_demo.start(3000);
      }
    break;

    case 5:
      if(ms_demo.remaining() == 0) {
        bargraphFull();
        i_speed_multiplier = 1;
        BARGRAPH_PATTERN = BG_RAMP_DOWN;
        ms_demo.start(3000);
      }
    break;
  }
}
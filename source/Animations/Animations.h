// Handles all animations for LEDs

enum RAMP_STATES { NO_RAMP, RAMP_UP, RAMP_DOWN };

// Structure to hold the parameters for each ring of LEDs
struct RingParams {
  LED_DEVICES deviceName = NO_DEVICE; // Which system LED device 
  CRGB* ledsArray;                    // Pointer to the LED array
  uint8_t numSteps = 0;               // Number of "steps" to take for a revolution (0 = Use numLEDs)
  uint8_t numLEDs = 0;                // True number of LEDs in the ring (up to 255)
  uint16_t revolutionTime = 0;        // Time for one complete revolution, in milliseconds
  bool spinClockwise = true;          // Spin clockwise (true) or anti-clockwise (false)
  CRGB ledColor = CRGB::Red;          // Color of the LEDs (default: Red)
  millisDelay ledTimer;               // Timer object for next change
  uint8_t currentLED = 0;             // Current LED index
  uint8_t previousLED = 0;            // Previous LED index
  uint8_t nextLED = 0;                // Next LED index
  RAMP_STATES rampState = NO_RAMP;    // Current ramping state
  uint16_t rampTime = 0;              // Time for ramp up/down, in milliseconds
  uint16_t rampStep = 0;              // Current step in the ramp
};

// Resets a ring for the next animation sequence
void resetRing(RingParams &ring) {
  ring.ledTimer.stop();
  ring.currentLED = 0;
  ring.previousLED = 0;
  ring.nextLED = 0;
  ring.rampState = NO_RAMP;
  ring.rampStep = 0;
}

/**
 * Function to animate the LED ring using steps per revolution and a time to complete
 * a single revolution. This does the math for us so that each LED will be lit for a
 * consistent time while animating around a ring. This should support movement in a
 * clockwise or anti-clockwise direction, color changes, speed, and ramping with many
 * options being changeable at runtime or mid-animation. This will also use a "tail"
 * effect by having a dimmer LED chase behind the current LED lit to the brightness
 * set by the ledColor option.
 */
bool animateRing(RingParams &ring) {
  // If the revolution time is 0 then consider the animation as complete
  // Also leave if other crucial values are missing or set to zero
  if(ring.revolutionTime == 0 || ring.numLEDs == 0 || ring.revolutionTime == 0) {
    resetRing(ring);
    return false;
  }

  if(ring.numSteps == 0) {
    ring.numSteps = ring.numLEDs; // When unset, use actual LEDs as steps
  }

  // Calculate the delay time for each LED based on steps per revolution
  // Takes into account the ramping state to calculate the delay time
  uint16_t delayTime;
  switch(ring.rampState) {
    case NO_RAMP:
    default:
      delayTime = ring.revolutionTime / ring.numSteps; // Get a constant time between LEDs
      break;

    case RAMP_UP:
      // Perform a logarithmic ramp-up 
      delayTime = ring.revolutionTime / ring.numSteps * pow(10, (float)ring.rampStep / ring.numSteps - 1);
      ring.rampStep++;
      if (ring.rampStep >= ring.numSteps) {
        ring.rampState = NO_RAMP;
      }
      break;

    case RAMP_DOWN:
      delayTime = ring.revolutionTime / ring.numSteps * pow(10, 1 - (float)ring.rampStep / ring.numSteps);
      ring.rampStep++;
      if (ring.rampStep >= ring.numSteps) {
        resetRing(ring);
        return false;
      }
      break;
  }

  // Start the delay if it hasn't been started
  if (!ring.ledTimer.isRunning()) {
    ring.ledTimer.start(delayTime);
  }

  // Check if the delay has timed out
  if (ring.ledTimer.justFinished()) {
    if(ring.deviceName != NO_DEVICE) {
      // Turn off the previous pixel but only if not on the first iteration around the ring
      if(ring.previousLED != ring.currentLED) {
        ring.ledsArray[ring.previousLED] = CRGB::Black;
      }

      // Set the current LED to a portion of its current brightness for a tail effect
      // This MUST be done before the "next", to account for the 0 index at startup
      ring.ledsArray[ring.currentLED] = blend(ring.ledColor, CRGB::Black, 128);

      // Turn on the next LED using the color (with saturation and brightness) as specified
      // On the first revolution all 3 values will be the same, so this will be the first LED
      ring.ledsArray[ring.nextLED] = ring.ledColor;

      // Show changes to the LEDs
      FastLED.show();

      // Shift the current LED to be the previous LED, and the next LED as the current
      ring.previousLED = ring.currentLED;
      ring.currentLED = ring.nextLED;

      // Set the next LED as based on the direction of spin
      if (ring.spinClockwise) {
        ring.nextLED = (ring.currentLED + 1) % ring.numLEDs; // Clockwise
      } else {
        ring.nextLED = (ring.currentLED - 1 + ring.numLEDs) % ring.numLEDs; // Anti-clockwise
      }
    }

    // Restart the delay in case the calculated value changed due to the revolution time
    ring.ledTimer.start(delayTime);

    if(ring.currentLED == 0 && ring.currentLED != ring.previousLED) {
      return true; // Returned to position 0, meaning a full revolution completed
    }
  }

  return false; // Return false to indicate a complete revolution has not occurred
}

// Function to ramp up the animation speed
void ringRampUp(RingParams &ring) {
  for (uint16_t time = ring.rampTime / ring.rampStep; time <= ring.rampTime; time += (ring.rampTime / ring.rampStep)) {
    debugln(time);
    animateRing(ring);
  }
}

// Function to ramp down the animation speed
void ringRampDown(RingParams &ring) {
  for (uint16_t time = ring.rampTime; time >= ring.rampTime / ring.rampStep; time -= (ring.rampTime / ring.rampStep)) {
    animateRing(ring);
  }
}
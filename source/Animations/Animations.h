// Handles all animations for LEDs

enum RAMP_STATES { NO_RAMP, RAMP_UP, RAMP_DOWN };

// Structure to hold the parameters for each ring of LEDs
struct RingParams {
  // Values which will not get changed once set:
  LED_DEVICES deviceName = NO_DEVICE; // Which system LED device 
  CRGB* ledsArray;                    // Pointer to the LED array
  uint8_t numSteps = 0;               // Number of "steps" to take for a revolution (0 = Use numLEDs)
  uint8_t numLEDs = 0;                // True number of LEDs in the ring (up to 255)
  millisDelay ledTimer;               // Timer object for next change
  rampInt rampSpeed;                  // Interpolation object for ramping speed
  // Values which may be changed at runtime:
  uint16_t revolutionTime = 0;        // Time for one complete revolution, in milliseconds
  bool spinClockwise = true;          // Spin clockwise (true) or anti-clockwise (false)
  CRGB ledColor = CRGB::Red;          // Color of the LEDs (default: Red)
  uint8_t currentLED = 0;             // Current LED index (based on numSteps)
  uint8_t previousLED = 0;            // Previous LED index (based on numSteps)
  uint8_t nextLED = 0;                // Next LED index (based on numSteps)
  RAMP_STATES rampState = NO_RAMP;    // Current ramping state for animation
  uint16_t rampTime = 0;              // Time for ramp up/down, in milliseconds
};

// Resets a ring for the next animation sequence
void resetRing(RingParams &ring) {
  ring.ledTimer.stop();
  ring.currentLED = 0;
  ring.previousLED = 0;
  ring.nextLED = 0;
  ring.rampState = NO_RAMP;
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
  if(ring.revolutionTime == 0 || ring.numLEDs == 0) {
    resetRing(ring);
    return false;
  }

  if(ring.numSteps == 0) {
    ring.numSteps = ring.numLEDs; // When unset, use actual LEDs as steps
  }

  // Start with the standard calculation for delay using steps per revolution
  uint16_t delayTime = ring.revolutionTime / ring.numSteps;

  // Start the delay if it hasn't been started
  if (!ring.ledTimer.isRunning()) {
    ring.ledTimer.start(delayTime);
  }

  // Check if the delay has timed out
  if (ring.ledTimer.justFinished()) {
    // Calculate the delay time for each LED based on steps per revolution
    // Takes into account the ramping state to calculate the delay time
debug("Standard Delay: ");
debugln(delayTime);
    switch(ring.rampState) {
      case NO_RAMP:
      default:
        // No change from the standard calculation
        break;

      case RAMP_UP:
        if (ring.rampSpeed.isFinished()) {
          // Completion of ramp-up implies the animation continues normally
          ring.rampState = NO_RAMP;
        } else {
          delayTime = ring.rampSpeed.getValue();
        }
        break;

      case RAMP_DOWN:
        if (ring.rampSpeed.isFinished()) {
          // Completion of ramp-down implies the animation is over, period
          ring.revolutionTime = 0;
          resetRing(ring);
          return false;
        } else {
          delayTime = ring.rampSpeed.getValue();
        }
        break;
    }
debug("Current Delay: ");
debugln(delayTime);
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

    // Update the ramp speed if ramping
    if(ring.rampState != NO_RAMP) {
      ring.rampSpeed.update();
    }

    if(ring.nextLED == 0 && ring.currentLED != ring.previousLED) {
      return true; // Returned to position 0, meaning a full revolution completed
    }
  }

  return false; // Return false to indicate a complete revolution has not occurred
}

// Function to ramp up the animation speed
void ringRampUp(RingParams &ring) {
  ring.rampState = RAMP_UP;
  ring.rampSpeed.setGrain(1);
  ring.rampSpeed.go(ring.revolutionTime, ring.rampTime, QUADRATIC_IN);
}

// Function to ramp down the animation speed
void ringRampDown(RingParams &ring) {
  ring.rampState = RAMP_DOWN;
  ring.rampSpeed.setGrain(1);
  ring.rampSpeed.go(ring.revolutionTime, ring.rampTime, QUADRATIC_OUT);
}
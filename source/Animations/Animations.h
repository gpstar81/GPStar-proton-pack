// Handles all animations for LEDs

// Structure to hold the parameters for each ring
struct RingParams {
  CRGB* ledsDevice;         // Pointer to the LED array
  uint8_t numLEDs;          // Number of LEDs in the ring
  uint16_t revolutionTime;  // Time for one complete rotation, in milliseconds
  CRGB color;               // Color of the LEDs
  uint8_t maxBrightness;    // Maximum brightness of the LEDs
  millisDelay ledDelay;     // Delay object for timing
  uint16_t rampTime;        // Time for ramp up/down, in milliseconds
  uint16_t rampStep;        // Increase speed by some factor per LED
  uint8_t currentLED;       // Current LED index
};

float iMinBrightness = 0.10; // Minimum brightness for ring tail

// Function to animate the LED ring
bool animateRing(RingParams &ring) {
  // Calculate the delay time for each LED
  unsigned long delayTime = ring.revolutionTime / ring.numLEDs;

  // Start the delay if it hasn't been started
  if (!ring.ledDelay.isRunning()) {
    ring.ledDelay.start(delayTime);
  }

  // Check if the delay has timed out
  if (ring.ledDelay.justFinished()) {
    // Update the LEDs to create the tail effect
    for (int i = 0; i < ring.numLEDs; i++) {
      // Calculate the current brightness
      uint8_t currentBrightness = ring.ledsDevice[i].getAverageLight();
      if (currentBrightness > (ring.maxBrightness * iMinBrightness)) {  // Check if above min brightness threshold
        ring.ledsDevice[i].fadeToBlackBy(ring.maxBrightness / 2);  // Reduce current brightness by 50%
      } else {
        ring.ledsDevice[i] = CRGB::Black;  // Turn off if below min brightness
      }
    }

    // Turn on the current LED at the max brightness specified
    ring.ledsDevice[ring.currentLED] = ring.color;
    ring.ledsDevice[ring.currentLED].nscale8(ring.maxBrightness); // Scale the brightness to maxBrightness

    // Show changes to the LEDs
    FastLED.show();

    // Move to the next LED in the ring (starting over as necessary)
    ring.currentLED = (ring.currentLED + 1) % ring.numLEDs;

    // Restart the delay
    ring.ledDelay.repeat();
  }

  return true; // Return true once completed.
}

// Function to ramp up the animation speed
bool ringRampUp(RingParams &ring) {
  for (uint16_t time = ring.rampTime / ring.rampStep; time <= ring.rampTime; time += (ring.rampTime / ring.rampStep)) {
    if(!animateRing(ring)) {
      return false; // Return false if animation is interrupted.
    }
  }

  return true; // Return true once completed.
}

// Function to ramp down the animation speed
bool ringRampDown(RingParams &ring) {
  for (uint16_t time = ring.rampTime; time >= ring.rampTime / ring.rampStep; time -= (ring.rampTime / ring.rampStep)) {
    if(!animateRing(ring)) {
      return false; // Return false if animation is interrupted.
    }
  }

  return true; // Return true once completed.
}
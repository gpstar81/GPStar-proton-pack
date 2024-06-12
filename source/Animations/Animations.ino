// Ring Animation Tests

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
#include <millisDelay.h>
#include <FastLED.h>

// Define the data pins for the LED rings
#define INNER_RING_PIN 13
#define OUTER_RING_PIN 53

// Structure to hold the parameters for each ring
struct RingParams {
  CRGB* leds;               // Pointer to the LED array
  uint8_t numLEDs;          // Number of LEDs in the ring
  uint16_t revolutionTime;  // Time for one complete rotation in milliseconds
  CRGB color;               // Color of the LEDs
  uint8_t maxBrightness;    // Maximum brightness of the LEDs
  millisDelay ledDelay;     // Delay object for timing
  uint16_t rampTime;        // Time for ramp up and ramp down in milliseconds
  int currentLED;           // Current LED index
};

// Initialize the LED arrays for each ring
CRGB ledsInner[16];
CRGB ledsOuter[24];

// Create structures to hold the parameters for each ring
RingParams innerRing;
RingParams outerRing;

// Function to animate the LED ring
void animateRing(RingParams &ring) {
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
      uint8_t currentBrightness = ring.leds[i].getAverageLight();
      if (currentBrightness > (ring.maxBrightness * 0.25)) {  // 25% brightness threshold
        ring.leds[i].fadeToBlackBy(ring.maxBrightness / 2);  // Reduce brightness by 50%
      } else {
        ring.leds[i] = CRGB::Black;  // Turn off if below 25% brightness
      }
    }

    // Turn on the current LED at the max brightness
    ring.leds[ring.currentLED] = ring.color;
    ring.leds[ring.currentLED].nscale8(ring.maxBrightness); // Scale the brightness to maxBrightness

    // Show the LEDs
    FastLED.show();

    // Move to the next LED
    ring.currentLED = (ring.currentLED + 1) % ring.numLEDs;

    // Restart the delay
    ring.ledDelay.repeat();
  }
}

// Function to ramp up the animation speed
void ringRampUp(RingParams &ring) {
  for (uint16_t time = ring.rampTime / 10; time <= ring.rampTime; time += (ring.rampTime / 10)) {
    animateRing(ring);
  }
}

// Function to ramp down the animation speed
void ringRampDown(RingParams &ring) {
  for (uint16_t time = ring.rampTime; time >= ring.rampTime / 10; time -= (ring.rampTime / 10)) {
    animateRing(ring);
  }
}

void setup() {
  // Initialize the FastLED library for each ring
  FastLED.addLeds<NEOPIXEL, INNER_RING_PIN>(ledsInner, 16);
  FastLED.addLeds<NEOPIXEL, OUTER_RING_PIN>(ledsOuter, 24);

  // Define parameters for the inner ring
  innerRing.leds = ledsInner;
  innerRing.numLEDs = 16;
  innerRing.revolutionTime = 1000;
  innerRing.color = CRGB::Red;
  innerRing.maxBrightness = 255;
  innerRing.rampTime = 2000;
  innerRing.currentLED = 0;

  // Define parameters for the outer ring
  outerRing.leds = ledsOuter;
  outerRing.numLEDs = 24;
  outerRing.revolutionTime = 1500;
  outerRing.color = CRGB::Blue;
  outerRing.maxBrightness = 200;
  outerRing.rampTime = 2500;
  outerRing.currentLED = 0;
}

void loop() {
  // Perform the ramp up, normal animation, and ramp down for each ring
  ringRampUp(innerRing);
  animateRing(innerRing);
  ringRampDown(innerRing);

  ringRampUp(outerRing);
  animateRing(outerRing);
  ringRampDown(outerRing);
}

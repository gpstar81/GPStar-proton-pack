// Ring Animation Tests

// Set to 1 to enable built-in debug messages
#define DEBUG 1

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

// Local Files
#include "Animations.h"

// Define the data pins for the LED rings
#define INNER_RING_PIN 13
#define OUTER_RING_PIN 53

#define MAX_INNER_LEDS 40
#define MAX_OUTER_LEDS 40

// Initialize the LED arrays for each ring
CRGB ledsInner[MAX_INNER_LEDS];
CRGB ledsOuter[MAX_OUTER_LEDS];

// Create structures to hold the parameters for each ring
RingParams innerRing;
RingParams outerRing;

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  // Initialize the FastLED library for each ring
  FastLED.addLeds<NEOPIXEL, INNER_RING_PIN>(ledsInner, MAX_INNER_LEDS);
  FastLED.addLeds<NEOPIXEL, OUTER_RING_PIN>(ledsOuter, MAX_OUTER_LEDS);

  CHSV hsv = CHSV(0, 255, 255); // Get red using the hue table, saturation, and brightness
  CRGB rgb;
  hsv2rgb_rainbow(hsv, rgb);

  // Define parameters for the inner ring
  innerRing.ledsDevice = ledsInner;
  innerRing.numLEDs = MAX_INNER_LEDS;
  innerRing.revolutionTime = 500;
  innerRing.ledColor = rgb;
  innerRing.rampTime = 6000;
  innerRing.rampStep = 5;
  outerRing.rampPercent = 0;
  innerRing.rampState = NO_RAMP;

  // Define parameters for the outer ring
  outerRing.ledsDevice = ledsOuter;
  outerRing.numLEDs = MAX_OUTER_LEDS;
  outerRing.revolutionTime = 500;
  outerRing.ledColor = rgb;
  outerRing.rampTime = 6000;
  outerRing.rampStep = 5;
  outerRing.rampPercent = 0;
  outerRing.rampState = NO_RAMP;

  debugln("Setup Complete");

  delay(500);
}

void loop() {
  // Perform the ramp up, normal animation, and ramp down for each ring
  //ringRampUp(innerRing);
  animateRing(innerRing);
  //ringRampDown(innerRing);

  // ringRampUp(outerRing);
  // animateRing(outerRing);
  // ringRampDown(outerRing);
}

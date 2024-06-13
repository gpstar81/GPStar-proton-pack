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
#include "Header.h"
#include "Animations.h"

// Create structures to hold the parameters for each ring
RingParams innerRing;
RingParams outerRing;

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  // Initialize the FastLED library for each ring
  FastLED.addLeds<NEOPIXEL, PACK_LED_PIN>(pack_leds, MAX_OUTER_LEDS);
  FastLED.addLeds<NEOPIXEL, CYCLOTRON_LED_PIN>(cyclotron_leds, MAX_INNER_LEDS);

  CHSV hsv = CHSV(0, 255, 255); // Get red using the hue table, saturation, and brightness
  CRGB rgb;
  hsv2rgb_rainbow(hsv, rgb);

  // Define parameters for the inner ring
  innerRing.deviceName = CYCLOTRON_INNER;
  innerRing.ledsArray = pack_leds;
  innerRing.numLEDs = MAX_INNER_LEDS;
  innerRing.ledColor = rgb;

  // Define parameters for the outer ring
  outerRing.deviceName = CYCLOTRON_OUTER;
  outerRing.ledsArray = cyclotron_leds;
  outerRing.numLEDs = MAX_OUTER_LEDS;
  outerRing.ledColor = rgb;

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

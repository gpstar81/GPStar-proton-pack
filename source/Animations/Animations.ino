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
#include <Ramp.h>

// Local Files
#include "Header.h"
#include "Animations.h"

// Create structures to hold the parameters for each ring
RingParams innerRing;
RingParams outerRing;

unsigned int iCounter = 0;
bool bChange = false;

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  // Initialize the FastLED library for each ring
  FastLED.addLeds<NEOPIXEL, PACK_LED_PIN>(pack_leds, OUTER_CYCLOTRON_LED_MAX);
  FastLED.addLeds<NEOPIXEL, CYCLOTRON_LED_PIN>(cyclotron_leds, INNER_CYCLOTRON_CAKE_LED_MAX);

  CHSV hsv = CHSV(0, 255, 255); // Create RED using the hue table, saturation, and brightness
  CRGB rgb; // Variable for the HSV to RGB conversion.
  hsv2rgb_rainbow(hsv, rgb); // Convert to RGB

  // Define parameters for the outer ring
  outerRing.deviceName = CYCLOTRON_OUTER;
  outerRing.ledsArray = cyclotron_leds;
  outerRing.numLEDs = OUTER_CYCLOTRON_LED_MAX;
  outerRing.revolutionTime = 500;
  outerRing.ledColor = rgb;
  outerRing.rampTime = 6000;

  // Define parameters for the inner ring
  innerRing.deviceName = CYCLOTRON_INNER;
  innerRing.ledsArray = pack_leds;
  innerRing.numLEDs = INNER_CYCLOTRON_CAKE_LED_MAX;
  innerRing.revolutionTime = 500;
  innerRing.ledColor = rgb;
  innerRing.rampTime = 6000;

  debugln("Setup Complete");

  delay(500);

  ringRampUp(outerRing); // Tell the outer ring to start with a ramp-up.
}

void loop() {
  // Perform an update to animate the ring
  if(animateRing(outerRing)) {
    iCounter += 1; // Increment when a full revolution is completed
  }

  if(iCounter > 0 && iCounter % 10 == 0) {
    debugln("Change Color");

    if(bChange) {
      CHSV hsv = CHSV(180, 255, 255); // Create BLUE using the hue table, saturation, and brightness
      CRGB rgb; // Variable for the HSV to RGB conversion.
      hsv2rgb_rainbow(hsv, rgb); // Convert to RGB
      outerRing.revolutionTime = 2000;
      outerRing.spinClockwise = false;
      outerRing.ledColor = rgb;
    }
    else {
      CHSV hsv = CHSV(0, 255, 255); // Create RED using the hue table, saturation, and brightness
      CRGB rgb; // Variable for the HSV to RGB conversion.
      hsv2rgb_rainbow(hsv, rgb); // Convert to RGB
      outerRing.revolutionTime = 500;
      outerRing.spinClockwise = true;
      outerRing.ledColor = rgb;
    }

    iCounter = 0;
    bChange = !bChange;
  }
}

// Common Variables

// Define the data pins for the LED rings
#define PACK_LED_PIN 53
#define CYCLOTRON_LED_PIN 13

#define OUTER_CYCLOTRON_LED_MAX 40
#define INNER_CYCLOTRON_CAKE_LED_MAX 40

// Initialize the LED arrays for each ring
CRGB pack_leds[OUTER_CYCLOTRON_LED_MAX];
CRGB cyclotron_leds[INNER_CYCLOTRON_CAKE_LED_MAX];

/*
 * LED Devices.
 */
enum LED_DEVICES {
  NO_DEVICE,
  CYCLOTRON_OUTER,
  CYCLOTRON_INNER
};
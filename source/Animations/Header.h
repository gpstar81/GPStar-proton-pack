// Common Variables

// Define the data pins for the LED rings
#define PACK_LED_PIN 53
#define CYCLOTRON_LED_PIN 13

#define MAX_OUTER_LEDS 40
#define MAX_INNER_LEDS 40

// Initialize the LED arrays for each ring
CRGB pack_leds[MAX_INNER_LEDS];
CRGB cyclotron_leds[MAX_OUTER_LEDS];

/*
 * LED Devices.
 */
enum device {
  POWERCELL,
  CYCLOTRON_OUTER,
  CYCLOTRON_INNER,
  CYCLOTRON_CAVITY,
  CYCLOTRON_PANEL,
  VENT_LIGHT
};
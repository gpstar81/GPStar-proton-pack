#define WIRE Wire

/*
 * Barmeter 28 segment bargraph mapping: allows accessing elements sequentially (0-27)
 * If the pattern appears inverted from what is expected, flip by using the following:
 */
//#define GPSTAR_INVERT_BARGRAPH
#ifdef GPSTAR_INVERT_BARGRAPH
  const uint8_t i_bargraph[28] = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
#else
  const uint8_t i_bargraph[28] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
#endif

/*
 * Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
 * This will use the following pins for i2c serial communication:
 * SDA -> A4
 * SCL -> A5
 */
HT16K33 ht_bargraph;
const uint8_t i_bargraph_delay = 20; // Base delay (ms) for any bargraph refresh.
const uint8_t i_bargraph_elements = 28; // Maximum elements for bargraph device.
const uint8_t i_bargraph_levels = 5; // Reflects the count of POWER_LEVELS elements.
uint8_t i_bargraph_sim_max = i_bargraph_elements; // Simulated maximum for patterns.
uint8_t i_bargraph_steps = i_bargraph_elements / 2; // Steps for patterns (1/2 Max).
uint8_t i_bargraph_step = 0; // Indicates current step for certain patterns.
bool b_bargraph_present = false; // Denotes that i2c bus found the bargraph.
int i_bargraph_element = 0; // Indicates current LED element for adjustment.
millisDelay ms_bargraph; // Timer to control bargraph updates consistently.

// Bargraph Patterns and States
enum BARGRAPH_PATTERNS { BG_RAMP_UP, BG_RAMP_DOWN, BG_OUTER_INNER, BG_INNER_PULSE };
enum BARGRAPH_PATTERNS BARGRAPH_PATTERN;
enum BARGRAPH_STATES { BG_UNKNOWN, BG_OFF, BG_EMPTY, BG_FULL, BG_MID };
enum BARGRAPH_STATES BARGRAPH_STATE;

// Power levels available (ENUM values shoud equate to 0-4).
enum POWER_LEVELS { LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5 };
enum POWER_LEVELS POWER_LEVEL;

// Denotes the speed of the cyclotron (1=Normal) which increases as firing continues.
// Used by other devices to speed up animations by dividing a delay by this value.
uint8_t i_speed_multiplier = 1;
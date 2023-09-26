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
const uint8_t i_bargraph_elements = 28; // Maximum elements for bargraph.
uint8_t i_bargraph_steps = i_bargraph_elements / 2; // Steps for patterns.
bool b_bargraph_present = false; // Denotes that i2c bus found the bargraph.
int i_bargraph_element = 0; // Indicates current element for adjustment.
uint8_t i_bargraph_delay = 20; // Base delay for any bargraph refresh.

// Patterns and States
enum BARGRAPH_PATTERNS { BG_RAMP_UP, BG_RAMP_DOWN, BG_POWER_CHECK, BG_OUTER_INNER, BG_UP_DOWN };
enum BARGRAPH_PATTERNS BARGRAPH_PATTERN;
enum BARGRAPH_STATES { BG_UNKNOWN, BG_EMPTY, BG_FULL };
enum BARGRAPH_STATES BARGRAPH_STATE;

bool b_bargraph_up = false;
uint8_t i_bargraph_status = 0;
const uint8_t i_bargraph_interval = 4;
const uint8_t i_bargraph_wait = 180;
const uint8_t d_bargraph_ramp_interval = 40;
millisDelay ms_bargraph_alt;
millisDelay ms_bargraph;
millisDelay ms_bargraph_firing;
uint8_t i_bargraph_status_alt = 0;
const uint8_t d_bargraph_ramp_interval_alt = 40;
const uint8_t i_bargraph_multiplier_ramp_1984 = 3;
const uint8_t i_bargraph_multiplier_ramp_2021 = 16;
unsigned int i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;
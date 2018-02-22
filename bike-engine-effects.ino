#include "RF24.h"
#include <wavTrigger.h> // WAV trigger is used for audio effects
#include <Adafruit_NeoPixel.h> // Neopixel LED strip/ring/pixel control
#include <WiiClassy.h>  //include the WiiClassy Libary, one serial interface for two joysticks and 15 buttons
#include <Animation.h>  // Include custom animation timing library


/*
  Begin defining audio track indexes
  Correspond to .WAV files stored on SD card inserted into WAV trigger; e.g. 001_START.WAV, 002_RUN.WAV, etc.
*/

// Define experimental 'Pod Racer' sound effects
#define AUDIO_START 12

#define AUDIO_BOOST1 13
#define AUDIO_BOOST2 14

#define AUDIO_SHIFT1 15
#define AUDIO_SHIFT2 16
#define AUDIO_SHIFT3 17
#define AUDIO_SHIFT4 18
#define AUDIO_SHIFT5 19

#define AUDIO_LOOP_CHOP1 20
#define AUDIO_LOOP_CHOP2 21
#define AUDIO_LOOP_CHOP3 22
#define AUDIO_LOOP_CHOP4 23

#define AUDIO_LOOP_STEADY_JET 24
#define AUDIO_LOOP_STEADY1 25
#define AUDIO_LOOP_STEADY2 26
#define AUDIO_LOOP_STEADY3 27
#define AUDIO_LOOP_STEADY4 28

#define AUDIO_LOOP_VARIABLE1 29
#define AUDIO_LOOP_VARIABLE2 30

#define AUDIO_FLYBY1 31
#define AUDIO_FLYBY2 32
#define AUDIO_FLYBY3 33

#define AUDIO_AIRBRAKE_OFF 34
#define AUDIO_AIRBRAKE_ON 35

#define AUDIO_SPRAY_LOOP 36
#define AUDIO_SPRAY_ON 37

#define AUDIO_SLIDE_UP 38
#define AUDIO_SLIDE_DOWN 39

#define AUDIO_SWITCH1 40
#define AUDIO_SWITCH2 41

// Define manually-triggered sound effects
#define AUDIO_BATTLEHORN 42
#define AUDIO_BATTLEHORN_LEN 8348
#define AUDIO_AIRHORN1 43
#define AUDIO_AIRHORN1_LEN 1724
#define AUDIO_HORN_BIKE1 44
#define AUDIO_HORN_BIKE1_LEN 800
#define AUDIO_HORN_FOG1 45
#define AUDIO_HORN_FOG1_LEN 3500
#define AUDIO_HORN_LACUCARACHA 46
#define AUDIO_HORN_LACUCARACHA_LEN 3792
#define AUDIO_HORN_INCEPTION 47
#define AUDIO_HORN_INCEPTION_LEN 3030
#define AUDIO_HORN_REAPER 48
#define AUDIO_HORN_REAPER_LEN 4101
#define AUDIO_AIRHORN2 49
#define AUDIO_AIRHORN2_LEN 3030
#define AUDIO_VIKING1 50
#define AUDIO_VIKING1_LEN 3100
#define AUDIO_MOCKERY_BITESTHEDUST 51
#define AUDIO_MOCKERY_BITESTHEDUST_LEN 18286
#define AUDIO_RICK_WUBBA 52
#define AUDIO_RICK_WUBBA_LEN 3422
#define AUDIO_RICK_PICKLERICK 53
#define AUDIO_RICK_PICKLERICK_LEN 1907
#define AUDIO_MOCKERY_HAHA 54
#define AUDIO_MOCKERY_HAHA_LEN 1358
#define AUDIO_CHAN 55
#define AUDIO_CHAN_LEN 2000
#define AUDIO_CHANCHAN 56
#define AUDIO_CHANCHAN_LEN 2000
#define AUDIO_ROADRUNNER 57
#define AUDIO_ROADRUNNER_LEN 1202
#define AUDIO_TROLOLO 58
#define AUDIO_TROLOLO_LEN 9979
#define AUDIO_WHIP 59
#define AUDIO_WHIP_LEN 1176
#define AUDIO_HORN_1 60
#define AUDIO_HORN_1_LEN 4467

#define AUDIO_KABOOM 61
#define AUDIO_KABOOM_LEN 5064

#define AUDIO_TRUCK_HORN_1 62
#define AUDIO_TRUCK_HORN_1_LEN 2508

#define AUDIO_TRAIN_HORN_BLAST 63
#define AUDIO_TRAIN_HORN_BLAST_LEN 4764

#define AUDIO_TRAIN_HORN_TOOT 64
#define AUDIO_TRAIN_HORN_TOOT_LEN 2415

#define AUDIO_OOGA_HORN 65
#define AUDIO_OOGA_HORN_LEN 4764

#define AUDIO_CAR_HORN_1 66
#define AUDIO_CAR_HORN_1_LEN 2415

// Pin definition for RF remote control FOB receiver
#define PIN_FOB_BUTTON_A 40
#define PIN_FOB_BUTTON_B 42
#define PIN_FOB_BUTTON_C 44
#define PIN_FOB_BUTTON_D 46

// Pin definitions for potentiometers in control box
#define PIN_CONTROLLER_POT_THROTTLE 2
#define PIN_CONTROLLER_POT_HEATER 0
#define PIN_CONTROLLER_POT_PUMP 1
#define PIN_CONTROLLER_POT_TASK 3
#define PIN_BOX_POT_MODE 4

// Pin definitions for buttons in control box
#define PIN_CONTROLLER_BUTTON_PUMP 31
#define PIN_CONTROLLER_BUTTON_BOOST 35

// Pin definitions for switches in control box
#define PIN_CONTROLLER_SWITCH_THROTTLE 23
#define PIN_CONTROLLER_SWITCH_HEATER 25
#define PIN_CONTROLLER_SWITCH_PUMP 27

// Pin definitions for MOSFETs controlling 0-12v for fans, heating strip and pump
#define PIN_MOSFET_FAN1 2
#define PIN_MOSFET_FAN2 3
#define PIN_MOSFET_HEATER 4
#define PIN_MOSFET_PUMP 5



// Pin definitions for Sparkfun spectrum analyzer board
#define SPECTRUMSHIELD_PIN_STROBE 47
#define SPECTRUMSHIELD_PIN_RESET 49
#define SPECTRUMSHIELD_PIN_LEFT A6 //analog
#define SPECTRUMSHIELD_PIN_RIGHT A7 //analog

// Pin definitions for Neopixel lights
#define NEOCIRCLE_PIN 8 // Rear thruster
#define NEOSTRIP_PIN 9 // Cylinder spinner
#define NEO_EYES_LEFT 11 // Left circular LED array
#define NEO_EYES_RIGHT 10 // Right circular LED array

RF24 radio(7, 8);

// Timing events for Pod Racer sequence
const unsigned long racer_audio_startup_duration = 11935;
const unsigned long racer_audio_startup_step_0 = 832;
const unsigned long racer_audio_startup_step_1 = 2300;
const unsigned long racer_audio_startup_step_2 = 5000;
const unsigned long racer_audio_startup_step_3 = 7000;

// Timing for specific audio effects
const unsigned long effects_mock_queen_duration = 18286; // 'Another one bites the dust' sequence


// Instantiate game controller interface
WiiClassy classy = WiiClassy();

// Flags and timing for audio effects that override default behavior
byte audio_effect_active            = 0;
byte audio_effect_overrideLights    = 0;
int audio_effect_active_group       = 0;
int audio_effect_active_index       = 0;
int audio_effect_hue                = 45;
unsigned long audio_effect_duration = 0;
unsigned long audio_effect_end      = 0;

byte OVERRIDE_EFFECT[2]     = {0, 0};
byte OVERRIDE_HUE[2]        = {0, 0};
byte OVERRIDE_SPEED[2]      = {0, 0};
byte OVERRIDE_BRIGHTNESS[2] = {0, 0};

int overrideSetting_effectIndex[2]  = {0, 0};
int overrideSetting_colorHue[2]     = {0, 0};
int overrideSetting_speed[2]        = {0, 0};
int overrideSetting_brightness[2]   = {0, 0};

int xmas_color_curr[3] = {0, 0, 0};
int xmas_color_num = 3;
int xmas_hue_red = 0;
int xmas_hue_green = 115;

unsigned long transmit_last_update;
unsigned long transmit_interval = 20;

unsigned long xmas_last_changed[3] = {0, 0, 0};
unsigned long xmas_change_interval[3] = {750, 500, 350};

//
int audio_wav_map[][2] = {
  {AUDIO_HORN_FOG1, AUDIO_HORN_BIKE1},
  {AUDIO_HORN_LACUCARACHA, AUDIO_BATTLEHORN},
  {AUDIO_CHAN, AUDIO_CHANCHAN},
  {AUDIO_VIKING1, AUDIO_ROADRUNNER},
  {AUDIO_MOCKERY_HAHA, AUDIO_HORN_INCEPTION},
  {AUDIO_HORN_REAPER, AUDIO_TROLOLO},
  {AUDIO_AIRHORN1, AUDIO_AIRHORN2},
  {AUDIO_HORN_1, AUDIO_WHIP},
  {AUDIO_RICK_PICKLERICK, AUDIO_MOCKERY_BITESTHEDUST},
  {AUDIO_CAR_HORN_1, AUDIO_OOGA_HORN},
  {AUDIO_TRAIN_HORN_BLAST, AUDIO_TRAIN_HORN_TOOT},
  {AUDIO_KABOOM, AUDIO_TRUCK_HORN_1}
};

unsigned long audio_length_map[][2] = {
  {AUDIO_HORN_FOG1_LEN, AUDIO_HORN_BIKE1_LEN},
  {AUDIO_HORN_LACUCARACHA_LEN, AUDIO_BATTLEHORN_LEN},
  {AUDIO_CHAN_LEN, AUDIO_CHANCHAN_LEN},
  {AUDIO_VIKING1_LEN, AUDIO_ROADRUNNER_LEN},
  {AUDIO_MOCKERY_HAHA_LEN, AUDIO_HORN_INCEPTION_LEN},
  {AUDIO_HORN_REAPER_LEN, AUDIO_TROLOLO_LEN},
  {AUDIO_AIRHORN1_LEN, AUDIO_AIRHORN2_LEN},
  {AUDIO_HORN_1_LEN, AUDIO_WHIP_LEN},
  {AUDIO_RICK_PICKLERICK_LEN, AUDIO_MOCKERY_BITESTHEDUST_LEN},
  {AUDIO_CAR_HORN_1_LEN, AUDIO_OOGA_HORN_LEN},
  {AUDIO_TRAIN_HORN_BLAST_LEN, AUDIO_TRAIN_HORN_TOOT_LEN},
  {AUDIO_KABOOM_LEN, AUDIO_TRUCK_HORN_1_LEN}
};

int num_audio_modes = 12;
int audio_mode = 0;


/*
    Spectrum analysis
*/
#define SPECTRUM_ANALYZER_ACTIVE 1 // Whether to enable hardware and read data

unsigned long spectrum_data_read_interval = 20; // How often to read spectrum data
unsigned long spectrum_data_lastRead = 0; // Last time spectrum data was read

int left[7]; // Left channel data 7 x 0-1023
int right[7]; // Right channel data 7 x 0-1023

int MIN_AUDIO_INPUT = 85; // What is the minimum audio value to accept; eliminates noise.

// Define ranges for joystick-controlled manual behavior
#define MANUAL_SPARKLE_MIN 0
#define MANUAL_SPARKLE_MAX 60
#define MANUAL_SPEED_MIN 0
#define MANUAL_SPEED_MAX 100

// WAV Trigger output
int AUDIO_GAIN = 0; // Volume; normal range is -70 to 0, 10 is extra loud
int AUDIO_GAIN_TMP = 0;
int AUDIO_LOOP_CROSSOVER_TIME = 2000; // Time to fade between engine loops (fading is currently problematic, too many loops in racer config)
signed int AUDIO_GAIN_MIN = -70;
signed int AUDIO_GAIN_MAX = 0;

// Main behavior control, controlled by rotary potentiometer switch
uint16_t mode = 0;
uint16_t mode_tmp;


// Pod Racer mode settings
uint16_t racer_mode_index = 0;
uint16_t racer_mode_startup_index = 0;
uint16_t racer_engine_loop_index = AUDIO_LOOP_VARIABLE1;


// Prototype mapping for eye/lens behavior
int eye_map_crosshair[9] = {0, 6, 12, 18, 24, 28, 32, 36, 40};

// Serial LCD
char tempstring1[10], tempstring2[10];

// Instantiate thruster lights
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, NEOSTRIP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring = Adafruit_NeoPixel(17, NEOCIRCLE_PIN, NEO_GRB + NEO_KHZ800);

// Instantiate control box left & right circular assembly lights
Adafruit_NeoPixel eye_right = Adafruit_NeoPixel(41, NEO_EYES_RIGHT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eye_left = Adafruit_NeoPixel(41, NEO_EYES_LEFT, NEO_GRB + NEO_KHZ800);

// Instantiate WAV Trigger serial control
wavTrigger wTrig;

// Flower effect behavior, defines ranges for granular ring-based behavior mapped to overall progress
int progressFlower[][2] = {{20, 33}, {200, 700}, {600, 1000}};

/*
   Randomizer
   When enabled, randomizes front lens' effect and color modes
*/
byte randomizer_active = 0;
unsigned long randomizer_last_updated[2] = {0, 0};
unsigned long randomizer_next_update[2] = {0, 0};
unsigned long randomizer_color_last_updated[2] = {0, 0};
unsigned long randomizer_color_next_update[2] = {0, 0};
unsigned long randomizer_update_duration_min = 5000;
unsigned long randomizer_update_duration_max = 20000;

/*
   Manual tempo set
   Allows tapping a button to set a beat for light effects to follow; not fully implmented across effects
   Will likely deprecate in favor of spectrum analysis logic
   Future re-implementation would involve a switch, potentiometer and serial display output
*/

#define BEAT_SPEED_MIN 200
#define BEAT_SPEED_MAX 1000

byte beat_sample_min = 3; // How many times should button be pressed until tempo is calculated(?)
byte beat_sample_max = 5;
byte beat_sample_currNum = 0;
unsigned long beat_sample_data[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Button presses' timestamp data
unsigned long beat_sample_data_first = 0;
unsigned long beat_output = 1000; // Calculated tempo
unsigned long beat_last_sampled = 0; // When was the last button press?
unsigned long beat_timeout = 2000; // If it's been this long since last button press, reset data(?)

unsigned long current_tempo_value = 500;
unsigned long current_tempo_bpm = 100;

/*
  Spinning cylinder lights will gently fade between colors when turbine (logic) is idle.
*/
byte pulse_active = 1;
byte pulse_direction = 0;
const int pulseLow = 20;
const int pulseHigh = 235;
int pulseCurrent = 0;
unsigned long pulseLastUpdate = 0;
unsigned long pulseUpdateDelay = 100;


/*
   Manually triggered 'Breaking the Law' sequence used to call out naughty behavior
    - Triggers playing brief (> 12 sec) section of 'Breaking the Law' .WAV
    - Overrides any current behavior with alternating red, white & blue colors.. celebrating freedom
*/
byte BREAKING_THE_LAW_Active = 0;

unsigned long BREAKING_THE_LAW_StartTime = 0;
unsigned long BREAKING_THE_LAW_ElapsedTime = 0;
unsigned long BREAKING_THE_LAW_Duration = 11970;
unsigned long BREAKING_THE_LAW_Step_ElapsedTime = 0;
unsigned long BREAKING_THE_LAW_Step_Duration = 75;
unsigned long BREAKING_THE_LAW_Step_StartTime = 0;
byte BREAKING_THE_LAW_Step_Index = 0;
byte BREAKING_THE_LAW_NumSteps = 8;



int lastDiff = 0;

int trailLength = 20;
int maxSaturation = 191;
int hue, sat, val;
int tmpHue = 0;
//unsigned char red, green, blue;
unsigned char maximumBrightness = 255;

int pos;

// Original is 2; new is 9
const byte AUDIO_INDEX_RUNNING = 2;
const byte AUDIO_INDEX_AMBIENT = 10;


const int BOOST_INDEXES[4] = {4, 6, 7, 8};

const unsigned long BOOST_TIMES[4] = {1500, 6500, 4360, 4756};

int BOOST_INDEX   = 0;
//int BOOST_RUNTIME = BOOST_TIME_0;

// Debouncing
long debounce_delay = 200;

byte num_buttons = 9;
byte button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
byte button_curr_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
byte button_last_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long button_last_debounce_time[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

byte remote_num_pots = 4;
int remote_pot_state[4] = {0, 0, 0, 0};
int remote_pot_curr_state[4] = {0, 0, 0, 0};
int remote_pot_last_state[4] = {0, 0, 0, 0};
unsigned long remote_pot_last_debounce_time[4] = {0, 0, 0, 0};

byte remote_num_buttons = 4;
byte remote_button_state[4] = {0, 0, 0, 0};
byte remote_button_curr_state[4] = {0, 0, 0, 0};
byte remote_button_last_state[4] = {0, 0, 0, 0};
unsigned long remote_button_last_debounce_time[4] = {0, 0, 0, 0};

byte fob_num_buttons = 4;
byte fob_button_state[4] = {0, 0, 0, 0};
byte fob_button_curr_state[4] = {0, 0, 0, 0};
byte fob_button_last_state[4] = {0, 0, 0, 0};
unsigned long fob_button_last_debounce_time[4] = {0, 0, 0, 0};

byte remote_num_switches = 3;
byte remote_switch_state[3] = {0, 0, 0};
byte remote_switch_curr_state[3] = {0, 0, 0};
byte remote_switch_last_state[3] = {0, 0, 0};
unsigned long remote_switch_last_debounce_time[3] = {0, 0, 0};

byte num_wii_buttons = 15;
byte wii_button_state[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte wii_button_curr_state[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte wii_button_last_state[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long wii_button_last_debounce_time[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

byte joystick_left_x_raw = 0;
byte joystick_left_y_raw = 0;
byte joystick_left_x = 0;
byte joystick_left_y = 0;
byte joystick_right_x = 0;
byte joystick_right_y = 0;

byte joystick_left_x_last = 16;
byte joystick_left_y_last = 16;
byte joystick_right_x_last = 16;
byte joystick_right_y_last = 16;
unsigned long joystick_action_lastUpdate = 0;
unsigned long joystick_action_update_interval = 100; // How frequently joystick behaviors change settings;


// LCD update timer
unsigned long lcd_last_time_read = 0;
unsigned long lcd_update_interval = 350;

unsigned long controls_last_time_read = 0;
unsigned long controls_update_interval = 20;

boolean turbineRunning = false;
boolean stopAnimation = false;

int STRENGTH_BASE = 8;
int STRENGTH_MIN  = (STRENGTH_BASE * 4) - 1;
int STRENGTH_NORMAL = (STRENGTH_BASE * 8) - 1;
int STRENGTH_STRONG = (STRENGTH_BASE * 12) - 1;
int STRENGTH_HALF = (STRENGTH_BASE * 16) - 1;
int STRENGTH_FULL = (STRENGTH_BASE * 32) - 1;

int strength = 63;
uint32_t RED = strip.Color(STRENGTH_NORMAL, 0, 0);
uint32_t REDDER = strip.Color(STRENGTH_NORMAL, 0, 0);
uint32_t REDDEST = strip.Color(255, 0, 0);
uint32_t GREEN = strip.Color(0, STRENGTH_NORMAL, 0);
uint32_t BLUE = strip.Color(0, 0, STRENGTH_NORMAL);
uint32_t BLUEST = strip.Color(0, 0, 255);
uint32_t WHITE = strip.Color(STRENGTH_NORMAL, STRENGTH_NORMAL, STRENGTH_NORMAL);
uint32_t WHITEST = strip.Color(255, 255, 255);
uint32_t BLACK = strip.Color(0, 0, 0);

uint32_t strip_animation_color = strip.Color(0, 0, 0);

int TURBINE_FREQUENCY_START = 100;

/*
   Turn signal
*/
int turn_signal_active = 0;
int turn_signal_isLeft = 1;
int turn_signal_flashActive = 0;
unsigned long turn_signal_flash_duration = 600;
unsigned long turn_signal_pause_duration = 350;
unsigned long turn_signal_lastUpdated = 0;

Animation Spinner = Animation(24, 40);
Animation Thruster = Animation(17, 40);


int POWER_STATUS_UPDATE_DELAY = 200;
const int NUM_STARTUP_STATES = 7;
long TURBINE_STARTUP_BEGIN = 0;
boolean TURBINE_STARTUP_ACTIVE = false;
//boolean TURBINE_BOOST_ACTIVE = false;
boolean TURBINE_BOOST_RUNNING = false;

int TURBINE_STARTUP_INDEX = 0;
int TURBINE_DISPLAY_INDEX = 0;

unsigned long _racerRunStartTime = 0;
unsigned long _racerRunElapsedTime = 0;
unsigned long _racerRunStopTime = 0;
unsigned long _racerElapsedTimeSinceStop = 0;

unsigned long _engineRunStartTime = 0;
unsigned long _engineRunElapsedTime = 0;
unsigned long _engineRunStopTime = 0;
unsigned long _engineElapsedTimeSinceStop = 0;

int _sprayActive = 0;
unsigned long _sprayStartTime = 0;
unsigned long _sprayElapsedTime = 0;
unsigned long _sprayDuration = 0;

int boostColor = 160;
int _boostActive = 0;
unsigned long _boostStartTime = 0;
unsigned long _boostElapsedTime = 0;
unsigned long _boostDuration = 0;
unsigned long _quarterDuration = 0;


int _engineStepIndex = 0;
int _engineStopStepIndex = 0;
int _engineRunning = 0;
int _engineShuttingDown = 0;

int _enginePowerLevel = 0;


int _racerRunning = 0;


/* Main power */



/* Revised behavior */
const int THROTTLE_MIN = 0;
const int THROTTLE_MAX = 255;

const int THROTTLE_MIN_RUNNING_NORMAL = 63;
const int THROTTLE_MAX_RUNNING_NORMAL = 192;

const int PUMP_TIME_MIN = 20;
const int PUMP_TIME_MAX = 2000;

byte enableFan = 0;
byte enableHeater = 0;
byte enablePump = 0;

int throttlePowerLevel  = 0;
int fanPowerLevel       = 0; // 0-255
int heaterPowerLevel    = 0;
int pumpTimeLevel       = 0;


/*
   EFFECTS
*/
uint32_t default_color = eye_left.Color(16, 16, 16);
uint32_t _BLACK = eye_left.Color(0, 0, 0);
byte effects_active = 1;
int num_effect_modes = 12;
int effect_mode = 0;
int effect_modes[2] = {6, 6}; // default effect modes

int effect_option_useEngine = 0;
int effect_option_useThruster = 0;

/*
   Effects
    - Sparkle
    - Sparkle Fade
    - Sparkle Fade (Rings)
    - Strobe
    - Strobe (Rings)
    - Pulse
    - Pulse (Rings)
    - Spin
    - Flower
    - Flower Sparkle
    - Boom
*/



byte effect_option_sync             = 1; // 1 = Control both side together, 0 = control independently
//byte effect_option_overrideSpinner  = 0;
//byte effect_option_overrideThruster = 0;
//byte effect_option_direction[2] = {1, 1}; // Variable

/* Effects update interval tracking */
unsigned long effect_cycle_start = 0;
//unsigned long effect_cycle_lastUpdated  = 0;


const int numEffects = 9;

/*
   Rainbow
*/
int effect_color_rainbow_index[2] = {0, 0};



/*
  0 - Sparkle
*/

#define EFFECT_OPTION_SPARKLE_DURATION_MIN 1
#define EFFECT_OPTION_SPARKLE_DURATION_MAX 50

unsigned long effect_sparkle_eventDuration[2] = {20, 20}; // How long between each (sparkle) event
unsigned long effect_sparkle_event_lastUpdated[2] = {0, 0};

int effect_sparkle_populationPercent[2] = {30, 30};

/*
   1 - Sparkle Fade
*/
byte effect_sparkleFade_travel_direction[2]           = {0, 0}; // Replaces 'active' state (non-binary light states)
byte effect_sparkleFade_atZero[2]                     = {1, 1};
byte effect_sparkleFade_atFull[2]                     = {0, 0};
unsigned long effect_sparkleFade_travelDuration[2]    = {1000, 1000}; // How long for sparkleFade to complete
unsigned long effect_sparkleFade_pauseDurationAtFull[2] = {200, 200};
unsigned long effect_sparkleFade_pauseDurationAtZero[2] = {200, 200};
unsigned long effect_sparkleFade_travel_start[2]      = {0, 0};
unsigned long effect_sparkleFade_travel_end[2]        = {0, 0};
unsigned long effect_sparkleFade_pause_start[2]       = {0, 0};
unsigned long effect_sparkleFade_pause_end[2]         = {0, 0};
int effect_sparkleFade_color_val[2]                   = {0, 0};


/*
   2 - Sparkle Fade Ring
*/
byte effect_sparkleFadeRing_travel_direction[][3]           = {{0, 0, 0}, {0, 0, 0}}; // Replaces 'active' state (non-binary light states)
byte effect_sparkleFadeRing_atZero[][3]                     = {{1, 1, 1}, {1, 1, 1}};
byte effect_sparkleFadeRing_atFull[][3]                     = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_sparkleFadeRing_travelDuration[][3]    = {{1000, 1000, 1000}, {1000, 1000, 1000}}; // How long for sparkleFadeRing to complete
unsigned long effect_sparkleFadeRing_pauseDurationAtFull[][3] = {{50, 150, 250}, {50, 150, 250}};
unsigned long effect_sparkleFadeRing_pauseDurationAtZero[][3] = {{50, 150, 250}, {50, 150, 250}};
unsigned long effect_sparkleFadeRing_travel_start[][3]      = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_sparkleFadeRing_travel_end[][3]        = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_sparkleFadeRing_pause_start[][3]       = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_sparkleFadeRing_pause_end[][3]         = {{0, 0, 0}, {0, 0, 0}};
int effect_sparkleFadeRing_color_val[][3]                   = {{0, 0, 0}, {0, 0, 0}};


/*
   3 - Strobe
*/
#define EFFECT_OPTION_STROBE_PAUSE_DURATION_MIN 30
#define EFFECT_OPTION_STROBE_PAUSE_DURATION_MAX 500
#define EFFECT_OPTION_STROBE_FLASH_DURATION_MIN 10
#define EFFECT_OPTION_STROBE_FLASH_DURATION_MAX 20

byte effect_strobe_flash_active[2]           = {0, 0};
unsigned long effect_strobe_flashDuration[2] = {10, 10}; // Long long strobe light is on
unsigned long effect_strobe_flashPause[2]    = {100, 100};
unsigned long effect_strobe_flash_start[2]   = {0, 0};
unsigned long effect_strobe_flash_end[2]     = {0, 0};
uint32_t effect_strobe_flash_color[2]        = {default_color, default_color};

/*
  4 - Strobe (Rings)
*/
byte effect_strobeRing_flash_active[][3] = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_strobeRing_flashDuration[][3] = {{10, 10, 10}, {10, 10, 10}};
unsigned long effect_strobeRing_flashPause[][3] = {{100, 100, 100}, {100, 100, 100}};
unsigned long effect_strobeRing_flash_start[][3] = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_strobeRing_flash_end[][3] = {{0, 0, 0}, {0, 0, 0}};
uint32_t effect_strobeRing_flash_color[][3] = {{default_color, default_color, default_color}, {default_color, default_color, default_color}};


/*
  5 - Pulse
*/
#define EFFECT_OPTION_PULSE_DURATION_MIN 100
#define EFFECT_OPTION_PULSE_DURATION_MAX 2000
byte effect_pulse_travel_direction[2]           = {0, 0}; // Replaces 'active' state (non-binary light states)
byte effect_pulse_atZero[2]                     = {1, 1};
byte effect_pulse_atFull[2]                     = {0, 0};
unsigned long effect_pulse_travelDuration[2]    = {1000, 1000}; // How long for pulse to complete
unsigned long effect_pulse_pauseDurationAtFull[2] = {2000, 500};
unsigned long effect_pulse_pauseDurationAtZero[2] = {500, 500};
unsigned long effect_pulse_travel_start[2]      = {0, 0};
unsigned long effect_pulse_travel_end[2]        = {0, 0};
unsigned long effect_pulse_pause_start[2]       = {0, 0};
unsigned long effect_pulse_pause_end[2]         = {0, 0};
int effect_pulse_color_val[2]                   = {0, 0};


/*
  6 - Pulse (Rings)
*/
byte effect_pulseRing_travel_direction[][3]           = {{0, 1, 0}, {0, 1, 0}}; // Replaces 'active' state (non-binary light states)
byte effect_pulseRing_atZero[][3]                     = {{1, 1, 1}, {1, 1, 1}};
byte effect_pulseRing_atFull[][3]                     = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_pulseRing_travelDuration[][3]    = {{1000, 1000, 1000}, {1000, 1000, 1000}}; // How long for pulse to complete
unsigned long effect_pulseRing_pauseDurationAtFull[][3] = {{50, 150, 250}, {50, 150, 250}};
unsigned long effect_pulseRing_pauseDurationAtZero[][3] = {{50, 150, 250}, {50, 150, 250}};
unsigned long effect_pulseRing_travel_start[][3]      = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_pulseRing_travel_end[][3]        = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_pulseRing_pause_start[][3]       = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_pulseRing_pause_end[][3]         = {{0, 0, 0}, {0, 0, 0}};
int effect_pulseRing_color_val[][3]                   = {{0, 0, 0}, {0, 0, 0}};


/*
  7 - Spin
*/
#define EFFECT_OPTION_SPIN_DURATION_MIN 350
#define EFFECT_OPTION_SPIN_DURATION_MAX 4000

byte effect_spin_travel_direction[][3]           = {{0, 0, 0}, {1, 1, 1}};
byte effect_spin_numLights[][3]                  = {{1, 16, 24}, {1, 16, 24}};
unsigned long effect_spin_travelDuration[][3]    = {{0, 1500, 1500}, {0, 1500, 1500}};
unsigned long effect_spin_travelStopTime[][3]    = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_spin_travel_start[][3]      = {{0, 0, 0}, {0, 0, 0}};
unsigned long effect_spin_update_interval        = 100;
unsigned long effect_spin_lastUpdated[2]         = {0, 0};
int effect_spin_color_val[][3]                   = {{0, 0, 0}, {0, 0, 0}};
int effect_spin_stepIndex[][3]                   = {{0, 0, 0}, {0, 0, 0}};

/*
  8 - Flower & 9 - Flower Sparkle
*/
#define EFFECT_OPTION_BLOOM_DURATION_MIN 200
#define EFFECT_OPTION_BLOOM_DURATION_MAX 3000

byte effect_flower_transition_type[2]             = {0, 0}; // 0 = fade, 1 = sparkle

byte effect_flower_travel_direction[2]            = {0, 0};
byte effect_flower_atZero[2]                      = {1, 1};
byte effect_flower_atFull[2]                      = {0, 0};
unsigned long effect_flower_travelDuration[2]     = {100, 100}; // How long for pulse to complete
unsigned long effect_flower_pauseDurationAtFull[2]  = {0, 0};
unsigned long effect_flower_pauseDurationAtZero[2]  = {200, 200};
unsigned long effect_flower_travel_start[2]       = {0, 0};
unsigned long effect_flower_travel_end[2]         = {0, 0};
unsigned long effect_flower_pause_start[2]        = {0, 0};
unsigned long effect_flower_pause_end[2]          = {0, 0};
int effect_flower_color_hue[][3]                  = {{0, 25, 50}, {0, 25, 50}};
int effect_flower_color_val[][3]                  = {{0, 0, 0}, {0, 0, 0}};
int effect_flower_color_hue_petal_diff            = 20;


/*
  10 - Flower Boom
*/
byte effect_boom_transition_type[2]             = {0, 0}; // 0 = fade, 1 = sparkle

byte effect_boom_travel_direction[]             = {0, 0};
byte effect_boom_atZero[2]                      = {1, 1};
byte effect_boom_atFull[2]                      = {0, 0};
unsigned long effect_boom_travelDuration[2]     = {100, 100}; // How long for pulse to complete
unsigned long effect_boom_pauseDurationAtFull[2]  = {0, 0};
unsigned long effect_boom_pauseDurationAtZero[2]  = {200, 200};
unsigned long effect_boom_travel_start[2]       = {0, 0};
unsigned long effect_boom_travel_end[2]         = {0, 0};
unsigned long effect_boom_pause_start[2]        = {0, 0};
unsigned long effect_boom_pause_end[2]          = {0, 0};
int effect_boom_color_hue[][3]                  = {{0, 25, 50}, {0, 25, 50}};
int effect_boom_color_val[][3]                  = {{0, 0, 0}, {0, 0, 0}};
int effect_boom_color_hue_petal_diff            = 20;


/*
  11 - Wipe
*/
byte effect_wipe_transition_type[2]             = {0, 0}; // 0 = fade, 1 = sparkle

byte effect_wipe_travel_direction[]             = {0, 0};
byte effect_wipe_atZero[2]                      = {1, 1};
byte effect_wipe_atFull[2]                      = {0, 0};
unsigned long effect_wipe_travelDuration[2]     = {100, 100}; // How long for pulse to complete
unsigned long effect_wipe_pauseDurationAtFull[2]  = {0, 0};
unsigned long effect_wipe_pauseDurationAtZero[2]  = {200, 200};
unsigned long effect_wipe_travel_start[2]       = {0, 0};
unsigned long effect_wipe_travel_end[2]         = {0, 0};
unsigned long effect_wipe_pause_start[2]        = {0, 0};
unsigned long effect_wipe_pause_end[2]          = {0, 0};
int effect_wipe_color_hue[][3]                  = {{0, 25, 50}, {0, 25, 50}};
int effect_wipe_color_val[][3]                  = {{0, 0, 0}, {0, 0, 0}};
int effect_wipe_color_hue_petal_diff            = 20;

#define WIPE_MAX_NUM_LIGHTS 8
#define WIPE_NUM_STEPS 8
int blankMap[WIPE_MAX_NUM_LIGHTS] = {0, 0, 0, 0, 0, 0, 0, 0};
byte wipeMapLength[2] = {0, 0};
int wipeMapData[][WIPE_MAX_NUM_LIGHTS] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
int wipeMapCurrent[][WIPE_MAX_NUM_LIGHTS] = {{0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}};

int updateWipeMap(int isLeft, int travelProgress) {

}


/*
   Effect colors
    0 - Manual color
    1 - Color cycle
    2 - White
    3 - Random
*/

int random_color_hues[14] = {0, 15, 31, 47, 63, 95, 127, 155, 172, 197, 212, 225, 245, 255};
int random_hue = 0;
int random_hue_index = 0;
int random_val_index = 0;

int random_hues[2] = {0, 0};
int random_hue_indexes[2] = {0, 0};
int random_val_indexes[2] = {0, 0};

// prevent hue from changing too rapidly, colors mix and get oversaturated
unsigned long random_color_update_last = 0;
unsigned long random_color_update_delay = 200;
int num_color_modes = 5;
//int effect_color_mode = 1;
int effect_color_modes[2] = {4, 4};
int color_option_sync = 1;
int effect_color_cycle_step_duration = 25; // How quickly to update color cycling
unsigned long effect_color_cycle_last_update   = 0;
int effect_manual_hue_current = 127;
int effect_color_cycle_hue_current = 0;
int effect_color_cycle_sat_current = 255;
int effect_color_cycle_val_current = 127;
int effect_color_cycle_maxBrightness = 127;

int effect_manual_chanceSparkle[2] = {30, 30};
int effect_manual_speed[2] = {50, 50};

unsigned long random_color_update_lasts[2] = {0, 0};
unsigned long random_color_update_delays[2] = {random_color_update_delay, random_color_update_delay};
int effect_color_cycle_step_durations[2] = { // How quickly to update color cycling
  effect_color_cycle_step_duration,
  effect_color_cycle_step_duration
};
unsigned long effect_color_cycle_last_updates[2]   = {0, 0};
int effect_color_cycle_hue_currents[2] = {0, 0};
int effect_color_cycle_sat_currents[2] = {255, 255};
int effect_color_cycle_val_currents[2] = {127, 127};
//int effect_color_cycle_maxBrightnesses[2] = {127, 127};

// Addresses for radio transmission & reception
byte addresses[][6] = {"1Mast", "2Left", "3Rght"};

// Data structure to send
struct transmitDataStructure {
  byte idx = 0;
  byte pos = 0;
  byte mode = 4;
  byte behavior = 0; // Which set of behaviors to follow

  byte right[7];  // 7 Channels of 0-255
  byte left[7];   // 7 Channels of 0-255

  byte red   = 0;
  byte green = 0;
  byte blue  = 0;

} lightData;


void setup() {

  radio.begin();
  radio.setChannel(108);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(10, 15);
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_16);

  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  radio.startListening();

  delay(100);

  classy.init();  //start classy library
  delay(100);
  classy.update();  //read data from the classic controller
  delay(100);

  Serial.begin(9600);

  // Serial LCD
  Serial2.begin(9600); // set up serial port for 9600 baud
  delay(500); // wait for display to boot up
  lcdBacklight(157);
  delay(200);
  //  lcdSetMessage(0);

  if (SPECTRUM_ANALYZER_ACTIVE) {
    //initialize eq
    pinMode(SPECTRUMSHIELD_PIN_RESET, OUTPUT); // reset
    pinMode(SPECTRUMSHIELD_PIN_STROBE, OUTPUT); // strobe
    digitalWrite(SPECTRUMSHIELD_PIN_RESET, LOW); // reset low
    digitalWrite(SPECTRUMSHIELD_PIN_STROBE, HIGH); //pin 5 is RESET on the shield
  }

  pinMode(PIN_CONTROLLER_SWITCH_THROTTLE, INPUT);
  digitalWrite(PIN_CONTROLLER_SWITCH_THROTTLE, LOW);

  pinMode(PIN_CONTROLLER_SWITCH_HEATER, INPUT);
  digitalWrite(PIN_CONTROLLER_SWITCH_HEATER, LOW);

  pinMode(PIN_CONTROLLER_SWITCH_PUMP, INPUT);
  digitalWrite(PIN_CONTROLLER_SWITCH_PUMP, LOW);

  pinMode(PIN_CONTROLLER_BUTTON_PUMP, INPUT);
  digitalWrite(PIN_CONTROLLER_BUTTON_PUMP, HIGH);

  pinMode(PIN_CONTROLLER_BUTTON_BOOST, INPUT);
  digitalWrite(PIN_CONTROLLER_BUTTON_BOOST, HIGH);

  pinMode(PIN_FOB_BUTTON_A, INPUT);
  digitalWrite(PIN_FOB_BUTTON_A, LOW);

  pinMode(PIN_FOB_BUTTON_B, INPUT);
  digitalWrite(PIN_FOB_BUTTON_B, LOW);

  pinMode(PIN_FOB_BUTTON_C, INPUT);
  digitalWrite(PIN_FOB_BUTTON_C, LOW);

  pinMode(PIN_FOB_BUTTON_D, INPUT);
  digitalWrite(PIN_FOB_BUTTON_D, LOW);

  pinMode(PIN_MOSFET_FAN1, OUTPUT);
  pinMode(PIN_MOSFET_FAN2, OUTPUT);

  delay(10);

  wTrig.start();
  delay(500);



  strip.begin();
  strip.show();

  ring.begin();
  ring.show();

  eye_right.begin();
  eye_right.show();

  eye_left.begin();
  eye_left.show();


  delay(10);

  Serial.begin(9600);

  //  wTrig.stopAllTracks();

  delay(10);

  fanOff();
  setHeatLevel(0);

  delay(5);

  if (false) {
    wTrig.trackLoop(AUDIO_LOOP_CHOP1, 1);
    wTrig.trackLoop(AUDIO_LOOP_CHOP2, 1);
    wTrig.trackLoop(AUDIO_LOOP_CHOP3, 1);
    wTrig.trackLoop(AUDIO_LOOP_CHOP4, 1);

    wTrig.trackLoop(AUDIO_LOOP_STEADY_JET, 1);
    wTrig.trackLoop(AUDIO_LOOP_STEADY1, 1);
    wTrig.trackLoop(AUDIO_LOOP_STEADY2, 1);
    wTrig.trackLoop(AUDIO_LOOP_STEADY3, 1);
    wTrig.trackLoop(AUDIO_LOOP_STEADY4, 1);

    wTrig.trackLoop(AUDIO_LOOP_VARIABLE1, 1);
    wTrig.trackLoop(AUDIO_LOOP_VARIABLE2, 1);
  }

  delay(100);



  initControllerData();

  initModeBehavior(mode);

  lcdShowSettings();


  clearBothLenses();

  //  Effects[0].setOption_population(1, 30);
}

void(* resetFunc) (void) = 0;//declare reset function at address 0

void loop() {
  readControllerData();
  updateAudioEffect();


  if (effects_active || audio_effect_overrideLights) {
    updateEffects();
    //    updateEffectsNew();
    if (!audio_effect_overrideLights) {
      updateRandomizer();
    }
  }
  updateSpray(); // Check to ensure shutoff happens even if 'special' sequence is running

  // Special sequences
  if (BREAKING_THE_LAW_Active) {

    updateBREAKINGTHELAW();

  } else { // Default non-sequence behavior

    switch (mode) {
      case 0: // Disabled
        break;

      case 1: // Turbine (default)
      case 4: // xmas
        runEngine();

        if (_engineShuttingDown) {
          runEngineShutdown();
        }

        if (Spinner.isActive() && Spinner.execute()) {
          updateSpinner();
        }

        if (Thruster.isActive() && Thruster.execute()) {
          flickerThrustLight(Thruster.getStrength());
        }

        if (pulse_active) {
          if (inXmasMode()) {
            updateXmasPole();
          } else {
            updatePulse();
          }
        }

        updateBoost();

        break;

      case 2: // Pod Racer
        updateRacer();

        break;

      case 3: // Horse Power

        break;

      case 9: // Commuter
        updateTurnSignal();
        break;
    } //end switch(mode)

  } // end default non-sequence behavior

  //  transmitData();
//  if (receiveStripData()) {
//    Serial.println("Got data");
//  }
}

void transmitData() {
  if (millis() >= (transmit_last_update + transmit_interval)) {
    transmit_last_update = millis();
    radio.stopListening();
    radio.openWritingPipe(addresses[1]);
//    radio.write(&lightData, sizeof(lightData));
    radio.startListening();
    //    prettyPrintSoundData(left);
  }
}

bool receiveStripData() {
  if (radio.available()) {
    while (radio.available()) {
      radio.read(&lightData, sizeof(lightData));
    }
    return 1;
  } else {
    return 0;
  }
}
void prettyPrintSoundData(int sound[8]) {
  for (int i = 0; i < 8; i++) {
    if (i == 7) {
      Serial.println(sound[i]);
    } else {
      Serial.print(sound[i]); Serial.print(',');
    }
  }
}

int inCommuterMode() {
  return mode == 9 ? 1 : 0;
}

int inXmasMode() {
  return mode == 4 ? 1 : 0;
}

void setCommuterLights() {
  //  Serial.println("setCommuterLights");
  lensFillAll(1, eye_left.Color(effect_color_cycle_maxBrightness, effect_color_cycle_maxBrightness, effect_color_cycle_maxBrightness));
  lensFillAll(0, eye_left.Color(effect_color_cycle_maxBrightness, effect_color_cycle_maxBrightness, effect_color_cycle_maxBrightness));

  //  stripColorSet(strip.Color(effect_color_cycle_maxBrightness, 0, 0));
  ringColorSet(ring.Color(effect_color_cycle_maxBrightness, 0, 0));
}




void hsv2rgb(unsigned int hue, unsigned int sat, unsigned int val, \
             unsigned char * r, unsigned char * g, unsigned char * b, unsigned char maxBrightness ) {
  unsigned int H_accent = hue / 60;
  unsigned int bottom = ((255 - sat) * val) >> 8;
  unsigned int top = val;
  unsigned char rising  = ((top - bottom)  * (hue % 60   )  )  /  60  +  bottom;
  unsigned char falling = ((top - bottom)  * (60 - hue % 60)  )  /  60  +  bottom;

  switch (H_accent) {
    case 0:
      *r = top;
      *g = rising;
      *b = bottom;
      break;

    case 1:
      *r = falling;
      *g = top;
      *b = bottom;
      break;

    case 2:
      *r = bottom;
      *g = top;
      *b = rising;
      break;

    case 3:
      *r = bottom;
      *g = falling;
      *b = top;
      break;

    case 4:
      *r = rising;
      *g = bottom;
      *b = top;
      break;

    case 5:
      *r = top;
      *g = bottom;
      *b = falling;
      break;
  }
  // Scale values to maxBrightness
  *r = *r * maxBrightness / 255;
  *g = *g * maxBrightness / 255;
  *b = *b * maxBrightness / 255;
}



void fanOn() {
  enableFan = 1;
}

void fanOff() {
  enableFan = 0;
}

void stripChase(int currStep, long freq, uint32_t color) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, currStep == i ? color : BLACK);
  }
  strip.show();
}


/*******************

    Eyeballs/Lens

*********************/
void eyesFillCenter(uint32_t color, byte isLeft) {
  if (isLeft) {
    eye_left.setPixelColor(eye_left.numPixels() - 1, color);
    eye_left.show();
  } else {
    eye_right.setPixelColor(eye_right.numPixels() - 1, color);
    eye_right.show();
  }
}

void eyesFillInner(uint32_t color, byte isLeft) {
  if (isLeft) {
    for (int currIndex = 24; currIndex < eye_left.numPixels() - 1; currIndex++) {
      eye_left.setPixelColor(currIndex, color);
    }

    eye_left.show();
  } else {
    for (int currIndex = 24; currIndex < eye_right.numPixels() - 1; currIndex++) {
      eye_right.setPixelColor(currIndex, color);
    }
    eye_right.show();
  }
}

void eyesFillOuter(uint32_t color, byte isLeft) {
  if (isLeft) {
    for (int currIndex = 0; currIndex < eye_left.numPixels() - 17; currIndex++) {
      eye_left.setPixelColor(currIndex, color);
    }
    eye_left.show();
  } else {
    for (int currIndex = 0; currIndex < eye_right.numPixels() - 17; currIndex++) {
      eye_right.setPixelColor(currIndex, color);
    }
    eye_right.show();
  }
}

void eyeSetInnerByIndex(byte isLeft, byte ringIdx, uint32_t color) {
  if (isLeft) {
    eye_left.setPixelColor(ringIdx + 24, color);
    eye_left.show();
  } else {
    eye_right.setPixelColor(ringIdx + 24, color);
    eye_right.show();
  }
}

void eyeSetOuterByIndex(byte isLeft, byte ringIdx, uint32_t color) {
  if (isLeft) {
    eye_left.setPixelColor(ringIdx, color);
    eye_left.show();
  } else {
    eye_right.setPixelColor(ringIdx, color);
    eye_right.show();
  }
}

void clearBothLenses() {
  lensClear(0);
  lensClear(1);
}

void lensClear(byte isLeft) {
  lensFillAll(isLeft, _BLACK);
}

void lensFillAll(byte isLeft, uint32_t color) {
  if (isLeft) {
    for (int currIndex = 0; currIndex < eye_left.numPixels(); currIndex++) {
      eye_left.setPixelColor(currIndex, color);
    }
    eye_left.show();
  } else {
    for (int currIndex = 0; currIndex < eye_right.numPixels(); currIndex++) {
      eye_right.setPixelColor(currIndex, color);
    }
    eye_right.show();
  }
}

void lensFillIdx(byte isLeft, byte idx, uint32_t color) {
  switch (idx) {
    case 0:
      eyesFillCenter(color, isLeft);
      break;
    case 1:
      eyesFillInner(color, isLeft);
      break;
    case 2:
      eyesFillOuter(color, isLeft);
      break;
  }
}


void eyeSetCrosshair(uint32_t color, byte isLeft) {
  uint32_t tmpColor;
  for (int currIndex = 0; currIndex < eye_left.numPixels(); currIndex++) {
    tmpColor = inTargetCrosshair(currIndex) ? color : _BLACK;
    if (isLeft) {
      eye_left.setPixelColor(currIndex, tmpColor);
    } else {
      eye_right.setPixelColor(currIndex, tmpColor);
    }
  }

  if (isLeft) {
    eye_left.show();
  } else {
    eye_right.show();
  }
}

boolean inTargetCrosshair(int pixelIndex) {
  for (int currIndex = 0; currIndex < 9; currIndex++) {
    if (pixelIndex == eye_map_crosshair[currIndex]) {
      return true;
    }
  }
  return false;
}

/*
  byte randomizer_active = 0;
  unsigned long randomizer_last_updated = 0;
  unsigned long randomizer_next_update = 0;
  unsigned long randomizer_update_duration_min = 3000;
  unsigned long randomizer_update_duration_max = 10000;
*/
void toggleRandomizer() {
  setRandomizerState((randomizer_active == 1) ? 0 : 1, 1);
}

void setRandomizerState(int isActive, int normalize) {
  randomizer_active = isActive;
  if (randomizer_active) {
    randomizeSettings(1);
    randomizeSettings(0);
    randomizeColorSettings(1);
    randomizeColorSettings(0);
  } else if (normalize == 1) {
    normalizeSettings();
  }
  lcdShowRandom();
}

void updateRandomizer() {
  if (randomizer_active) {
    if (millis() >= randomizer_next_update[1]) {
      randomizeSettings(1);
    }
    if (millis() >= randomizer_next_update[0]) {
      randomizeSettings(0);
    }
    if (millis() >= randomizer_color_next_update[1]) {
      randomizeColorSettings(1);
    }
    if (millis() >= randomizer_color_next_update[0]) {
      randomizeColorSettings(0);
    }
  }
}

void toggleEffectSync() {
  setEffectSync(effect_option_sync == 1 ? 0 : 1);
}

void setEffectSync(int isSynced) {
  effect_option_sync = isSynced;
  color_option_sync = isSynced;
  lcdShowSync();
}

int getRandomColorMode() {
  if (random(1, 100) <= 10) {
    return num_color_modes - 1; // White is least likely
  } else {
    return random(0, num_color_modes - 2);
  }
}

void randomizeAllSettings() {
  setEffectSync(0);
  randomizeEffectSpeed();
  randomizeSettings(1);
  randomizeSettings(0);
  randomizeColorSettings(1);
  randomizeColorSettings(0);
}

void randomizeSettings(int isLeft) {
  randomizer_next_update[isLeft] = (millis() + random(randomizer_update_duration_min, randomizer_update_duration_max));

  // How likely is it that both sides match
  int chanceOfSyncDifference = 50;

  // Sync/unsync sides.. figure out how to do this, harder now with isLeft in this method
  setEffectSync((random(1, 100) <= chanceOfSyncDifference) ? 1 : 0);

  setNewEffectMode(isLeft, random(0, num_effect_modes - 1));



  lcdShowEffectMode(0);
  lcdShowEffectMode(1);



  randomizeEffectSettings();

}



void randomizeColorSettings(int isLeft) {
  randomizer_color_next_update[isLeft] = (millis() + random(randomizer_update_duration_min, randomizer_update_duration_max));
  effect_color_modes[isLeft] = getRandomColorMode();
  effect_color_cycle_step_durations[isLeft] = random(5, 150);
  lcdShowColorMode(isLeft);
}

void randomizeEffectSettings() {
  setEffectOptions_sparkle(
    getRandomFlashDuration(),
    getRandomFlashDuration(),
    random(2, 50),
    random(2, 50)
  );

  setEffectOptions_strobe(getRandomFlashDuration(), getRandomFlashDuration(), getRandomFlashPause(), getRandomFlashPause());
  setEffectOptions_strobeRing(
    getRandomFlashDuration(), getRandomFlashDuration(), getRandomFlashDuration(),
    getRandomFlashDuration(), getRandomFlashDuration(), getRandomFlashDuration(),
    getRandomFlashPause(), getRandomFlashPause(), getRandomFlashPause(),
    getRandomFlashPause(), getRandomFlashPause(), getRandomFlashPause()
  );

  setEffectOptions_pulse(random(255, 2000), random(255, 2000), random(25, 350), random(25, 350), random(25, 350), random(25, 350));

  setEffectOptions_pulseRing_travelDurations(
    random(200, 2000), random(200, 2000), random(200, 2000),
    random(200, 2000), random(200, 2000), random(200, 2000)
  );
  setEffectOptions_pulseRing_pauseDurationAtFull(
    random(0, 50), random(0, 50), random(0, 50),
    random(0, 50), random(0, 50), random(0, 50)
  );
  setEffectOptions_pulseRing_pauseDurationAtZero(
    random(0, 50), random(0, 50), random(0, 50),
    random(0, 50), random(0, 50), random(0, 50)
  );

  // TODO: setEffectOptions for spin & flower
}

void normalizeSettings() {
  setEffectSync(1);
  effect_modes[1] = effect_modes[0];
  effect_color_cycle_step_durations[1] = effect_color_cycle_step_duration;
  effect_color_cycle_step_durations[0] = effect_color_cycle_step_duration;
  setEffectOptions_sparkle(20, 20, 30, 30);
  setEffectOptions_strobe(10, 10, 100, 100);
  setEffectOptions_strobeRing(10, 10, 10, 10, 10, 10, 100, 100, 100, 100, 100, 100);
  setEffectOptions_pulse(1000, 1000, 50, 50, 50, 50);
  setEffectOptions_pulseRing_travelDurations(1000, 1000, 1000, 1000, 1000, 1000);
  setEffectOptions_pulseRing_pauseDurationAtFull(50, 150, 250, 50, 150, 250);
  setEffectOptions_pulseRing_pauseDurationAtZero(50, 150, 250, 50, 150, 250);
}

int getRandomFlashPause() {
  return random(50, 200);
}

int getRandomFlashDuration() {
  return random(5, 50);
}

/*
   Implement master tempo logic

   Correct logic will be complex

*/


void updateEffectsWithTempo() {

  unsigned long tempo_double = current_tempo_value * 2;
  unsigned long tempo_half = current_tempo_value / 2;

  effect_sparkleFade_travelDuration[1] = tempo_double;
  effect_sparkleFade_travelDuration[0] = tempo_double;
  effect_sparkleFade_pauseDurationAtFull[1] = 0;
  effect_sparkleFade_pauseDurationAtFull[0] = 0;
  effect_sparkleFade_pauseDurationAtZero[1] = 0;
  effect_sparkleFade_pauseDurationAtZero[0] = 0;

  effect_sparkleFadeRing_travelDuration[0][0] = tempo_half;
  effect_sparkleFadeRing_travelDuration[0][1] = current_tempo_value;
  effect_sparkleFadeRing_travelDuration[0][2] = tempo_double;

  effect_sparkleFadeRing_travelDuration[1][0] = tempo_half;
  effect_sparkleFadeRing_travelDuration[1][1] = current_tempo_value;
  effect_sparkleFadeRing_travelDuration[1][2] = tempo_double;

  effect_sparkleFadeRing_pauseDurationAtFull[0][0] = 0;
  effect_sparkleFadeRing_pauseDurationAtFull[0][1] = 0;
  effect_sparkleFadeRing_pauseDurationAtFull[0][2] = 0;

  effect_sparkleFadeRing_pauseDurationAtFull[1][0] = 0;
  effect_sparkleFadeRing_pauseDurationAtFull[1][1] = 0;
  effect_sparkleFadeRing_pauseDurationAtFull[1][2] = 0;

  effect_sparkleFadeRing_pauseDurationAtZero[0][0] = 0;
  effect_sparkleFadeRing_pauseDurationAtZero[0][1] = 0;
  effect_sparkleFadeRing_pauseDurationAtZero[0][2] = 0;

  effect_sparkleFadeRing_pauseDurationAtZero[1][0] = 0;
  effect_sparkleFadeRing_pauseDurationAtZero[1][1] = 0;
  effect_sparkleFadeRing_pauseDurationAtZero[1][2] = 0;





  effect_pulseRing_travelDuration[0][0] = tempo_half;
  effect_pulseRing_travelDuration[0][1] = current_tempo_value;
  effect_pulseRing_travelDuration[0][2] = tempo_double;

  effect_pulseRing_travelDuration[1][0] = tempo_half;
  effect_pulseRing_travelDuration[1][1] = current_tempo_value;
  effect_pulseRing_travelDuration[1][2] = tempo_double;

  effect_pulseRing_pauseDurationAtFull[0][0] = 0;
  effect_pulseRing_pauseDurationAtFull[0][1] = 0;
  effect_pulseRing_pauseDurationAtFull[0][2] = 0;

  effect_pulseRing_pauseDurationAtFull[1][0] = 0;
  effect_pulseRing_pauseDurationAtFull[1][1] = 0;
  effect_pulseRing_pauseDurationAtFull[1][2] = 0;

  effect_pulseRing_pauseDurationAtZero[0][0] = 0;
  effect_pulseRing_pauseDurationAtZero[0][1] = 0;
  effect_pulseRing_pauseDurationAtZero[0][2] = 0;

  effect_pulseRing_pauseDurationAtZero[1][0] = 0;
  effect_pulseRing_pauseDurationAtZero[1][1] = 0;
  effect_pulseRing_pauseDurationAtZero[1][2] = 0;






  setEffectOptions_pulseRing_travelDurations(
    tempo_half, current_tempo_value, tempo_double,
    tempo_half, current_tempo_value, tempo_double
  );

  setEffectOptions_pulseRing_pauseDurationAtFull(0, 0, 0, 0, 0, 0);
  setEffectOptions_pulseRing_pauseDurationAtZero(0, 0, 0, 0, 0, 0);


  //  effect_color_cycle_step_duration = 25;
  //  setEffectOptions_sparkle(20,20, 30, 30);
  //  setEffectOptions_strobe(10, 10, 100, 100);
  //  setEffectOptions_strobeRing(10, 10, 10, 10, 10, 10, 100, 100, 100, 100, 100, 100);
  //  setEffectOptions_pulse(1000, 1000, 50, 50, 50, 50);



  switch (effect_modes[1]) {
    case 0:
      // Sparkle
      break;
    case 1:
      // Sparkle fade
      setSparkleFadeToZero(1);
      setSparkleFadeToZero(0);
      break;
    case 2:
      // Sparkle fade ring
      setSparkleFadeRingToZero(1, 0);
      setSparkleFadeRingToZero(1, 1);
      setSparkleFadeRingToZero(1, 2);
      setSparkleFadeRingToZero(0, 0);
      setSparkleFadeRingToZero(0, 1);
      setSparkleFadeRingToZero(0, 2);
      break;
    case 3:
      // Strobe
      break;
    case 4:
      // Strobe ring
      break;
    case 5:
      // Pulse
      setPulseToZero(1);
      setPulseToZero(0);
      break;
    case 6:
      // Pulse ring
      setPulseRingToZero(1, 0);
      setPulseRingToZero(1, 1);
      setPulseRingToZero(1, 2);
      setPulseRingToZero(0, 0);
      setPulseRingToZero(0, 1);
      setPulseRingToZero(0, 2);
      break;
    case 7:
      // Spin
      break;
    case 8: case 9:
      // Flower
      break;
    case 10: // Boom
      break;
    case 11: case 12: // Audio
      break;
  }


}


void setEffectOptions_sparkle(unsigned long duration1, unsigned long duration2, int populate1, int populate2) {
  effect_sparkle_eventDuration[1] = duration1;
  effect_sparkle_eventDuration[0] = effect_option_sync ? duration1 : duration2;;
  effect_sparkle_populationPercent[1] = populate1;
  effect_sparkle_populationPercent[0] = effect_option_sync ? populate1 : populate2;
}

void setEffectOptions_sparkleFade(unsigned long duration1, unsigned long duration2, int populate1, int populate2) {
  effect_sparkle_eventDuration[1] = duration1;
  effect_sparkle_eventDuration[0] = effect_option_sync ? duration1 : duration2;;
  effect_sparkle_populationPercent[1] = populate1;
  effect_sparkle_populationPercent[0] = effect_option_sync ? populate1 : populate2;
}

void setEffectOptions_sparkleFadeRing(unsigned long duration1, unsigned long duration2, int populate1, int populate2) {
  effect_sparkle_eventDuration[1] = duration1;
  effect_sparkle_eventDuration[0] = effect_option_sync ? duration1 : duration2;;
  effect_sparkle_populationPercent[1] = populate1;
  effect_sparkle_populationPercent[0] = effect_option_sync ? populate1 : populate2;
}

void setEffectOptions_strobe(int duration1, int duration2, int pause1, int pause2) {
  effect_strobe_flashDuration[1] = duration1;
  effect_strobe_flashDuration[0] = effect_option_sync ? duration1 : duration2;
  effect_strobe_flashPause[1]    = pause1;
  effect_strobe_flashPause[0]    = effect_option_sync ? pause1 : pause2;
}

void setEffectOptions_strobeRing(int d00, int d01, int d02, int d10, int d11, int d12, int p00, int p01, int p02, int p10, int p11, int p12) {
  effect_strobeRing_flashDuration[1][0] = d00;
  effect_strobeRing_flashDuration[1][1] = d01;
  effect_strobeRing_flashDuration[1][2] = d02;
  effect_strobeRing_flashDuration[0][0] = d10;
  effect_strobeRing_flashDuration[0][1] = d11;
  effect_strobeRing_flashDuration[0][2] = d12;
  effect_strobeRing_flashPause[1][0] = p00;
  effect_strobeRing_flashPause[1][1] = p01;
  effect_strobeRing_flashPause[1][2] = p02;
  effect_strobeRing_flashPause[0][0] = p10;
  effect_strobeRing_flashPause[0][1] = p11;
  effect_strobeRing_flashPause[0][2] = p12;
}

void setEffectOptions_pulse(int d0, int d1, int pf0, int pf1, int pz0, int pz1) {
  effect_pulse_travelDuration[1]    = d1;
  effect_pulse_pauseDurationAtFull[1] = pf1;
  effect_pulse_pauseDurationAtZero[1] = pz1;

  effect_pulse_travelDuration[0]    = d0;
  effect_pulse_pauseDurationAtFull[0] = pf0;
  effect_pulse_pauseDurationAtZero[0] = pz0;
}

void setEffectOptions_pulseRing_travelDurations(int l0, int l1, int l2, int r0, int r1, int r2) {
  effect_pulseRing_travelDuration[1][0] = l0;
  effect_pulseRing_travelDuration[1][1] = l1;
  effect_pulseRing_travelDuration[1][2] = l2;
  effect_pulseRing_travelDuration[0][0] = r0;
  effect_pulseRing_travelDuration[0][1] = r1;
  effect_pulseRing_travelDuration[0][2] = r2;
}

void setEffectOptions_pulseRing_pauseDurationAtFull(int l0, int l1, int l2, int r0, int r1, int r2) {
  effect_pulseRing_pauseDurationAtFull[1][0] = l0;
  effect_pulseRing_pauseDurationAtFull[1][1] = l1;
  effect_pulseRing_pauseDurationAtFull[1][2] = l2;
  effect_pulseRing_pauseDurationAtFull[0][0] = r0;
  effect_pulseRing_pauseDurationAtFull[0][1] = r1;
  effect_pulseRing_pauseDurationAtFull[0][2] = r2;
}

void setEffectOptions_pulseRing_pauseDurationAtZero(int l0, int l1, int l2, int r0, int r1, int r2) {
  effect_pulseRing_pauseDurationAtZero[1][0] = l0;
  effect_pulseRing_pauseDurationAtZero[1][1] = l1;
  effect_pulseRing_pauseDurationAtZero[1][2] = l2;
  effect_pulseRing_pauseDurationAtZero[0][0] = r0;
  effect_pulseRing_pauseDurationAtZero[0][1] = r1;
  effect_pulseRing_pauseDurationAtZero[0][2] = r2;
}

void setEffectOptions_boomHue(int isLeft, int hueCenter) {
  int hueInner = getFlowerPetalHue(hueCenter, 1);
  int hueOuter = getFlowerPetalHue(hueCenter, 2);
  setEffectOptions_boomSide(isLeft, hueCenter, hueInner, hueOuter);
}

void setEffectOptions_flowerHue(int isLeft, int hueCenter) {
  int hueInner = getFlowerPetalHue(hueCenter, 1);
  int hueOuter = getFlowerPetalHue(hueCenter, 2);
  setEffectOptions_flowerSide(isLeft, hueCenter, hueInner, hueOuter);
}

void setEffectOptions_boomSide(int isLeft, int hueCenter, int hueInner, int hueOuter) {
  effect_boom_color_hue[isLeft][0] = hueCenter;
  effect_boom_color_hue[isLeft][1] = hueInner;
  effect_boom_color_hue[isLeft][2] = hueOuter;
}

void setEffectOptions_flowerSide(int isLeft, int hueCenter, int hueInner, int hueOuter) {
  effect_flower_color_hue[isLeft][0] = hueCenter;
  effect_flower_color_hue[isLeft][1] = hueInner;
  effect_flower_color_hue[isLeft][2] = hueOuter;
}

int getFlowerPetalHue(int hueCenter, int ringIdx) {
  int hueOutput = hueCenter + effect_flower_color_hue_petal_diff;
  if (ringIdx == 1) {
    hueOutput = hueCenter + effect_flower_color_hue_petal_diff;
  } else {
    hueOutput = hueCenter + (effect_flower_color_hue_petal_diff * 2);
  }
  if (hueOutput > 255) {
    hueOutput = hueOutput - 255;
  }
  return hueOutput;
}




/*
   ---------- EFFECTS ----------
   effect_mode:
    0 - Sparkle
    1 - Sparkle Fade
    2 - Sparkle Fade (Rings)
    3 - Strobe
    4 - Strobe (Rings)
    5 - Pulse
    6 - Pulse (Rings)
    7 - Spin
    8 - Flower
    9 - Flower Sparkle
    10 - Boom
*/
void updateEffectsNew() {
  //  effect_cycle_lastUpdated = millis();
  int currMode = 0;

  // Left & right sides
  for (int isLeft = 1; isLeft > -1; isLeft--) {
    currMode = effect_modes[isLeft]; // Current mode for active side
    //    Effects[currMode].update(isLeft); // Update active effect
    for (int ledIndex = 0; ledIndex < 41; ledIndex++) { // Get light values set by update
      if (isLeft) {
        //        eye_left.setPixelColor(ledIndex, Effects[currMode].getLEDColors(isLeft, ledIndex));
      } else {
        //        eye_right.setPixelColor(ledIndex, Effects[currMode].getLEDColors(isLeft, ledIndex));
      }
    }
    if (isLeft) {
      eye_left.show();
    } else {
      eye_right.show();
    }
  }

}

void updateEffects() {
  if (effect_modes[1] == 11 || effect_modes[0] == 11) {
    readSpectrum();
  }

  if (audio_effect_overrideLights) {
    updateEffectStrobe(1);
    if (!effect_option_sync) {
      updateEffectStrobe(0);
    }
    return;
  }

  // Left effect
  switch (effect_modes[1]) {
    case 0:
      updateEffectSparkle(1);
      break;
    case 1:
      updateEffectSparkleFade(1);
      break;
    case 2:
      updateEffectSparkleFadeRing(1);
      break;
    case 3:
      updateEffectStrobe(1);
      break;
    case 4:
      updateEffectStrobeRing(1);
      break;
    case 5:
      updateEffectPulse(1);
      break;
    case 6:
      updateEffectPulseRing(1);
      break;
    case 7:
      updateEffectSpin(1);
      break;
    case 8: case 9:
      updateEffectFlower(1);
      break;
    case 10:
      updateEffectBoom(1);
      break;
    case 11:
      updateEffectAudio(1);
      break;
    case 12:
      updateEffectAudioStripes(1);
      break;
  }

  if (!effect_option_sync) {
    // Right effect
    switch (effect_modes[0]) {
      case 0:
        updateEffectSparkle(0);
        break;
      case 1:
        updateEffectSparkleFade(0);
        break;
      case 2:
        updateEffectSparkleFadeRing(0);
        break;
      case 3:
        updateEffectStrobe(0);
        break;
      case 4:
        updateEffectStrobeRing(0);
        break;
      case 5:
        updateEffectPulse(0);
        break;
      case 6:
        updateEffectPulseRing(0);
        break;
      case 7:
        updateEffectSpin(0);
        break;
      case 8: case 9:
        updateEffectFlower(0);
        break;
      case 10:
        updateEffectBoom(0);
        break;
      case 11:
        updateEffectAudio(0);
        break;
      case 12:
        updateEffectAudioStripes(0);
        break;
    }
  }

}


void updateEffectAudioStripes(int isLeft) {
  int i;
  uint32_t colors[7];
  for (i = 0; i < 7; i++) {

    if (isLeft) {
      colors[i] = getColorByVal(isLeft, left[0]);
    } else {
      colors[i] = getColorByVal(effect_option_sync ? 1 : 0, right[0]);
    }
  }

  setLightsOuterRingSegmentRight(isLeft, left[1], colors[1]);
  setLightsOuterRingSegmentLeft(isLeft, left[2], colors[2]);
  setLightsOuterRingSegmentBottom(isLeft, left[3], colors[3]);

  setLightsInnerRingSegmentRight(isLeft, left[4], colors[4]);
  setLightsInnerRingSegmentLeft(isLeft, left[5], colors[5]);

  if (effect_option_sync && isLeft) {
    updateEffectAudioStripes(0);
  }

  if (isLeft) {
    eye_left.show();
  } else {
    eye_right.show();
  }

}

#define SEGMENT_OUTER_RIGHT_INDEX_START 0
#define SEGMENT_OUTER_RIGHT_INDEX_END 7

#define SEGMENT_OUTER_BOTTOM_INDEX_START 8
#define SEGMENT_OUTER_BOTTOM_INDEX_END 15

#define SEGMENT_OUTER_LEFT_INDEX_START 16
#define SEGMENT_OUTER_LEFT_INDEX_END 23

#define SEGMENT_INNER_LEFT_INDEX_START 24
#define SEGMENT_INNER_LEFT_INDEX_END 31

#define SEGMENT_INNER_RIGHT_INDEX_START 32
#define SEGMENT_INNER_RIGHT_INDEX_END 39

void setLightsInnerRingSegmentRight(int isLeft, int numLights, uint32_t color) {
  uint32_t tempColor;
  for (int i = SEGMENT_INNER_RIGHT_INDEX_START; i <= SEGMENT_INNER_RIGHT_INDEX_END; i++) {
    tempColor = ((i - SEGMENT_INNER_RIGHT_INDEX_START) <= numLights) ? color : BLACK;
    if (isLeft) {
      eye_left.setPixelColor(i, tempColor);
    } else {
      eye_right.setPixelColor(i, tempColor);
    }
  }
}

void setLightsInnerRingSegmentLeft(int isLeft, int numLights, uint32_t color) {
  uint32_t tempColor;
  for (int i = SEGMENT_INNER_LEFT_INDEX_START; i <= SEGMENT_INNER_LEFT_INDEX_END; i++) {
    tempColor = (numLights <= (i - SEGMENT_INNER_LEFT_INDEX_START)) ? color : BLACK;
    if (isLeft) {
      eye_left.setPixelColor(i, tempColor);
    } else {
      eye_right.setPixelColor(i, tempColor);
    }
  }
}

void setLightsOuterRingSegmentRight(int isLeft, int numLights, uint32_t color) {
  uint32_t tempColor;
  for (int i = SEGMENT_OUTER_RIGHT_INDEX_START; i <= SEGMENT_OUTER_RIGHT_INDEX_END; i++) {
    tempColor = ((i - SEGMENT_OUTER_RIGHT_INDEX_START) <= numLights) ? color : BLACK;
    if (isLeft) {
      eye_left.setPixelColor(i, tempColor);
    } else {
      eye_right.setPixelColor(i, tempColor);
    }
  }
}

void setLightsOuterRingSegmentBottom(int isLeft, int numLights, uint32_t color) {
  uint32_t tempColor;
  for (int i = SEGMENT_OUTER_BOTTOM_INDEX_START; i <= SEGMENT_OUTER_BOTTOM_INDEX_END; i++) {
    tempColor = ((i - SEGMENT_OUTER_BOTTOM_INDEX_START) <= numLights) ? color : BLACK;
    if (isLeft) {
      eye_left.setPixelColor(i, tempColor);
    } else {
      eye_right.setPixelColor(i, tempColor);
    }
  }
}

void setLightsOuterRingSegmentLeft(int isLeft, int numLights, uint32_t color) {
  uint32_t tempColor;
  for (int i = SEGMENT_OUTER_LEFT_INDEX_START; i <= SEGMENT_OUTER_LEFT_INDEX_END; i++) {
    tempColor = (numLights <= (i - SEGMENT_OUTER_LEFT_INDEX_START)) ? color : BLACK;
    if (isLeft) {
      eye_left.setPixelColor(i, tempColor);
    } else {
      eye_right.setPixelColor(i, tempColor);
    }
  }
}


void updateEffectAudio(int isLeft) {
  int i;

  effectAudioCenter(isLeft,
                    isLeft ? left[0] : right[0],
                    isLeft ? left[1] : right[1]
                   );
  effectAudioInner(isLeft,
                   isLeft ? left[2] : right[2],
                   isLeft ? left[3] : right[3]
                  );
  effectAudioOuter(isLeft,
                   isLeft ? left[4] : right[4],
                   isLeft ? left[5] : right[5],
                   isLeft ? left[6] : right[6]
                  );
  if (effect_option_sync && isLeft) {
    updateEffectAudio(0);
  }
  //  for (i = 0; i < 7; i++) {
  //    if (isLeft) {
  //
  //    } else {
  //
  //    }
  //  }
}

void effectAudioCenter(int isLeft, int band_0, int band_1) {
  uint32_t color = getColorByVal(effect_option_sync ? 1 : isLeft, map( ((band_0 + band_1) / 2), 0, 1023, 0, 255));
  lensFillIdx(isLeft, 0, color);
}

void effectAudioInner(int isLeft, int band_2, int band_3) {
  uint32_t color = getColorByVal(effect_option_sync ? 1 : isLeft, map(((band_2 + band_3) / 2), 0, 1023, 0, 255));
  lensFillIdx(isLeft, 1, color);
}

void effectAudioOuter(int isLeft, int band_4, int band_5, int band_6) {
  uint32_t color = getColorByVal(effect_option_sync ? 1 : isLeft, map( ((band_4 + band_5 + band_6) / 3), 0, 1023, 0, 255));
  lensFillIdx(isLeft, 2, color);
}

/*
   Effect update - sparkle
*/
void updateEffectSparkle(int isLeft) {
  //  Serial.println("updateEffectSparkle()");
  if (millis() >= (effect_sparkle_event_lastUpdated[isLeft] + effect_sparkle_eventDuration[isLeft])) {
    effect_sparkle_event_lastUpdated[isLeft] = millis();
    //    if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
    uint32_t color = getColor(isLeft);
    eyeColorSparkle(isLeft, color, effect_manual_chanceSparkle[isLeft]);
    if (isLeft && effect_option_sync) {
      eyeColorSparkle(0, color, effect_manual_chanceSparkle[isLeft]);
    }
  }
}


void eyeColorSparkle(byte isLeft, uint32_t color, int percentSparkle) {
  uint32_t tmpColor;
  for (int currIndex = 0; currIndex < eye_left.numPixels(); currIndex++) {
    tmpColor = (random(0, 100) <= percentSparkle) ? getSparkleColor(isLeft, color) : eye_left.Color(0, 0, 0);
    if (isLeft) {
      eye_left.setPixelColor(currIndex, tmpColor);
    } else {
      eye_right.setPixelColor(currIndex, tmpColor);
    }
  }

  if (isLeft) {
    eye_left.show();
  } else {
    eye_right.show();
  }
}




void eyeColorSparkleIdx(byte isLeft, byte idx, uint32_t color, int percentSparkle) {
  switch (idx) {
    case 0:
      eyeSparkleCenter(isLeft, idx, color, percentSparkle);
      break;
    case 1:
      eyeSparkleInner(isLeft, idx, color, percentSparkle);
      break;
    case 2:
      eyeSparkleOuter(isLeft, idx, color, percentSparkle);
      break;
  }
}

void eyeSparkleCenter(int isLeft, int idx, uint32_t color, int percentSparkle) {
  //  uint32_t tmpColor = (random(0, 100) <= percentSparkle) ? getSparkleColor(isLeft, color) : eye_left.Color(0, 0, 0);
  uint32_t tmpColor = (random(0, 100) <= percentSparkle) ? color : eye_left.Color(0, 0, 0);
  if (isLeft) {
    eye_left.setPixelColor(eye_left.numPixels() - 1, tmpColor);
    eye_left.show();
  } else {
    eye_right.setPixelColor(eye_right.numPixels() - 1, tmpColor);
    eye_right.show();
  }
}

void eyeSparkleInner(int isLeft, int idx, uint32_t color, int percentSparkle) {
  uint32_t tmpColor;
  if (isLeft) {
    for (int currIndex = 24; currIndex < eye_left.numPixels() - 1; currIndex++) {
      tmpColor = (random(0, 100) <= percentSparkle) ? color : eye_left.Color(0, 0, 0);
      eye_left.setPixelColor(currIndex, tmpColor);
    }

    eye_left.show();
  } else {
    for (int currIndex = 24; currIndex < eye_right.numPixels() - 1; currIndex++) {
      tmpColor = (random(0, 100) <= percentSparkle) ? color : eye_left.Color(0, 0, 0);
      eye_right.setPixelColor(currIndex, tmpColor);
    }
    eye_right.show();
  }
}

void eyeSparkleOuter(int isLeft, int idx, uint32_t color, int percentSparkle) {
  uint32_t tmpColor;
  if (isLeft) {
    for (int currIndex = 0; currIndex < eye_left.numPixels() - 17; currIndex++) {
      tmpColor = (random(0, 100) <= percentSparkle) ? color : eye_left.Color(0, 0, 0);
      eye_left.setPixelColor(currIndex, tmpColor);
    }
    eye_left.show();
  } else {
    for (int currIndex = 0; currIndex < eye_right.numPixels() - 17; currIndex++) {
      tmpColor = (random(0, 100) <= percentSparkle) ? color : eye_left.Color(0, 0, 0);
      eye_right.setPixelColor(currIndex, tmpColor);
    }
    eye_right.show();
  }
}


uint32_t getSparkleColor(int isLeft, uint32_t color) {
  int useRandom = ((effect_color_modes[isLeft] == 4) || (effect_option_sync && effect_color_modes[1] == 4));
  return useRandom ? getRainbowColorByVal(isLeft, random(2, 255)) : color;

}



/*
  Eye Sparkle Fade
*/
void updateEffectSparkleFade(int isLeft) {
  if (effect_sparkleFade_atZero[isLeft] || effect_sparkleFade_atFull[isLeft]) { // Sitting at zero or full
    updateSparkleFadePause(isLeft);
  } else {
    updateSparkleFadeTravel(isLeft);
  }
}


void updateSparkleFadePause(int isLeft) {
  //  Serial.println("updateSparkleFadePause");
  if (millis() >= effect_sparkleFade_pause_end[isLeft]) {
    if (effect_sparkleFade_atZero[isLeft]) {
      beginSparkleFadeTravelToFull(isLeft);
    } else {
      beginSparkleFadeTravelToZero(isLeft);
    }
  } else {
    // Prevent sparkle from stagnating
    displaySparkleFade(isLeft, effect_sparkleFade_atZero[isLeft] ? 0 : 255);
  }
}

void updateSparkleFadeTravel(int isLeft) {
  byte travelComplete = (millis() >= (effect_sparkleFade_travel_start[isLeft] + effect_sparkleFade_travelDuration[isLeft]));
  int travelValue = map(millis(),
                        effect_sparkleFade_travel_start[isLeft],
                        effect_sparkleFade_travel_start[isLeft] + effect_sparkleFade_travelDuration[isLeft],
                        0,
                        255); // make into vars
  if (effect_sparkleFade_travel_direction[isLeft] == 1) {

    if (travelComplete) {
      setSparkleFadeToFull(isLeft);
    } else {
      displaySparkleFade(isLeft, travelValue);
    }
  } else {

    if (travelComplete) {
      setSparkleFadeToZero(isLeft);
    } else {
      displaySparkleFade(isLeft, 255 - travelValue);
    }
  }
}

void beginSparkleFadeTravelToFull(int isLeft) {
  effect_sparkleFade_atZero[isLeft] = 0;
  effect_sparkleFade_atFull[isLeft] = 0;
  effect_sparkleFade_travel_start[isLeft] = millis();
  effect_sparkleFade_travel_direction[isLeft] = 1;
}

void beginSparkleFadeTravelToZero(int isLeft) {
  effect_sparkleFade_atZero[isLeft] = 0;
  effect_sparkleFade_atFull[isLeft] = 0;
  effect_sparkleFade_travel_start[isLeft] = millis();
  effect_sparkleFade_travel_direction[isLeft] = 0;
}

void setSparkleFadeToFull(int isLeft) {
  //  Serial.println("setSparkleFadeToFull");
  effect_sparkleFade_pause_start[isLeft] = millis();
  effect_sparkleFade_pause_end[isLeft]   = (effect_sparkleFade_pause_start[isLeft] + effect_sparkleFade_pauseDurationAtFull[isLeft]);
  effect_sparkleFade_atFull[isLeft] = 1;
  effect_sparkleFade_travel_end[isLeft] = millis();
}

void setSparkleFadeToZero(int isLeft) {
  effect_sparkleFade_pause_start[isLeft] = millis();
  effect_sparkleFade_pause_end[isLeft]   = (effect_sparkleFade_pause_start[isLeft] + effect_sparkleFade_pauseDurationAtZero[isLeft]);
  effect_sparkleFade_atZero[isLeft] = 1;
  effect_sparkleFade_travel_end[isLeft] = millis();
}


void displaySparkleFade(int isLeft, int travelComplete) {
  //  if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
  uint32_t tempColor = getColor(isLeft);
  int sparklePercent = map(travelComplete, 0, 255, 0, 100);
  eyeColorSparkle(isLeft, tempColor, sparklePercent);
  if (isLeft && effect_option_sync) {
    eyeColorSparkle(0, tempColor, sparklePercent);
  }
}



/*
  Eye Sparkle Fade (Rings)
*/
void updateEffectSparkleFadeRing(int isLeft) {
  for (int q = 0; q < 3; q++) {
    if (effect_sparkleFadeRing_atZero[isLeft][q] || effect_sparkleFadeRing_atFull[isLeft][q]) { // Sitting at zero or full
      updateSparkleFadeRingPause(isLeft, q);
    } else {
      updateSparkleFadeRingTravel(isLeft, q);
    }
  }
}

void updateSparkleFadeRingPause(int isLeft, int idx) {
  if (millis() >= effect_sparkleFadeRing_pause_end[isLeft][idx]) {
    if (effect_sparkleFadeRing_atZero[isLeft][idx]) {
      beginSparkleFadeRingTravelToFull(isLeft, idx);
    } else {
      beginSparkleFadeRingTravelToZero(isLeft, idx);
    }
  } else {
    // Prevent color from stagnating
    displaySparkleFadeRing(isLeft, idx, effect_sparkleFadeRing_atZero[isLeft][idx] ? 0 : 255);
  }
}



void beginSparkleFadeRingTravelToFull(int isLeft, int idx) {
  effect_sparkleFadeRing_atZero[isLeft][idx] = 0;
  effect_sparkleFadeRing_atFull[isLeft][idx] = 0;
  effect_sparkleFadeRing_travel_start[isLeft][idx] = millis();
  effect_sparkleFadeRing_travel_direction[isLeft][idx] = 1;
}

void beginSparkleFadeRingTravelToZero(int isLeft, int idx) {
  effect_sparkleFadeRing_atZero[isLeft][idx] = 0;
  effect_sparkleFadeRing_atFull[isLeft][idx] = 0;
  effect_sparkleFadeRing_travel_start[isLeft][idx] = millis();
  effect_sparkleFadeRing_travel_direction[isLeft][idx] = 0;
}


void updateSparkleFadeRingTravel(int isLeft, int idx) {
  byte travelComplete = (millis() >= (effect_sparkleFadeRing_travel_start[isLeft][idx] + effect_sparkleFadeRing_travelDuration[isLeft][idx]));
  int travelValue = map(millis(),
                        effect_sparkleFadeRing_travel_start[isLeft][idx],
                        effect_sparkleFadeRing_travel_start[isLeft][idx] + effect_sparkleFadeRing_travelDuration[isLeft][idx],
                        0,
                        255); // make into vars
  if (effect_sparkleFadeRing_travel_direction[isLeft][idx] == 1) {

    if (travelComplete) {
      setSparkleFadeRingToFull(isLeft, idx);
      if (isLeft && effect_option_sync) {
        setSparkleFadeRingToFull(0, idx);
      }
    } else {
      displaySparkleFadeRing(isLeft, idx, travelValue);

    }
  } else {

    if (travelComplete) {
      setSparkleFadeRingToZero(isLeft, idx);
      if (isLeft && effect_option_sync) {
        setSparkleFadeRingToZero(0, idx);
      }
    } else {
      displaySparkleFadeRing(isLeft, idx, 255 - travelValue);

    }
  }
}




void setSparkleFadeRingToFull(int isLeft, int idx) {
  effect_sparkleFadeRing_pause_start[isLeft][idx] = millis();
  effect_sparkleFadeRing_pause_end[isLeft][idx]   = (effect_sparkleFadeRing_pause_start[isLeft][idx] +
      effect_sparkleFadeRing_pauseDurationAtFull[isLeft][idx]);
  effect_sparkleFadeRing_atFull[isLeft][idx] = 1;
  effect_sparkleFadeRing_travel_end[isLeft][idx] = millis();
}

void setSparkleFadeRingToZero(int isLeft, int idx) {
  effect_sparkleFadeRing_pause_start[isLeft][idx] = millis();
  effect_sparkleFadeRing_pause_end[isLeft][idx]   = (effect_sparkleFadeRing_pause_start[isLeft][idx] +
      effect_sparkleFadeRing_pauseDurationAtZero[isLeft][idx]);
  effect_sparkleFadeRing_atZero[isLeft][idx] = 1;
  effect_sparkleFadeRing_travel_end[isLeft][idx] = millis();
}

void displaySparkleFadeRing(int isLeft, int idx, int travelComplete) {
  uint32_t color = getColorByVal(isLeft, travelComplete);
  eyeColorSparkleIdx(isLeft, idx, color, travelComplete);
  if (isLeft && effect_option_sync) {
    eyeColorSparkleIdx(0, idx, color, travelComplete);
  }

}









/*
   Effect update - pulse (rings)
*/
void updateEffectPulseRing(int i) {
  for (int q = 0; q < 3; q++) {
    if (effect_pulseRing_atZero[i][q] || effect_pulseRing_atFull[i][q]) { // Sitting at zero or full
      updatePulseRingPause(i, q);
    } else {
      updatePulseRingTravel(i, q);
    }
  }
}

void updatePulseRingPause(int isLeft, int idx) {
  if (millis() >= effect_pulseRing_pause_end[isLeft][idx]) {
    if (effect_pulseRing_atZero[isLeft][idx]) {
      beginPulseRingTravelToFull(isLeft, idx);
    } else {
      beginPulseRingTravelToZero(isLeft, idx);
    }
  } else {
    // Prevent color from stagnating
    displayPulseRing(isLeft, idx, effect_pulseRing_atZero[isLeft][idx] ? 0 : 255);
  }
}



void beginPulseRingTravelToFull(int isLeft, int idx) {
  effect_pulseRing_atZero[isLeft][idx] = 0;
  effect_pulseRing_atFull[isLeft][idx] = 0;
  effect_pulseRing_travel_start[isLeft][idx] = millis();
  effect_pulseRing_travel_direction[isLeft][idx] = 1;
}

void beginPulseRingTravelToZero(int isLeft, int idx) {
  effect_pulseRing_atZero[isLeft][idx] = 0;
  effect_pulseRing_atFull[isLeft][idx] = 0;
  effect_pulseRing_travel_start[isLeft][idx] = millis();
  effect_pulseRing_travel_direction[isLeft][idx] = 0;
}


void updatePulseRingTravel(int isLeft, int idx) {
  byte travelComplete = (millis() >= (effect_pulseRing_travel_start[isLeft][idx] + effect_pulseRing_travelDuration[isLeft][idx]));
  int travelValue = map(millis(),
                        effect_pulseRing_travel_start[isLeft][idx],
                        effect_pulseRing_travel_start[isLeft][idx] + effect_pulseRing_travelDuration[isLeft][idx],
                        0,
                        255); // make into vars
  if (effect_pulseRing_travel_direction[isLeft][idx] == 1) {

    if (travelComplete) {
      setPulseRingToFull(isLeft, idx);
      if (isLeft && effect_option_sync) {
        setPulseRingToFull(0, idx);
      }
    } else {
      displayPulseRing(isLeft, idx, travelValue);

    }
  } else {

    if (travelComplete) {
      setPulseRingToZero(isLeft, idx);
      if (isLeft && effect_option_sync) {
        setPulseRingToZero(0, idx);
      }
    } else {
      displayPulseRing(isLeft, idx, 255 - travelValue);

    }
  }
}


void setPulseRingToFull(int isLeft, int idx) {
  effect_pulseRing_pause_start[isLeft][idx] = millis();
  effect_pulseRing_pause_end[isLeft][idx]   = (effect_pulseRing_pause_start[isLeft][idx] + effect_pulseRing_pauseDurationAtFull[isLeft][idx]);
  effect_pulseRing_atFull[isLeft][idx] = 1;
  effect_pulseRing_travel_end[isLeft][idx] = millis();
}

void setPulseRingToZero(int isLeft, int idx) {
  effect_pulseRing_pause_start[isLeft][idx] = millis();
  effect_pulseRing_pause_end[isLeft][idx]   = (effect_pulseRing_pause_start[isLeft][idx] + effect_pulseRing_pauseDurationAtZero[isLeft][idx]);
  effect_pulseRing_atZero[isLeft][idx] = 1;
  effect_pulseRing_travel_end[isLeft][idx] = millis();
}

void displayPulseRing(int isLeft, int idx, int travelComplete) {
  uint32_t color;
  if (inXmasMode()) {
    color = getXmasColorVal(isLeft, idx, travelComplete);
  } else {
    color = getColorByVal(isLeft, travelComplete);
  }
  lensFillIdx(isLeft, idx, color);
  if (isLeft && effect_option_sync) {
    lensFillIdx(0, idx, color);
  }

}





/*
   Effect update - spin
*/
void updateEffectSpin(int isLeft) {
  //  if (millis() >= effect_spin_lastUpdated[isLeft] + effect_spin_update_interval) {
  effect_spin_lastUpdated[isLeft] = millis();

  for (int q = 1; q < 3; q++) {
    updateSpinTravel(isLeft, q);
  }
  //  }
}

void updateSpinTravel(int isLeft, int idx) {

  if (millis() >= effect_spin_travelStopTime[isLeft][idx]) {
    restartSpinLoop(isLeft, idx);
    return;
  }

  // Determine current step based on time through duration
  int stepIndex = map(
                    millis(),
                    effect_spin_travel_start[isLeft][idx],
                    effect_spin_travelStopTime[isLeft][idx],
                    0,
                    effect_spin_numLights[isLeft][idx] - 1
                  );
  if (stepIndex != effect_spin_stepIndex[isLeft][idx]) {
    effect_spin_stepIndex[isLeft][idx] = stepIndex;
    displaySpinState(isLeft, idx, stepIndex);
  }
}


void restartSpinLoop(int isLeft, int idx) {
  effect_spin_travel_start[isLeft][idx] = millis();
  effect_spin_travelStopTime[isLeft][idx] = effect_spin_travel_start[isLeft][idx] + effect_spin_travelDuration[isLeft][idx];
}


void displaySpinState(int isLeft, int idx, int currIdx) {
  int targetIndex;
  uint32_t tempColor;
  int clockwise = (effect_spin_travel_direction[isLeft][idx] == 0);

  for (int x = 0; x < effect_spin_numLights[isLeft][idx]; x++) {
    targetIndex = getDifferenceFromActiveIndexInLoop(x, currIdx, effect_spin_numLights[isLeft][idx]);
    //    if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
    tempColor = getSpinColor(isLeft, idx, targetIndex, effect_spin_numLights[isLeft][idx]);
    switch (idx) {
      case 0:
        break;
      case 1:
        eyeSetInnerByIndex(isLeft, clockwise ? 15 - x : x, tempColor);
        if (isLeft && effect_option_sync) {
          eyeSetInnerByIndex(0, (effect_spin_travel_direction[0][idx] == 0) ? 15 - x : x, tempColor);
        }
        break;
      case 2:
        eyeSetOuterByIndex(isLeft, clockwise ? x : 23 - x, tempColor);
        if (isLeft && effect_option_sync) {
          eyeSetOuterByIndex(0, (effect_spin_travel_direction[0][idx] == 0) ? x : 23 - x, tempColor);
        }
        break;
    }
  }
}

uint32_t getSpinColor(int isLeft, int idx, int spinIndex, int spinMax) {
  //  return getRainbowColorByVal(map(spinIndex, 0, spinMax - 1, 255, 0));
  return getColorByVal(isLeft, map(spinIndex, 0, spinMax - 1, 255, 0));
}



void updateEffectFlower(int i) {
  if (effect_flower_atZero[i] || effect_flower_atFull[i]) { // Sitting at zero or full
    updateFlowerPause(i);
  } else {
    updateFlowerTravel(i);
  }
}


void updateFlowerPause(int isLeft) {
  if (millis() >= effect_flower_pause_end[isLeft]) {
    if (effect_flower_atZero[isLeft]) {
      beginFlowerTravelToFull(isLeft);
    } else {
      beginFlowerTravelToZero(isLeft);
    }
  } else {
    // Prevent color from stagnating
    displayFlower(isLeft, effect_flower_atZero[isLeft] ? 0 : 1023);

    //    if (effect_flower_transition_type[isLeft] == 0) { // fade(0) or sparkle(1)
    //      displayNormalFlower(isLeft, effect_flower_atZero[isLeft] ? 0 : 1023);
    //    } else {
    //      displayFlowerSparkle(isLeft, effect_flower_atZero[isLeft] ? 0 : 1023);
    //    }
    //    if (isLeft && effect_option_sync) {
    //      displayFlower(0, effect_flower_atZero[isLeft] ? 0 : 1023);
    //      //      if (effect_flower_transition_type[isLeft] == 0) {
    //      //        displayNormalFlower(0, effect_flower_atZero[isLeft] ? 0 : 1023);
    //      //      } else {
    //      //        displayFlowerSparkle(0, effect_flower_atZero[isLeft] ? 0 : 1023);
    //      //      }
    //    }
  }
}



void beginFlowerTravelToFull(int isLeft) {
  effect_flower_atZero[isLeft] = 0;
  effect_flower_atFull[isLeft] = 0;
  effect_flower_travel_start[isLeft] = millis();
  effect_flower_travel_direction[isLeft] = 1;
}

void beginFlowerTravelToZero(int isLeft) {
  effect_flower_atZero[isLeft] = 0;
  effect_flower_atFull[isLeft] = 0;
  effect_flower_travel_start[isLeft] = millis();
  effect_flower_travel_direction[isLeft] = 0;
}


void updateFlowerTravel(int isLeft) {
  byte travelComplete = (millis() >= (effect_flower_travel_start[isLeft] + effect_flower_travelDuration[isLeft]));
  int travelValue = map(millis(),
                        effect_flower_travel_start[isLeft],
                        effect_flower_travel_start[isLeft] + effect_flower_travelDuration[isLeft],
                        0,
                        1023); // make into vars
  if (effect_flower_travel_direction[isLeft] == 1) {

    if (travelComplete) {
      setFlowerToFull(isLeft);
      if (isLeft && effect_option_sync) {
        setFlowerToFull(0);
      }
    } else {
      displayFlower(isLeft, travelValue);

      //      if (effect_flower_transition_type[isLeft] == 0) {
      //        displayNormalFlower(isLeft, travelValue);
      //      } else {
      //        displayFlowerSparkle(isLeft, travelValue);
      //      }

      //      if (isLeft && effect_option_sync) {
      //        displayFlower(0, travelValue);
      //
      //        //        if (effect_flower_transition_type[isLeft] == 0) {
      //        //          displayNormalFlower(0, travelValue);
      //        //        } else {
      //        //          displayFlowerSparkle(0, travelValue);
      //        //        }
      //      }
    }
  } else {

    if (travelComplete) {
      setFlowerToZero(isLeft);
      if (isLeft && effect_option_sync) {
        setFlowerToZero(0);
      }
    } else {

      displayFlower(isLeft, 1023 - travelValue);

    }
  }
}


void setFlowerToFull(int isLeft) {
  effect_flower_pause_start[isLeft] = millis();
  effect_flower_pause_end[isLeft]   = (effect_flower_pause_start[isLeft] + effect_flower_pauseDurationAtFull[isLeft]);
  effect_flower_atFull[isLeft] = 1;
  effect_flower_travel_end[isLeft] = millis();
}

void setFlowerToZero(int isLeft) {
  effect_flower_pause_start[isLeft] = millis();
  effect_flower_pause_end[isLeft]   = (effect_flower_pause_start[isLeft] + effect_flower_pauseDurationAtZero[isLeft]);
  effect_flower_atZero[isLeft] = 1;
  effect_flower_travel_end[isLeft] = millis();

  // change color
  //  if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
  setEffectOptions_flowerHue(isLeft, effect_color_rainbow_index[isLeft]);
  if (isLeft == 0 && effect_option_sync) {
    setEffectOptions_flowerHue(isLeft, effect_color_rainbow_index[1]);
  }
}



void updateEffectBoom(int isLeft) {
  if (effect_boom_atZero[isLeft] || effect_boom_atFull[isLeft]) { // Sitting at zero or full
    updateBoomPause(isLeft);
  } else {
    updateBoomTravel(isLeft);
  }
}


void updateBoomPause(int isLeft) {
  if (millis() >= effect_boom_pause_end[isLeft]) {
    if (effect_boom_atZero[isLeft]) {
      beginBoomTravelToFull(isLeft);
    } else {
      beginBoomTravelToZero(isLeft);
    }
  } else {
    // Prevent color from stagnating
    displayBoom(isLeft, effect_boom_atZero[isLeft] ? 0 : 1023);
  }
}

//effect_boom_travel_direction





void updateBoomTravel(int isLeft) {
  byte travelComplete = (millis() >= (effect_boom_travel_start[isLeft] + effect_boom_travelDuration[isLeft]));
  int travelValue = map(millis(),
                        effect_boom_travel_start[isLeft],
                        effect_boom_travel_start[isLeft] + effect_boom_travelDuration[isLeft],
                        0,
                        1023); // make into vars

  if (travelComplete) {
    setBoomToZero(isLeft);
    if (isLeft && effect_option_sync) {
      setBoomToZero(0);
    }
  } else {

    displayBoom(isLeft, travelValue);

  }
}


void setBoomToFull(int isLeft) {
  effect_boom_pause_start[isLeft] = millis();
  effect_boom_pause_end[isLeft]   = (effect_boom_pause_start[isLeft] + effect_boom_pauseDurationAtFull[isLeft]);
  effect_boom_atFull[isLeft] = 1;
  effect_boom_travel_end[isLeft] = millis();
}

void setBoomToZero(int isLeft) {
  effect_boom_pause_start[isLeft] = millis();
  effect_boom_pause_end[isLeft]   = (effect_boom_pause_start[isLeft] + effect_boom_pauseDurationAtZero[isLeft]);
  effect_boom_atZero[isLeft] = 1;
  effect_boom_travel_end[isLeft] = millis();

  // change color
  //  if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
  setEffectOptions_boomHue(isLeft, effect_color_rainbow_index[isLeft]);
  if (isLeft == 0 && effect_option_sync) {
    setEffectOptions_boomHue(isLeft, effect_color_rainbow_index[1]);
  }
}

void beginBoomTravelToFull(int isLeft) {
  Serial.println("beginBoomTravelToFull()");
  effect_boom_atZero[isLeft] = 0;
  effect_boom_atFull[isLeft] = 0;
  effect_boom_travel_start[isLeft] = millis();
  effect_boom_travel_direction[isLeft] = 1;
}

void beginBoomTravelToZero(int isLeft) {
  Serial.println("beginBoomTravelToZero()");
  effect_boom_atZero[isLeft] = 0;
  effect_boom_atFull[isLeft] = 0;
  effect_boom_travel_start[isLeft] = millis();
  effect_boom_travel_direction[isLeft] = 0;
}


void displayBoom(int isLeft, int travelComplete) {
  if (effect_boom_transition_type[isLeft] == 0) { // fade(0) or sparkle(1)
    displayNormalBoom(isLeft, travelComplete);
  } else {
    displaySparkleBoom(isLeft, travelComplete);
  }

  if (isLeft && effect_option_sync) {
    if (effect_boom_transition_type[isLeft] == 0) { // fade(0) or sparkle(1)
      displayNormalBoom(0, travelComplete);
    } else {
      displaySparkleBoom(0, travelComplete);
    }
  }
}



//int progressBoom[][2] = {{20, 153}, {200, 700}, {600, 1000}};


void displayNormalBoom(int isLeft, int travelComplete) {
  int ringTravel[3] = {0, 0, 0};
  uint32_t color;

  if (travelComplete <= 255) {
    ringTravel[0] = map(travelComplete, 0, 255, 0, 127);
    ringTravel[1] = 0;
    ringTravel[2] = 0;

  } else if (travelComplete > 255 && travelComplete < 511) {
    ringTravel[0] = map(travelComplete, 255, 511, 127, 255);
    ringTravel[1] = map(travelComplete, 255, 511, 0, 255);
    ringTravel[2] = 0;

  } else if (travelComplete > 512 && travelComplete < 767) {
    ringTravel[0] = map(travelComplete, 512, 600, 255, 0);
    ringTravel[1] = map(travelComplete, 512, 767, 255, 63);
    ringTravel[2] = map(travelComplete, 512, 767, 0, 255);
  } else if (travelComplete >= 768) {
    ringTravel[0] = 0;
    ringTravel[1] = map(travelComplete, 768, 900, 64, 0);
    ringTravel[2] = map(travelComplete, 768, 1023, 255, 0);
  }

  for (int x = 0; x < 3; x++) {
    if (ringTravel[x] < 0) {
      ringTravel[x] = 0;
    } else if (ringTravel[x] > 255) {
      ringTravel[x] = 255;
    }

    lensFillIdx(isLeft, x, getFlowerColor(isLeft, x, ringTravel[x]));
    if (isLeft == 0 && effect_option_sync) {
      lensFillIdx(0, x, getFlowerColor(1, x, ringTravel[x]));
    }
  }

}




void displaySparkleBoom(int isLeft, int travelComplete) {
  int ringTravel[3] = {0, 0, 0};
  int travelPercentage;
  uint32_t color;

  if (travelComplete <= 255) {
    ringTravel[0] = map(travelComplete, 0, 255, 0, 127);
    ringTravel[1] = 0;
    ringTravel[2] = 0;

  } else if (travelComplete > 255 && travelComplete < 511) {
    ringTravel[0] = map(travelComplete, 255, 511, 127, 255);
    ringTravel[1] = map(travelComplete, 255, 511, 0, 255);
    ringTravel[2] = 0;

  } else if (travelComplete > 512 && travelComplete < 767) {
    ringTravel[0] = map(travelComplete, 512, 600, 255, 0);
    ringTravel[1] = map(travelComplete, 512, 767, 255, 63);
    ringTravel[2] = map(travelComplete, 512, 767, 0, 255);

  } else if (travelComplete >= 768) {
    ringTravel[0] = 0;
    ringTravel[1] = map(travelComplete, 768, 900, 64, 0);
    ringTravel[2] = map(travelComplete, 768, 1023, 255, 0);
  }

  for (int x = 0; x < 3; x++) {

    if (ringTravel[x] < 0) {
      ringTravel[x] = 0;
    } else if (ringTravel[x] > 255) {
      ringTravel[x] = 255;
    }

    travelPercentage = map(ringTravel[x], 0, 255, 0, 100);
    eyeColorSparkleIdx(isLeft, x, getFlowerColor(isLeft, x, travelPercentage), travelPercentage);

    if (isLeft == 0 && effect_option_sync) {
      eyeColorSparkleIdx(0, x, getFlowerColor(1, x, travelPercentage), travelPercentage);
    }

  }

}




void displayFlower(int isLeft, int travelComplete) {
  if (effect_flower_transition_type[isLeft] == 0) { // fade(0) or sparkle(1)
    displayNormalFlower(isLeft, travelComplete);
  } else {
    displayFlowerSparkle(isLeft, travelComplete);
  }

  if (isLeft && effect_option_sync) {
    if (effect_flower_transition_type[isLeft] == 0) { // fade(0) or sparkle(1)
      displayNormalFlower(0, travelComplete);
    } else {
      displayFlowerSparkle(0, travelComplete);
    }
  }
}

void displayNormalFlower(int isLeft, int travelComplete) {
  int ringTravel[3] = {0, 0, 0};
  uint32_t color;

  if (travelComplete <= progressFlower[0][1]) {
    ringTravel[0] = map(travelComplete, progressFlower[0][0], progressFlower[0][1], 0, 255);
    ringTravel[1] = 0;
    ringTravel[2] = 0;
  } else if (travelComplete >= progressFlower[1][0] && travelComplete <= progressFlower[1][1]) {
    ringTravel[0] = 255;
    ringTravel[1] = map(travelComplete, progressFlower[1][0], progressFlower[1][1], 0, 255);
    ringTravel[2] = 0;
  } else if (travelComplete >= progressFlower[2][0]) {
    ringTravel[0] = 255;
    ringTravel[1] = 255;
    ringTravel[2] = map(travelComplete, progressFlower[2][0], progressFlower[2][1], 0, 255);
  }


  for (int x = 0; x < 3; x++) {
    if (ringTravel[x] < 0) {
      ringTravel[x] = 0;
    } else if (ringTravel[x] > 255) {
      ringTravel[x] = 255;
    }

    // ISSUE IS HERE; this will be called twice, right gets updated both times; don't call synced display functions twice
    lensFillIdx(isLeft, x, getFlowerColor(isLeft, x, ringTravel[x]));
    if (isLeft == 0 && effect_option_sync) {
      lensFillIdx(0, x, getFlowerColor(1, x, ringTravel[x]));
    }
  }
}


void displayFlowerSparkle(int isLeft, int travelComplete) {
  //  Serial.print("displayFlowerSparkle(");Serial.print(travelComplete, DEC);Serial.println(")");
  int ringTravel[3] = {0, 0, 0};
  int travelPercentage;
  uint32_t color;

  if (travelComplete <= progressFlower[0][1]) {
    ringTravel[0] = map(travelComplete, progressFlower[0][0], progressFlower[0][1], 0, 255);
    ringTravel[1] = 0;
    ringTravel[2] = 0;
  } else if (travelComplete >= progressFlower[1][0] && travelComplete <= progressFlower[1][1]) {
    ringTravel[0] = 255;
    ringTravel[1] = map(travelComplete, progressFlower[1][0], progressFlower[1][1], 0, 255);
    ringTravel[2] = 0;
  } else if (travelComplete >= progressFlower[2][0]) {
    ringTravel[0] = 255;
    ringTravel[1] = 255;
    ringTravel[2] = map(travelComplete, progressFlower[2][0], progressFlower[2][1], 0, 255);
  }

  for (int x = 0; x < 3; x++) {

    if (ringTravel[x] < 0) {
      ringTravel[x] = 0;
    } else if (ringTravel[x] > 255) {
      ringTravel[x] = 255;
    }

    travelPercentage = map(ringTravel[x], 0, 255, 0, 100);
    eyeColorSparkleIdx(isLeft, x, getFlowerColor(isLeft, x, travelPercentage), travelPercentage);

    if (isLeft == 0 && effect_option_sync) {
      eyeColorSparkleIdx(0, x, getFlowerColor(1, x, travelPercentage), travelPercentage);
    }
  }
}


/*
   Effect update - pulse
*/
void updateEffectPulse(int i) {
  if (effect_pulse_atZero[i] || effect_pulse_atFull[i]) { // Sitting at zero or full
    updatePulsePause(i);
  } else {
    updatePulseTravel(i);
  }

}

void updatePulsePause(int isLeft) {
  if (millis() >= effect_pulse_pause_end[isLeft]) {
    if (effect_pulse_atZero[isLeft]) {
      beginPulseTravelToFull(isLeft);
    } else {
      beginPulseTravelToZero(isLeft);
    }
  } else {
    // Prevent color from stagnating
    displayPulse(isLeft, effect_pulse_atZero[isLeft] ? 0 : 255);
  }
}

void beginPulseTravelToFull(int isLeft) {
  effect_pulse_atZero[isLeft] = 0;
  effect_pulse_atFull[isLeft] = 0;
  //  Serial.println("beginPulseTravelToFull()");
  effect_pulse_travel_start[isLeft] = millis();
  effect_pulse_travel_direction[isLeft] = 1;
}

void beginPulseTravelToZero(int isLeft) {
  effect_pulse_atZero[isLeft] = 0;
  effect_pulse_atFull[isLeft] = 0;
  //  Serial.println("beginPulseTravelToZero()");
  effect_pulse_travel_start[isLeft] = millis();
  effect_pulse_travel_direction[isLeft] = 0;

  //  if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
}

void updatePulseTravel(int isLeft) {
  byte travelComplete = (millis() >= (effect_pulse_travel_start[isLeft] + effect_pulse_travelDuration[isLeft]));
  int travelValue = map(millis(),
                        effect_pulse_travel_start[isLeft],
                        effect_pulse_travel_start[isLeft] + effect_pulse_travelDuration[isLeft],
                        0,
                        255); // make into vars
  if (effect_pulse_travel_direction[isLeft] == 1) {

    if (travelComplete) {
      setPulseToFull(isLeft);
    } else {
      displayPulse(isLeft, travelValue);
    }
  } else {

    if (travelComplete) {
      setPulseToZero(isLeft);
    } else {
      displayPulse(isLeft, 255 - travelValue);
    }
  }
}

void setPulseToFull(int isLeft) {
  effect_pulse_pause_start[isLeft] = millis();
  effect_pulse_pause_end[isLeft]   = (effect_pulse_pause_start[isLeft] + effect_pulse_pauseDurationAtFull[isLeft]);
  effect_pulse_atFull[isLeft] = 1;
  effect_pulse_travel_end[isLeft] = millis();
}

void setPulseToZero(int isLeft) {
  effect_pulse_pause_start[isLeft] = millis();
  effect_pulse_pause_end[isLeft]   = (effect_pulse_pause_start[isLeft] + effect_pulse_pauseDurationAtZero[isLeft]);
  effect_pulse_atZero[isLeft] = 1;
  effect_pulse_travel_end[isLeft] = millis();

  //  if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
}

void displayPulse(int isLeft, int travelComplete) {
  uint32_t color = getColorByVal(isLeft, travelComplete);
  lensFillAll(isLeft, color);
  if (isLeft && effect_option_sync) {
    lensFillAll(0, color);
  }
}

/*
   Effect update - strobe
*/
void updateEffectStrobe(int i) {
  if (effect_strobe_flash_active[i] == 1) { // Flash is on
    if (millis() >= effect_strobe_flash_start[i] + effect_strobe_flashDuration[i]) { // End flash based on duration
      endStrobe(i);
    }
  } else { // Flash is off
    if (millis() >= effect_strobe_flash_end[i] + effect_strobe_flashPause[i]) { // Begin flash based on duration
      beginStrobe(i);
    }
  }
}

void beginStrobe(int isLeft) {
  effect_strobe_flash_active[isLeft]  = 1;
  effect_strobe_flash_start[isLeft]   = millis();
  effect_strobe_flash_color[isLeft] = getColor(isLeft);
  lensFillAll(isLeft, effect_strobe_flash_color[isLeft]);
  if (isLeft && effect_option_sync) {
    lensFillAll(0, effect_strobe_flash_color[isLeft]);
  }
}

void endStrobe(int isLeft) {
  effect_strobe_flash_active[isLeft]  = 0;
  effect_strobe_flash_end[isLeft]     = millis();
  lensFillAll(isLeft, _BLACK);
  if (isLeft && effect_option_sync) {
    lensFillAll(0, _BLACK);
  }
  //  if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
}



/*
   Effect update - strobe ring
*/
void updateEffectStrobeRing(int isLeft) {
  //  Serial.println(i);
  for (int q = 0; q < 3; q++) {
    if (effect_strobeRing_flash_active[isLeft][q] == 1) { // Flash is on
      if (millis() >= effect_strobeRing_flash_start[isLeft][q] + effect_strobeRing_flashDuration[isLeft][q]) { // End flash based on duration
        endStrobeRing(isLeft, q);
      }
    } else { // Flash is off
      if (millis() >= effect_strobeRing_flash_end[isLeft][q] + effect_strobeRing_flashPause[isLeft][q]) { // Begin flash based on duration
        beginStrobeRing(isLeft, q);
      }
    }
  }
}

void beginStrobeRing(int isLeft, int idx) {
  effect_strobeRing_flash_active[isLeft][idx]  = 1;
  effect_strobeRing_flash_start[isLeft][idx]   = millis();
  effect_strobeRing_flash_color[isLeft][idx] = getColor(isLeft);

  lensFillIdx(isLeft, idx, effect_strobeRing_flash_color[isLeft][idx]);
  if (isLeft && effect_option_sync) {
    lensFillIdx(0, idx, effect_strobeRing_flash_color[isLeft][idx]);
  }
}

void endStrobeRing(int isLeft, int idx) {
  effect_strobeRing_flash_active[isLeft][idx]  = 0;
  effect_strobeRing_flash_end[isLeft][idx]     = millis();
  lensFillIdx(isLeft, idx, _BLACK);
  if (isLeft && effect_option_sync) {
    lensFillIdx(0, idx, _BLACK);
  }
  //  if(effect_color_modes[isLeft]) updateRainbowColor(isLeft);
  //  randomizeStrobeRingSettings(isLeft, idx);
}

void randomizeStrobeRingSettings(int isLeft, int idx) {
  effect_strobeRing_flashPause[isLeft][idx] = random(50, 500);
  effect_strobeRing_flashDuration[isLeft][idx] = random(5, 20);
}




/*
   Get Hue value for strip chase
*/
uint32_t getChaseHue() {
  if (_boostActive) {
    switch (getBoostProgress()) {
      case 0:
        tmpHue = map(_boostElapsedTime, 0, _quarterDuration, 0, boostColor);
        break;

      case 1:
        tmpHue = boostColor;
        break;

      case 2:
        tmpHue = map(_boostElapsedTime, _quarterDuration * 3, _boostDuration, boostColor, 0);
        break;
    }
  } else {
    tmpHue = 0;
  }
  return tmpHue;
}

uint32_t getChaseColorByOffset(int diffFromIndex) {
  unsigned char red, green, blue;
  int sat = 200;
  int val = 0;

  if (diffFromIndex == trailLength) {
    hsv2rgb(getChaseHue() + 40, 255, 200, &red, &green, &blue, 200);
    return ring.Color(red, green, blue);
  }

  if (diffFromIndex > trailLength) {
    val = 0;
  } else {
    val = map(diffFromIndex, 0, trailLength, 0, 200);
  }

  hsv2rgb(getChaseHue(), 255, val, &red, &green, &blue, val);
  return ring.Color(red, green, blue);
}



void stripChaseTrail(int activeIndex, long freq, uint32_t color) {
  int diff;
  for (int currIndex = 0; currIndex < strip.numPixels(); currIndex++) {
    diff = getDifferenceFromActiveIndexInLoop(activeIndex, currIndex, 24);
    strip.setPixelColor(currIndex, getChaseColorByOffset(diff));
  }
  strip.show();
}

int getDifferenceFromActiveIndexInLoop(int currIndex, int activeIndex, int loopLength) {
  if (currIndex == activeIndex) return 0;
  if (currIndex < activeIndex) {
    return activeIndex - currIndex;
  } else {
    return (loopLength - currIndex) + activeIndex;
  }
}

void flickerThrustLight(int strength) {
  //  int minStrength = strength * 8;
  //  int maxStrength = strength * 32;
  int randomStr;
  for (uint16_t i = 0; i < ring.numPixels(); i++) {

    if (_boostActive) {
      randomStr = random(32, 255);
      ring.setPixelColor(i, getFlickerColor(randomStr));
    } else {
      randomStr = random(32, _enginePowerLevel * 16);
      ring.setPixelColor(i, getFlickerColor(randomStr));
    }
  }
  ring.show();
}

//uint32_t getThrusterHue() {
//  return
//}

/*
   Get flickering color light color
*/
uint32_t getFlickerColor(int strength) {
  unsigned char red, green, blue;
  if (_boostActive) {
    switch (getBoostProgress()) {
      case 0:
        tmpHue = map(_boostElapsedTime, 0, _quarterDuration, 0, boostColor);
        break;

      case 1:
        tmpHue = boostColor;
        break;

      case 2:
        tmpHue = map(_boostElapsedTime, _quarterDuration * 3, _boostDuration, boostColor, 0);
        break;
    }
    if (tmpHue > 100 && tmpHue < boostColor) tmpHue = boostColor;
  } else {
    tmpHue = 0;
  }
  tmpHue = random(tmpHue, tmpHue + 40);
  hsv2rgb(tmpHue, 255, strength, &red, &green, &blue, strength);
  return ring.Color(red, green, blue);
}


void ringColorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, c);
    ring.show();
    delay(wait);
  }
}


void ringColorSet(uint32_t c) {
  for (uint16_t i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, c);
  }
  ring.show();
}

void stripColorSet(uint32_t c) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


void setAlternatingRingColors(uint32_t color1, uint32_t color2) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, i % 2 ? color1 : color2);
  }
  strip.show();
}

void setHalfRingColors(uint32_t color1, uint32_t color2) {
  for (uint16_t i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, i < (ring.numPixels() / 2) ? color1 : color2);
  }
  ring.show();
}

void setHalfStringColors(uint32_t color1, uint32_t color2) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, i < (strip.numPixels() / 2) ? color1 : color2);
  }
  strip.show();
}




uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}




void initControllerData() {
  if (SPECTRUM_ANALYZER_ACTIVE) {
    readSpectrum();
  }
  readControllerSwitches(1);
  mode = getModePotentiometer();
}

void readControllerData() {

  // Prevent continuous reading of controls
  if (millis() >= (controls_last_time_read + controls_update_interval)) {
    readWiiController();

    readControllerButtons();

    readFobButtons();

    readControllerPotentiometers();

    readControllerSwitches(0);

    //    if (SPECTRUM_ANALYZER_ACTIVE) {
    //      readSpectrum();
    //    }

    controls_last_time_read = millis();
  }
}

void readControllerButtons() {
  // Remote controller - buttons
  for (pos = 0; pos < remote_num_buttons; pos++) {
    switch (pos) {
      case 0:
        remote_button_state[pos] = (digitalRead(PIN_CONTROLLER_BUTTON_PUMP) == LOW) ? 1 : 0;
        //          Serial.print(remote_button_state[pos]);Serial.print(", ");
        break;

      case 1:
        remote_button_state[pos] = (digitalRead(PIN_CONTROLLER_BUTTON_BOOST) == LOW) ? 1 : 0;
        //          Serial.print(remote_button_state[pos]);Serial.print(", ");
        break;
    }

    // Anything triggered by button presses
    if (remote_button_state[pos] != remote_button_last_state[pos]) {
      remote_button_last_debounce_time[pos] = millis();
    }

    if ((millis() - remote_button_last_debounce_time[pos]) < debounce_delay) {
      if (remote_button_state[pos] != remote_button_curr_state[pos]) {
        remote_button_curr_state[pos] = remote_button_state[pos];
        if (remote_button_curr_state[pos]) {
          triggerControllerButton(pos);
        }
      }
    }



  }
}

void readWiiController() {
  classy.update();

  joystick_left_x_raw = classy.leftStickX;
  joystick_left_y_raw = classy.leftStickY;
  joystick_left_x = (joystick_left_x_raw == 32) ? 16 : map(joystick_left_x_raw, 0, 63, 0, 31);
  joystick_left_y = (classy.leftStickY == 32) ? 16 : map(classy.leftStickY, 0, 63, 31, 0);
  joystick_right_x = classy.rightStickX;
  joystick_right_y = (classy.rightStickY == 16) ? 16 : 31 - classy.rightStickY;

  handleJoystickData(joystick_left_x, joystick_left_y, joystick_right_x, joystick_right_y);

  for (pos = 0; pos < num_wii_buttons; pos++) {
    switch (pos) {
      case 0:
        wii_button_state[pos] = (classy.homePressed == true) ? 1 : 0;
        break;
      case 1:
        wii_button_state[pos] = (classy.startPressed == true) ? 1 : 0;
        break;
      case 2:
        wii_button_state[pos] = (classy.selectPressed == true) ? 1 : 0;
        break;

      case 3:
        wii_button_state[pos] = (classy.xPressed == true) ? 1 : 0;
        break;
      case 4:
        wii_button_state[pos] = (classy.yPressed == true) ? 1 : 0;
        break;
      case 5:
        wii_button_state[pos] = (classy.aPressed == true) ? 1 : 0;
        break;
      case 6:
        wii_button_state[pos] = (classy.bPressed == true) ? 1 : 0;
        break;

      case 7:
        wii_button_state[pos] = (classy.upDPressed == true) ? 1 : 0;
        break;
      case 8:
        wii_button_state[pos] = (classy.rightDPressed == true) ? 1 : 0;
        break;
      case 9:
        wii_button_state[pos] = (classy.downDPressed == true) ? 1 : 0;
        break;
      case 10:
        wii_button_state[pos] = (classy.leftDPressed == true) ? 1 : 0;
        break;

      case 11:
        wii_button_state[pos] = (classy.leftShoulderPressed == true) ? 1 : 0;
        break;
      case 12:
        wii_button_state[pos] = (classy.rightShoulderPressed == true) ? 1 : 0;
        break;
      case 13:
        wii_button_state[pos] = (classy.lzPressed == true) ? 1 : 0;
        break;
      case 14:
        wii_button_state[pos] = (classy.rzPressed == true) ? 1 : 0;
        break;
    } // End switch

    if (wii_button_state[pos] != wii_button_last_state[pos]) {
      wii_button_last_debounce_time[pos] = millis();
    }

    if ((millis() - wii_button_last_debounce_time[pos]) < debounce_delay) {

      if (wii_button_state[pos] != wii_button_curr_state[pos]) {
        wii_button_curr_state[pos] = wii_button_state[pos];
        if (wii_button_curr_state[pos]) {
          triggerWiiButtonByIndex(pos);
        }
      }

    }
  }
}


/*
   Trigger actions based on Wii controller
*/
void triggerWiiButtonByIndex(int idx) {
  if (!inCommuterMode()) {
    triggerDefaultWiiButtonByIndex(idx);
  }
  switch (mode) {
    case 1: case 4: // xmas
      //        triggerEngineWiiButtonByIndex(idx);
      break;
    case 2:
      //        triggerRacerWiiButtonByIndex(idx);
      break;
    case 8:
      triggerDevelopmentWiiButtonsByIndex(idx);
      break;
    case 9: // commuting
      triggerCommuterWiiButtonsByIndex(idx);
      break;
  }

}



void toggleCommuterTurnSignal(int isLeft) {
  turn_signal_active = (turn_signal_active == 0) ? 1 : 0;
  turn_signal_isLeft = isLeft;
  if (!turn_signal_active) {
    setCommuterLights();
  } else if (isLeft) {
    lensClear(0);
  } else {
    lensClear(1);
  }
}

void updateTurnSignal() {
  uint32_t color;
  unsigned char red, green, blue;

  if (turn_signal_active) {
    if (millis() >= (turn_signal_lastUpdated + (turn_signal_flashActive ? turn_signal_flash_duration : turn_signal_pause_duration))) {
      turn_signal_lastUpdated = millis();
      turn_signal_flashActive = (turn_signal_flashActive == 0) ? 1 : 0;
      Serial.println(turn_signal_flashActive, DEC);
      if (turn_signal_flashActive) {
        hsv2rgb(
          54,
          255,
          255,
          &red, &green, &blue, effect_color_cycle_maxBrightness);

        color = eye_left.Color(red, green, blue);
      } else {
        color = BLACK;
      }
      lensFillAll(turn_signal_isLeft, color);
    }
  } else {
    setCommuterLights();
  }
}

/*
   Commuter
*/
void triggerCommuterWiiButtonsByIndex(int idx) {

  switch (idx) {

    case 3: // X

      break;
    case 4: // Y

      break;
    case 5: // A

      break;
    case 6: // B

      break;

    case 11: // Left Shoulder
      toggleCommuterTurnSignal(1);

      break;
    case 12: // Right Shoulder
      toggleCommuterTurnSignal(0);

      break;
    case 13: // Left Trigger
      toggleCommuterTurnSignal(1);

      break;
    case 14: // Right Trigger
      toggleCommuterTurnSignal(0);
      break;

  } // End switch
}

/*
   Development mode buttons
*/

void triggerDevelopmentWiiButtonsByIndex(int idx) {

  switch (idx) {
    case 0: // Home

      break;
    case 1: // Start/+

      break;
    case 2: // Select/-

      break;
    case 3: // X

      break;
    case 4: // Y

      break;
    case 5: // A

      break;
    case 6: // B

      break;

    case 7: // D Pad Up

      break;
    case 8: // D Pad Right

      break;
    case 9: // D Pad Down

      break;
    case 10: // D Pad Left

      break;

    case 11: // Left Shoulder
      //      eyeSparkleFadeUp(500, getColor(), true);
      //      eyeSparkleFadeUp(500, getColor(), false);

      break;
    case 12: // Right Shoulder
      //      eyeSparkleFadeDown(500, getColor(), true);
      //      eyeSparkleFadeDown(500, getColor(), false);

      break;
    case 13: // Left Trigger
      eyeSetCrosshair(getColor(1), 1);

      break;
    case 14: // Right Trigger
      eyeSetCrosshair(getColor(0), 0);
      break;

  } // End switch
}

uint32_t getColor(int isLeft) {
  if (BREAKING_THE_LAW_Active) return getPoliceColor(isLeft);
  switch (effect_color_modes[isLeft]) {
    case 0:
      return getColorFromJoysticks();
      break;
    case 1:
      return getColorCycle(isLeft);
      break;

    case 2:
      return getRandomColor(isLeft);
      break;

    case 3:
      return getWhite();
      break;

    case 4:
      return getRainbowColor(isLeft);
      break;
  }
}


void updateRainbowColor(int isLeft) {
  effect_color_rainbow_index[isLeft]++;
  if (effect_color_rainbow_index[isLeft] > 255) effect_color_rainbow_index[isLeft] = 0;
}

uint32_t getRainbowColor(int isLeft) {
  updateRainbowColor(isLeft);
  unsigned char red, green, blue;
  hsv2rgb(
    effect_color_rainbow_index[isLeft],
    255,
    effect_color_cycle_val_current,
    &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_left.Color(red, green, blue);
}

uint32_t getRainbowColorByVal(int isLeft, int val) {
  //  updateRainbowColor(isLeft);
  unsigned char red, green, blue;
  hsv2rgb(
    val,
    255,
    effect_color_cycle_val_current,
    &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_left.Color(red, green, blue);
}

uint32_t getColorByVal(int idx, int val) {
  if (BREAKING_THE_LAW_Active) return getPoliceColorByVal(idx, val);
  if (audio_effect_overrideLights) {
    return getColorFromAudioVal(idx, val);
  }

  // xmas
  //  if (inXmasMode()) {
  //    return getXmasColorVal(idx, 0, val);
  //  }

  switch (effect_color_modes[idx]) {
    case 0:
      return getColorFromJoysticksVal(val);
      break;
    case 1:
      return getColorCycleVal(idx, val);
      break;
    case 2:
      return getRandomColorVal(idx, val);
      break;
    case 3:
      return getWhiteVal(val);
      break;
    case 4:
      //      updateRainbowColor(idx);
      return getRainbowColorByVal(idx, val);
      break;

  }
}
/*
   effect_flower_color_hue[][3]                   = {{0, 0, 0}, {0, 0, 0}};
  int effect_flower_color_val[][3]
*/
uint32_t getFlowerColor(int isLeft, int idx, int val) {
  unsigned char red, green, blue;

  switch (effect_color_modes[isLeft]) {
    case 0:
      setEffectOptions_flowerHue(isLeft, effect_manual_hue_current);
      break;
    case 1:
      updateColorCycle(isLeft);
      setEffectOptions_flowerHue(isLeft, effect_color_cycle_hue_currents[isLeft]);
      break;
    case 2:
      //      return getRandomColorVal(idx, val);
      setEffectOptions_flowerHue(isLeft, getRandomHue(isLeft));
      break;
    case 3:
      return getWhiteVal(val);
      break;
    case 4:
      //      return getRainbowColorByVal(isLeft, val);

      break;

  }

  hsv2rgb(
    effect_flower_color_hue[isLeft][idx],
    255,
    val,
    &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_left.Color(red, green, blue);
}

uint32_t getPoliceColor(int idx) {
  if ((BREAKING_THE_LAW_NumSteps % BREAKING_THE_LAW_Step_Index) != 0) {
    return RED;
  } else {
    return BLUE;
  }
}

uint32_t getPoliceColorByVal(int idx, int val) {
  if ((BREAKING_THE_LAW_NumSteps % BREAKING_THE_LAW_Step_Index) != 0) {
    return eye_left.Color(val, 0, 0);
  } else {
    return eye_left.Color(0, 0, val);
  }
}


void updateXmasColorCycle() {

  //xmas_last_changed[3] = {0,0,0};
  //unsigned long xmas_change_interval[3]

  for (int ringIdx = 0; ringIdx < 3; ringIdx++) {



    if ((millis() - xmas_last_changed[ringIdx]) >= xmas_change_interval[ringIdx]) {
      if (xmas_color_curr[ringIdx]++ >= xmas_color_num) {
        xmas_color_curr[ringIdx] = 0;
      }
      xmas_last_changed[ringIdx] = millis();
    }
  }
}

// Co Co Code for xmas
uint32_t getXmasColor(int isLeft) {
  unsigned char red, green, blue;
  hsv2rgb(
    xmas_hue_red,
    effect_color_cycle_sat_currents[isLeft],
    127,
    &red, &green, &blue, effect_color_cycle_maxBrightness);
  return eye_left.Color(red, green, blue);
}


uint32_t getXmasColorVal(int isLeft, int ringIdx, int val) {
  updateXmasColorCycle();
  unsigned char red, green, blue;
  hsv2rgb(
    xmas_color_curr[ringIdx] == 1 ? xmas_hue_green : xmas_hue_red,
    xmas_color_curr[ringIdx] == 0 ? 0 : 255,
    val,
    &red, &green, &blue, effect_color_cycle_maxBrightness);
  return eye_left.Color(red, green, blue);
}


uint32_t getWhite() {
  unsigned char red, green, blue;
  hsv2rgb(
    0,
    0,
    effect_color_cycle_maxBrightness,
    &red, &green, &blue, effect_color_cycle_maxBrightness); // replacing maxBrightness with val, better?

  return eye_left.Color(red, green, blue);
}

uint32_t getWhiteVal(int val) { // monkey
  unsigned char red, green, blue;
  hsv2rgb(
    0,
    0,
    val,
    &red, &green, &blue, effect_color_cycle_maxBrightness); // replacing maxBrightness with val, better?

  return eye_left.Color(red, green, blue);
}

uint32_t getColorFromAudioVal(int isLeft, int val) {
  unsigned char red, green, blue;
  hsv2rgb(
    audio_effect_hue,
    effect_color_cycle_sat_currents[isLeft],
    val,
    &red, &green, &blue, effect_color_cycle_maxBrightness);
  return eye_left.Color(red, green, blue);
}

uint32_t getColorCycle(int isLeft) {
  unsigned char red, green, blue;
  updateColorCycle(isLeft);

  hsv2rgb(
    effect_color_cycle_hue_currents[isLeft],
    effect_color_cycle_sat_currents[isLeft],
    effect_color_cycle_val_currents[isLeft],
    &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_left.Color(red, green, blue);
}

void updateColorCycle(int isLeft) {
  if (millis() >= (effect_color_cycle_last_updates[isLeft] + effect_color_cycle_step_durations[isLeft])) {
    if (effect_color_cycle_hue_currents[isLeft] >= 255) {
      effect_color_cycle_hue_currents[isLeft] = 0;
    } else {
      effect_color_cycle_hue_currents[isLeft]++;
    }
    effect_color_cycle_last_updates[isLeft] = millis();
  }
}

uint32_t getColorCycleVal(int isLeft, int val) {
  unsigned char red, green, blue;
  updateColorCycle(isLeft);

  hsv2rgb(
    effect_color_cycle_hue_currents[isLeft],
    effect_color_cycle_sat_currents[isLeft],
    val,
    &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_left.Color(red, green, blue);
}


int getRandomHue(int isLeft) {
  if (millis() >= random_color_update_lasts[isLeft] + random_color_update_delays[isLeft]) {
    random_hues[isLeft] = random(0, 255);
    random_hue_indexes[isLeft] = random(0, 13);
    random_val_indexes[isLeft] = random(32, 192);
    random_color_update_lasts[isLeft] = millis();
  }
  return random_hues[isLeft] ;
  //  return random_color_hues[random_hue_index];
}

int getRandomVal() {
  return random_val_index;
}

uint32_t getRandomColor(int isLeft) {
  unsigned char red, green, blue;
  hsv2rgb(
    getRandomHue(isLeft),
    effect_color_cycle_sat_currents[isLeft],
    127,
    &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_left.Color(red, green, blue);
}

uint32_t getRandomColorVal(int isLeft, int val) {
  unsigned char red, green, blue;
  hsv2rgb(
    getRandomHue(isLeft),
    effect_color_cycle_sat_currents[isLeft],
    val,
    &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_left.Color(red, green, blue);
}

uint32_t getColorFromJoysticks() {
  unsigned char red, green, blue;
  joystick_left_x = (joystick_left_x_raw == 32) ? 16 : map(joystick_left_x_raw, 0, 63, 0, 31);
  joystick_left_y = (classy.leftStickY == 32) ? 16 : map(classy.leftStickY, 0, 63, 31, 0);

  int Hue, Sat, Val;

  //  Hue = map(joystick_left_x_raw, 0, 63, 0, 127) + map(joystick_left_y_raw, 0, 63, 0, 128);
  Hue = map(joystick_left_x_raw, 0, 63, 0, 255);
  Sat = 255;
  //  Val = map(joystick_left_y_raw, 0, 63, 15, 255);
  Val = effect_color_cycle_val_current;

  hsv2rgb(effect_manual_hue_current, Sat, Val, &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_right.Color(red, green, blue);
}

uint32_t getColorFromJoysticksVal(int val) {
  unsigned char red, green, blue;
  joystick_left_x = (joystick_left_x_raw == 32) ? 16 : map(joystick_left_x_raw, 0, 63, 0, 31);
  joystick_left_y = (classy.leftStickY == 32) ? 16 : map(classy.leftStickY, 0, 63, 31, 0);

  int Hue, Sat;

  //  Hue = map(joystick_left_x_raw, 0, 63, 0, 127) + map(joystick_left_y_raw, 0, 63, 0, 128);
  Hue = map(joystick_left_x_raw, 0, 63, 0, 255);
  Sat = 255;

  hsv2rgb(effect_manual_hue_current, Sat, val, &red, &green, &blue, effect_color_cycle_maxBrightness);

  return eye_right.Color(red, green, blue);
}


/*
   Triggered all the time regardless of mode
*/
void triggerDefaultWiiButtonByIndex(int idx) {
  switch (idx) {
    case 0: // Home
      toggleRandomizer();
      break;
    case 1: // Start/+
      effectsOn();
      break;
    case 2: // Select/-
      effectsOff();
      break;
    case 3: // X
      //      triggerAudioInMode(0);
      //      triggerAudioEffect(0);
      startBREAKINGTHELAW();
      break;
    case 4: // Y
      //      pressBeatButton();
      pressTempoButton();
      //      triggerAudioEffect(1);
      //      pressBeatButton();
      break;
    case 5: // A
      triggerAudioEffect(1);
      //      triggerAudioInMode(1);
      break;
    case 6: // B
      //      triggerAudioInMode(0);
      triggerAudioEffect(0);
      break;

    case 7: // D Pad Up
      randomizeAllSettings();
      break;
    case 8: // D Pad Right
      audioModeNext();
      break;
    case 9: // D Pad Down
      toggleEffectSync();
      break;
    case 10: // D Pad Left
      audioModePrevious();
      break;

    case 11: // Left Shoulder
      //      colorModePrevious(0);
      if (effect_option_sync) {
        colorModePrevious(1);
      } else {
        colorModeNext(1);
      }
      //      colorModeNext(1);
      break;
    case 12: // Right Shoulder
      colorModeNext(effect_option_sync ? 1 : 0);
      break;
    case 13: // Left Trigger
      if (effect_option_sync) {
        effectModePrevious(1);
      } else {
        effectModeNext(1);
      }

      break;
    case 14: // Right Trigger
      effectModeNext(effect_option_sync ? 1 : 0);
      break;
  } // End switch
}

void colorModeNext(int isLeft) {
  if (effect_color_modes[isLeft] == (num_color_modes - 1)) {
    setNewColorMode(isLeft, 0);
    //    effect_color_modes[isLeft] = 0;
  } else {
    //    effect_color_modes[isLeft]++;
    setNewColorMode(isLeft, effect_color_modes[isLeft] + 1);
  }
  //  if (effect_color_modes[isLeft] == 1) {
  //    effect_color_cycle_hue_currents[isLeft] = map(joystick_left_x_raw, 0, 63, 0, 255);
  //  }
  lcdShowColorMode(isLeft);
}


void colorModePrevious(int isLeft) {
  //  if (effect_color_modes[isLeft] != 0) effect_color_modes[isLeft]--;
  if (effect_color_modes[isLeft] == 0) {
    setNewColorMode(isLeft, (num_color_modes - 1));
  } else {
    setNewColorMode(isLeft, effect_color_modes[isLeft] - 1);
    //    effect_modes[isLeft]++;
  }
  //  setNewColorMode(isLeft)
  lcdShowColorMode(isLeft);
}



void effectModeNext(int isLeft) {
  setRandomizerState(0, 0);
  //  resetEffectModes();
  if (effect_modes[isLeft] == (num_effect_modes - 1)) {
    setNewEffectMode(isLeft, 0);
  } else {
    setNewEffectMode(isLeft, effect_modes[isLeft] + 1);
    //    effect_modes[isLeft]++;
  }
  //  effect_modes[0] = effect_mode;
  //  effect_modes[1] = effect_mode;
  //  lcdShowEffectMode(isLeft);
}


void effectModePrevious(int isLeft) {
  //  resetEffectModes();
  setRandomizerState(0, 0);
  if (effect_modes[isLeft] == 0) {
    setNewEffectMode(isLeft, (num_effect_modes - 1));
    //    effect_modes[isLeft] = num_effect_modes - 1;
  } else {
    setNewEffectMode(isLeft, effect_modes[isLeft] - 1);
    //    effect_modes[isLeft]--;
  }

}


void setNewEffectMode(int isLeft, int newMode) {
  if (newMode == 8) {
    effect_flower_transition_type[isLeft] = 0;
  } else if (newMode == 9) {
    effect_flower_transition_type[isLeft] = 1;
  }
  effect_modes[isLeft] = newMode;
  lcdShowEffectMode(isLeft);
}

void setNewColorMode(int isLeft, int newMode) {
  effect_color_modes[isLeft] = newMode;
}


void resetEffectModes() {
  //  endStrobe(0);
  //  endStrobe(1);
  clearBothLenses();
}

void audioModeNext() {
  if (audio_mode == (num_audio_modes - 1)) {
    audio_mode = 0;
  } else {
    audio_mode++;
  }
  lcdShowAudioMode();
}

void audioModePrevious() {
  if (audio_mode == 0) {
    audio_mode = num_audio_modes - 1;
  } else {
    audio_mode--;
  }
  lcdShowAudioMode();
}


void triggerAudioEffect(int idx) {

  audio_effect_active = 1;
  audio_effect_active_group = audio_mode;
  audio_effect_active_index = idx;
  audio_effect_duration = audio_length_map[audio_mode][idx];
  audio_effect_end = millis() + audio_effect_duration;
  wTrig.trackPlayPoly(audio_wav_map[audio_mode][idx]);

  // Start visual- fix this
  if (true) {
    //    effect_mode = 3;
    //    effect_modes[0] = effect_mode;
    //    effect_modes[1] = effect_mode;
    //    effect_pulse_travelDuration[0] = audio_effect_duration;
    //    effect_pulse_travelDuration[1] = audio_effect_duration;
    //    beginPulseTravelToZero(0);
    //    beginPulseTravelToZero(1);
    audio_effect_overrideLights = 1;
  }
}

void updateAudioEffect() {
  if (audio_effect_active) {
    if (millis() >= audio_effect_end) {
      endAudioEffect();
    } else {
      updateAudioVisualEffects();
    }
  }
}

void updateAudioVisualEffects() {

}

void endAudioEffect() {
  audio_effect_active = 0;
  audio_effect_overrideLights = 0;
  //  normalizeSettings();
}

void triggerAudioInMode(int idx) {
  wTrig.trackPlayPoly(audio_wav_map[audio_mode][idx]);
}


void triggerEngineWiiButtonByIndex(int idx) {


}

void triggerRacerWiiButtonByIndex(int idx) {
  switch (idx) {
    case 0: // Home
      startBREAKINGTHELAW();
      break;

    case 1: // Start/+
      wTrig.trackPlayPoly(AUDIO_AIRBRAKE_OFF);
      break;
    case 2: // Select/-
      wTrig.trackPlayPoly(AUDIO_AIRBRAKE_ON);
      break;
    case 3: // X
      setRacerLoopChoppy(1);
      break;
    case 4: // Y
      setRacerLoopChoppy(2);
      break;
    case 5: // A
      setRacerLoopChoppy(3);
      break;
    case 6: // B
      setRacerLoopChoppy(4);
      break;

    case 7: // D Pad Up
      setRacerLoopSteady(1);
      break;
    case 8: // D Pad Right
      setRacerLoopSteady(2);
      break;
    case 9: // D Pad Down
      setRacerLoopSteady(3);
      break;
    case 10: // D Pad Left
      setRacerLoopSteady(4);
      break;

    case 11: // Left Shoulder
      setRacerLoopVariable(1);

      break;
    case 12: // Right Shoulder
      setRacerLoopVariable(2);
      break;
    case 13: // Left Trigger
      wTrig.trackPlayPoly(AUDIO_BOOST1);
      break;
    case 14: // Right Trigger
      wTrig.trackPlayPoly(AUDIO_BOOST2);
      break;
  } // End switch
}

void readFobButtons() {

  // Remote controller - buttons
  for (pos = 0; pos < fob_num_buttons; pos++) {
    switch (pos) {
      case 0:
        fob_button_state[pos] = (digitalRead(PIN_FOB_BUTTON_A) == HIGH) ? 1 : 0;
        //          Serial.print(fob_button_state[pos]);Serial.print(", ");
        break;

      case 1:
        fob_button_state[pos] = (digitalRead(PIN_FOB_BUTTON_B) == HIGH) ? 1 : 0;
        //          Serial.print(fob_button_state[pos]);Serial.print(", ");
        break;

      case 2:
        fob_button_state[pos] = (digitalRead(PIN_FOB_BUTTON_C) == HIGH) ? 1 : 0;
        //          Serial.print(fob_button_state[pos]);Serial.print(", ");
        break;

      case 3:
        fob_button_state[pos] = (digitalRead(PIN_FOB_BUTTON_D) == HIGH) ? 1 : 0;
        //          Serial.print(fob_button_state[pos]);Serial.print(", ");
    }

    // Anything triggered by button presses
    if (fob_button_state[pos] != fob_button_last_state[pos]) {
      fob_button_last_debounce_time[pos] = millis();
    }

    if ((millis() - fob_button_last_debounce_time[pos]) < debounce_delay) {
      if (fob_button_state[pos] != fob_button_curr_state[pos]) {
        fob_button_curr_state[pos] = fob_button_state[pos];
        if (fob_button_curr_state[pos]) {
          triggerFobButton(pos);
        }
      }
    }



  }
}

uint16_t getModePotentiometer() {
  return (analogRead(PIN_BOX_POT_MODE) + 56) * 9 / 1023;
}

void readControllerPotentiometers() {
  mode_tmp = getModePotentiometer();

  if (mode != mode_tmp) {
    switchMode(mode_tmp);
  }


  // Remote controller - potentiometers
  for (pos = 0; pos < remote_num_pots; pos++) {
    switch (pos) {
      case 0:
        remote_pot_state[pos] = analogRead(PIN_CONTROLLER_POT_THROTTLE);

        throttlePowerLevel = map(remote_pot_state[pos], 320, 530, THROTTLE_MAX, THROTTLE_MIN);
        if (throttlePowerLevel < THROTTLE_MIN) throttlePowerLevel = THROTTLE_MIN;
        if (throttlePowerLevel > THROTTLE_MAX) throttlePowerLevel = THROTTLE_MAX;
        //          Serial.println(throttlePowerLevel);



        break;

      case 1:
        remote_pot_state[pos] = analogRead(PIN_CONTROLLER_POT_HEATER);

        heaterPowerLevel = map(remote_pot_state[pos], 0, 1023, 0, 255);
        if (heaterPowerLevel < 0) heaterPowerLevel = 0;
        if (heaterPowerLevel > 255) heaterPowerLevel = 255;

        if (enableHeater) {
          setHeatLevel(heaterPowerLevel);
        } else {
          setHeatLevel(0);
        }

        //          Serial.print(heaterPowerLevel);Serial.print(", ");
        break;

      case 2:
        remote_pot_state[pos] = analogRead(PIN_CONTROLLER_POT_PUMP);

        pumpTimeLevel = map(remote_pot_state[pos], 0, 1023, PUMP_TIME_MIN, PUMP_TIME_MAX);
        if (pumpTimeLevel < PUMP_TIME_MIN) pumpTimeLevel = PUMP_TIME_MIN;
        if (pumpTimeLevel > PUMP_TIME_MAX) pumpTimeLevel = PUMP_TIME_MAX;
        //          Serial.print(pumpTimeLevel);Serial.print(", ");
        break;

      case 3:
        remote_pot_state[pos] = analogRead(PIN_CONTROLLER_POT_TASK);
        AUDIO_GAIN_TMP = map(remote_pot_state[pos], 0, 1023, AUDIO_GAIN_MIN, AUDIO_GAIN_MAX);
        if (AUDIO_GAIN_TMP != AUDIO_GAIN) { // Ensure this isn't done too often, not sure how well wav trigger handles it
          AUDIO_GAIN = AUDIO_GAIN_TMP;
          wTrig.masterGain(AUDIO_GAIN);
        }

        //          lcdBacklight(map(remote_pot_state[pos], 0, 1023, 128, 157));
        //          Serial.println(map(remote_pot_state[pos], 0, 1023, 128, 157));
        break;
    }
  }
}

void readControllerSwitches(byte isFirstRun) {
  for (pos = 0; pos < remote_num_switches; pos++) {
    switch (pos) {
      case 0:
        remote_switch_state[pos] = (digitalRead(PIN_CONTROLLER_SWITCH_THROTTLE) == HIGH) ? 1 : 0;
        //          Serial.print(remote_switch_state[pos]);Serial.print(", ");
        break;

      case 1:
        remote_switch_state[pos] = (digitalRead(PIN_CONTROLLER_SWITCH_HEATER) == HIGH) ? 1 : 0;
        //          Serial.print(remote_switch_state[pos]);Serial.print(", ");
        break;

      case 2:
        remote_switch_state[pos] = (digitalRead(PIN_CONTROLLER_SWITCH_PUMP) == HIGH) ? 1 : 0;
        //          Serial.println(remote_switch_state[pos]);
        break;


    } // End switch

    // Anything triggered by switch changes
    if (remote_switch_state[pos] != remote_switch_last_state[pos]) {
      remote_switch_last_debounce_time[pos] = millis();
    }

    if ((millis() - remote_switch_last_debounce_time[pos]) < debounce_delay) {
      if (remote_switch_state[pos] != remote_switch_curr_state[pos]) {
        remote_switch_curr_state[pos] = remote_switch_state[pos];
        //          if (remote_switch_curr_state[pos]) {
        triggerControllerSwitch(pos, remote_switch_curr_state[pos], isFirstRun);
        //          }
      }
    }
  }
}


void triggerFobButton(int index) {

  Serial.println(index);

  switch (pos) {
    case 0:
      effectsOn();
      //      wTrig.trackPlayPoly(AUDIO_MOCKERY_HAHA);
      //      eyeSparkleFadeUp(500, getColor(), true);
      //      eyeSparkleFadeUp(500, getColor(), false);
      break;

    case 1:
      effectsOff();
      //      wTrig.trackPlayPoly(AUDIO_HORN_1);
      //      eyeSparkleFadeDown(500, getColor(), true);
      //      eyeSparkleFadeDown(500, getColor(), false);
      break;

    case 2:
      effectModeNext(1);
      //      wTrig.trackPlayPoly(AUDIO_RICK_WUBBA);
      break;

    case 3:
      effectModeNext(0);
      //      wTrig.trackPlayPoly(AUDIO_RICK_PICKLERICK);
      break;
  }
}


void triggerControllerButton(int index) {
  switch (pos) {
    case 0:
      Serial.println("Trigger manual pump");
      startSpray(pumpTimeLevel);
      break;

    case 1:
      Serial.println("Trigger boost");
      triggerEngineBoost();
      break;

    case 2:
      Serial.println("Trigger action 1");
      startBREAKINGTHELAW();
      break;

    case 3:
      Serial.println("Trigger action 2");
  }
}

// Here, add switch for modes
void triggerControllerSwitch(int index, byte switchState, byte isFirstRun) {
  //  Serial.println("triggerControllerSwitch");
  switch (index) {
    case 0: // Throttle
      switch (mode) {
        case 1: case 9: case 4:
          if (!isFirstRun) {
            if (switchState) {
              Serial.println("Start engine");
              triggerEngineStart();
            } else {
              Serial.println("Stop engine");
              triggerEngineStop();
            }
          }
          break;
        case 2:
          if (!isFirstRun) {
            if (switchState) {
              triggerRacerStartup();
            } else {
              triggerRacerShutdown();
            }
          }
          break;
      }

      break;

    case 1: // Heater
      enableHeater = switchState;
      if (switchState) {
        Serial.println("Heater on");
      } else {
        Serial.println("Heater off");
      }
      break;

    case 2: // Pump
      enablePump = switchState;
      if (switchState) {
        Serial.println("Pump on");
      } else {
        Serial.println("Pump off");
      }
      break;


  } // End switch
}

void fullEffectsStop() {
  wTrig.stopAllTracks();
  stopSpray();
}


void startSpray(unsigned long duration) {
  if (_sprayActive) return;
  if (!enablePump) return;
  Serial.println("startSpray()");

  setPumpLevel(255);
  //  if(enable_pump) digitalWrite(PUMPRELAY_PIN, HIGH);

  _sprayDuration = duration;
  _sprayStartTime = millis();
  _sprayActive = 1;
}

void stopSpray() {
  if (!_sprayActive) return;

  setPumpLevel(0);
  //  if(enable_pump) digitalWrite(PUMPRELAY_PIN, LOW);

  _sprayActive = 0;
  Serial.println("stopSpray()");
}

void updateSpray() {
  if (!_sprayActive) return;

  _sprayElapsedTime = millis() - _sprayStartTime;
  if (_sprayElapsedTime >= _sprayDuration) {
    stopSpray();
  }

}

void startBREAKINGTHELAW() {
  BREAKING_THE_LAW_Active = 1;
  BREAKING_THE_LAW_StartTime = millis();
  BREAKING_THE_LAW_Step_StartTime = millis();
  BREAKING_THE_LAW_Step_ElapsedTime = 0;
  BREAKING_THE_LAW_Step_Index = 0;

  stripColorSet(strip.Color(0, 0, 0));
  ringColorSet(strip.Color(0, 0, 0));

  wTrig.trackPlayPoly(5);
  delay(400);

  //  wTrig.stopAllTracks();
  //  wTrig.trackFade(AUDIO_INDEX_AMBIENT, -15, 2000, 0);
  wTrig.trackFade(1, -50, 5, false);
  wTrig.trackFade(AUDIO_INDEX_RUNNING, -50, 5, false);
  wTrig.trackFade(3, -50, 5, false);
  wTrig.trackFade(4, -50, 5, false);


}


void stopBREAKINGTHELAW() {
  BREAKING_THE_LAW_Active = 0;

  stripColorSet(strip.Color(0, 0, 0));
  ringColorSet(strip.Color(0, 0, 0));

  wTrig.trackFade(1, 0, 5, false);
  wTrig.trackFade(AUDIO_INDEX_RUNNING, -12, 5, false);
  wTrig.trackFade(3, -15, 5, false);
  wTrig.trackFade(4, -50, 5, false);
}


void iterateBreakingTheLawStep() {
  BREAKING_THE_LAW_Step_StartTime = millis();
  BREAKING_THE_LAW_Step_Index++;

  if (BREAKING_THE_LAW_Step_Index >= BREAKING_THE_LAW_NumSteps) {
    BREAKING_THE_LAW_Step_Index = 0;
  }
}

void updateBREAKINGTHELAW() {
  if (!BREAKING_THE_LAW_Active) return;

  BREAKING_THE_LAW_ElapsedTime = millis() - BREAKING_THE_LAW_StartTime;
  BREAKING_THE_LAW_Step_ElapsedTime = millis() - BREAKING_THE_LAW_Step_StartTime;



  if (BREAKING_THE_LAW_Step_ElapsedTime >= BREAKING_THE_LAW_Step_Duration) {
    iterateBreakingTheLawStep();

    switch (BREAKING_THE_LAW_Step_Index) {
      case 0:
        setHalfStringColors(REDDEST, BLACK);
        setHalfRingColors(REDDEST, BLACK);
        break;
      case 1:
        setAlternatingRingColors(BLACK, WHITEST);
        setHalfRingColors(WHITEST, WHITE);
        break;
      case 2:
        setHalfStringColors(BLACK, BLUEST);
        setHalfRingColors(BLACK, BLUEST);
        break;
      case 3:
        setAlternatingRingColors(BLACK, BLACK);
        setHalfRingColors(WHITEST, WHITE);
        break;
      case 4:
        setAlternatingRingColors(REDDEST, BLACK);
        setHalfRingColors(REDDEST, BLACK);
        break;
      case 5:
        setAlternatingRingColors(WHITEST, BLACK);
        //        setHalfRingColors(WHITEST, WHITEST);
        break;
      case 6:
        setAlternatingRingColors(BLACK, BLUEST);
        setHalfRingColors(BLACK, BLUEST);
        break;
      case 7:
        setAlternatingRingColors(BLACK, BLACK);
        //        setHalfRingColors(BLACK, BLACK);
        break;
    }

  }


  if (BREAKING_THE_LAW_ElapsedTime >= BREAKING_THE_LAW_Duration) {
    stopBREAKINGTHELAW();
  }



}


void updateBoost() {
  if (!_boostActive) return;

  _boostElapsedTime = millis() - _boostStartTime;
  if (_boostElapsedTime >= _boostDuration) {
    Serial.println("Stop boost");
    stopBoost();
  }

}

/*
   Determine boost 'step' based on time progress in duration

*/
int getBoostProgress() {


  if (_boostElapsedTime <= _quarterDuration) {
    return 0;
  } else if (_boostElapsedTime <= _quarterDuration * 3) {
    return 1;
  } else {
    return 2;
  }


}

void triggerEngineStart() {
  //  lcdSetMessage(2);
  pulse_active = 0;
  if (_engineRunning || _engineShuttingDown) return;
  Serial.println("Starting..");

  fanOn();
  delay(10);
  wTrig.stopAllTracks();
  //  playEngineHum();
  wTrig.trackFade(AUDIO_INDEX_AMBIENT, -10, 3000, 0);

  turbineRunning = true;
  delay(50);
  wTrig.trackPlayPoly(1);
  Serial.println("wTrig.trackPlayPoly(1)");
  _engineRunStartTime = millis();
  _engineRunElapsedTime = 0;
  delay(50);

  //  triggerEngineStepIndex(0);
  ringColorSet(strip.Color(STRENGTH_MIN, 0, 0));

  Spinner.setFrequency(TURBINE_FREQUENCY_START);
  Spinner.setColor(strip.Color(STRENGTH_MIN, 0, 0));
  Spinner.start(1);
  Thruster.setStrength(1);
  Thruster.start(1);
  stripColorSet(strip.Color(63, 63, 0));
  ringColorSet(ring.Color(63, 63, 0));
  delay(5);

  startSpray(pumpTimeLevel);



  _engineRunning = 1;
  _engineStepIndex = 0;
  _engineStopStepIndex = 0;
}

void runEngineShutdown() {
  //  _engineRunElapsedTime = millis() - _engineRunStartTime;
  _engineElapsedTimeSinceStop = millis() - _engineRunStopTime;

  switch (_engineStopStepIndex) {
    case 0: // Shutting down
      _enginePowerLevel = map(_engineElapsedTimeSinceStop, 0, 11000, 16, 1);
      if (_engineElapsedTimeSinceStop >= 11000) {
        _engineStopStepIndex++;
      }
      break;

    case 1: // Done
      // Set idle message
      _engineRunElapsedTime = 0;
      _enginePowerLevel = 0;
      _engineShuttingDown = 0;
      _engineStopStepIndex = 0;

      ringColorSet(strip.Color(0, 0, 0));
      stripColorSet(strip.Color(0, 0, 0));

      Spinner.stop(0);
      delay(5);

      Thruster.stop(0);
      delay(5);

      _engineRunning = 0;
      _engineStepIndex = 0;

      stripColorSet(strip.Color(63, 63, 0));
      ringColorSet(ring.Color(63, 63, 0));

      delay(100);

      stripColorSet(strip.Color(0, 0, 0));
      ringColorSet(strip.Color(0, 0, 0));

      pulse_active = 1;
      break;
  }

}

void runEngine() {
  if (!_engineRunning) {
    updateFanSpeed(false);
    return;
  }
  updateFanSpeed(true);

  _engineRunElapsedTime = millis() - _engineRunStartTime;

  switch (_engineStepIndex) {
    case 0: // Just triggered
      setEngineSpeed(0);
      //

      if (_engineRunElapsedTime >= 182) {
        _engineStepIndex++;
        //        triggerEngineStepIndex(_engineStepIndex);
      }

      break;

    case 1: // Warming up

      // Determine thruster strength
      _enginePowerLevel = map(_engineRunElapsedTime, 182, 3578, 0, 4);
      setEngineSpeed(map(_engineRunElapsedTime, 182, 3578, 0, 63));

      if (_engineRunElapsedTime >= 3578) {
        //        startSpray(500);
        _engineStepIndex++;
        //        triggerEngineStepIndex(_engineStepIndex);
      }

      break;

    case 2: // Ignition

      // Determine thruster strength
      _enginePowerLevel = map(_engineRunElapsedTime, 3578, 5894, 4, 10);
      setEngineSpeed(map(_engineRunElapsedTime, 3578, 5894, 63, 127));

      if (_engineRunElapsedTime >= 5894) {
        startSpray(pumpTimeLevel);
        _engineStepIndex++;
        //        triggerEngineStepIndex(_engineStepIndex);
      }

      break;

    case 3: // Acceleration

      // Determine thruster strength
      _enginePowerLevel = map(_engineRunElapsedTime, 5894, 7528, 10, 16);
      setEngineSpeed(map(_engineRunElapsedTime, 5894, 7528, 127, 192));

      if (_engineRunElapsedTime >= 7528) {
        //        startSpray(500);
        _engineStepIndex++;
        //        triggerEngineStepIndex(_engineStepIndex);
      }

      break;

    case 4: // Running
      //      lcdSetMessage(3);
      setEngineSpeed(throttlePowerLevel);
      if (_engineRunElapsedTime >= 10000) {
        //        startSpray(500);
        wTrig.trackFade(1, -50, 3000, 1);
        Serial.println("wTrig.trackPlayPoly(2)");
        wTrig.trackPlayPoly(AUDIO_INDEX_RUNNING);
        wTrig.trackLoop(AUDIO_INDEX_RUNNING, 1);
        wTrig.trackFade(AUDIO_INDEX_RUNNING, -7, 3000, false);
        _engineStepIndex++;
      }

      break;

    case 5: // Running, muted
      setEngineSpeed(throttlePowerLevel);
      if (_engineRunElapsedTime >= 5888) {
        wTrig.trackFade(AUDIO_INDEX_RUNNING, -30, 3000, false);
        _engineStepIndex++;
      }
      break;

    case 6:
      if (_boostActive) {
        _enginePowerLevel = 255;
        fanPowerLevel = 255;
      } else {
        _enginePowerLevel = map(throttlePowerLevel, 0, 255, 3, 16);
        setEngineSpeed(throttlePowerLevel);
      }

      break;
  }

}


void triggerEngineStop() {
  //  lcdSetMessage(4);
  if (!_engineRunning) return;
  Serial.println("Stopping..");

  fanOff();
  stopSpray();

  _engineRunStopTime = millis();
  _engineShuttingDown = 1;


  // Stop engine run sound, start spindown sound
  wTrig.trackFade(AUDIO_INDEX_RUNNING, -50, 1000, true);
  wTrig.trackPlayPoly(3);
  wTrig.trackFade(3, -15, 1000, false);


}


void triggerEngineBoost() {
  if (!_engineRunning || _engineShuttingDown) return;
  Serial.println("Boosting..");
  startSpray(pumpTimeLevel);
  startBoost(getBoostTime());
  wTrig.trackPlayPoly(getBoostWAVIndex());

  iterateBoostIndex();
}


void startBoost(int RUNTIME) {
  _boostDuration = RUNTIME;
  _quarterDuration = _boostDuration / 4;
  _boostActive = 1;
  _boostStartTime = millis();
  Spinner.setColor(strip.Color(STRENGTH_MIN, 0, STRENGTH_MIN));
  Spinner.setFrequency(0);
}

void stopBoost() {
  _boostActive = 0;
  Spinner.setColor(strip.Color(STRENGTH_MIN, 0, 0));
  Spinner.setFrequency(10);
}


void updateSpinner() {

  if (_engineShuttingDown) {
    Spinner.setFrequency(map(_engineElapsedTimeSinceStop, 0, 11000, 10, 100));
  } else {
    long elapsedMS = millis() - Spinner.getStartMS();
    long elapsed = elapsedMS / 100;

    if (elapsedMS <= 5000) {
      Spinner.setFrequency(map(elapsedMS, 0, 5000, 100, 10));
    } else if (_boostActive) {
      Spinner.setFrequency(0);
    } else {
      Spinner.setFrequency(map(throttlePowerLevel, 0, 255, 16, 0));
    }
  }

  //  stripChase(Spinner.getCurrStep(), Spinner.getFrequency(), Spinner.getColor());
  stripChaseTrail(Spinner.getCurrStep(), Spinner.getFrequency(), Spinner.getColor());

}


void updateXmasPole() {

}

void updatePulse() {
  unsigned char red, green, blue;
  if (millis() - pulseLastUpdate >= pulseUpdateDelay) {
    if (pulse_direction) {
      pulseCurrent++;
      if (pulseCurrent >= pulseHigh) {
        pulse_direction = 0;
      }
    } else {
      pulseCurrent--;
      if (pulseCurrent <= pulseLow) {
        pulse_direction = 1;
      }
    }


    hsv2rgb(pulseCurrent, 255, 255, &red, &green, &blue, 200);

    //
    stripColorSet(strip.Color(red, green, blue));


    pulseLastUpdate = millis();
  }
}


void resetEverything() {
  wTrig.stopAllTracks();
  stripColorSet(strip.Color(0, 0, 0));
  ringColorSet(ring.Color(0, 0, 0));
  resetFunc();

}




/* Boost cycling */

int getBoostWAVIndex() {
  Serial.println(BOOST_INDEXES[BOOST_INDEX]);
  return BOOST_INDEXES[BOOST_INDEX];
}

int getBoostTime() {
  Serial.println(BOOST_TIMES[BOOST_INDEX]);
  return BOOST_TIMES[BOOST_INDEX];
}

void iterateBoostIndex() {
  BOOST_INDEX++;
  if (BOOST_INDEX >= 4) {
    BOOST_INDEX = 0;
  }
}

void playEngineHum() {
  wTrig.trackPlayPoly(AUDIO_INDEX_AMBIENT);
  wTrig.trackLoop(AUDIO_INDEX_AMBIENT, 1);
  wTrig.trackFade(AUDIO_INDEX_AMBIENT, -15, 2000, 0);
}





/*
   Serial LCD control
*/
int lcdBacklight(int brightness)// 128 = OFF, 157 = Fully ON, everything inbetween = varied brightnbess
{
  //this function takes an int between 128-157 and turns the backlight on accordingly
  Serial2.write(0x7C); //NOTE THE DIFFERENT COMMAND FLAG = 124 dec
  Serial2.write(brightness); // any value between 128 and 157 or 0x80 and 0x9D
}


void lcdSetMessage(byte index) {
  lcdClear();
  lcdSetPosition(0, 0);
  switch (index) {
    case 0:
      Serial2.write("Turbine v2.7");
      lcdSetPosition(1, 0);
      Serial2.write("Initializing..");
      break;
    case 1:
      Serial2.write("Idle");

      break;
    case 2:
      Serial2.write("Starting...");
      break;
    case 3:
      Serial2.write("Running");
      break;
    case 4:
      Serial2.write("Shutting down...");
      break;
  }

}


/*
   Row - 0,1
   Column - 0-15
*/
void lcdSetPosition(byte row, byte col) {
  Serial2.write(254);
  byte target = ((row == 0) ? 128 : 192) + col;
  Serial2.write(target);
}


void lcdClear() {
  lcdSetPosition(0, 0);

  Serial2.write("                "); // clear display
  Serial2.write("                ");
}


void lcdSetSettingsBackground() {
  lcdSetPosition(0, 0);

  Serial2.write("    |  |   |    "); // clear display
  Serial2.write("                ");
}

void lcdShowSettings() {
  lcdSetSettingsBackground();
  lcdUpdateAllSettings();

}

void lcdUpdateAllSettings() {
  lcdShowEffectMode(1);
  lcdShowEffectMode(0);
  lcdShowRandom();
  lcdShowSync();
  lcdShowBrightness();
  lcdShowColorMode(1);
  lcdShowColorMode(0);
  lcdShowAudioMode();
}


/*
   Effects
    - Sparkle
    - Strobe
    - Strobe (Rings)
    - Pulse
    - Pulse (Rings)
    - Spin
    - In/out
    - Eyeballs
*/
void lcdShowEffectMode(int isLeft) {
  lcdSetPosition(0, isLeft ? 0 : 12);
  switch (effect_modes[isLeft]) {
    case 0:
      Serial2.write("Sprk");
      break;
    case 1:
      Serial2.write("SpkF");
      break;
    case 2:
      Serial2.write("SpkR");
      break;
    case 3:
      Serial2.write("Stb ");
      break;
    case 4:
      Serial2.write("StbR");
      break;
    case 5:
      Serial2.write("Puls");
      break;
    case 6:
      Serial2.write("PulR");
      break;
    case 7:
      Serial2.write("Spin");
      break;
    case 8:
      Serial2.write("Flwr");
      break;
    case 9:
      Serial2.write("FlwS");
      break;
    case 10:
      Serial2.write("Boom");
      break;
    case 11:
      Serial2.write("Aud1");
      break;
    case 12:
      Serial2.write("Aud2");
      break;
  }

}




void lcdShowRandom() {
  lcdSetPosition(0, 5);
  Serial2.write((randomizer_active == 1) ? "R" : " ");
}

void lcdShowSync() {
  lcdSetPosition(0, 6);
  Serial2.write((effect_option_sync == 1) ? "S" : " ");
}

void lcdShowBrightness() {
  lcdSetPosition(0, 8);
  if (effect_color_cycle_maxBrightness < 10) {
    Serial2.print("  ");
    lcdSetPosition(0, 10);
  } else if (effect_color_cycle_maxBrightness < 100) {
    Serial2.print(" ");
    lcdSetPosition(0, 9);
  }


  Serial2.print(effect_color_cycle_maxBrightness, DEC);
  //  Serial.println(effect_color_cycle_maxBrightness, DEC);
}

void lcdShowColorMode(int isLeft) {
  lcdSetPosition(1, isLeft ? 0 : 13);
  if (inXmasMode()) {
    Serial2.write("Ho");
    return;
  }
  switch (effect_color_modes[isLeft]) {
    case 0:
      Serial2.write("Man");
      break;
    case 1:
      Serial2.write("Cyc");
      break;
    case 2:
      Serial2.write("Rnd");
      break;
    case 3:
      Serial2.write("Wht");
      break;
    case 4:
      Serial2.write("Bow");
      break;

  }
}


void lcdShowAudioMode() {
  lcdSetPosition(1, 4);
  switch (audio_mode) {
    case 0:
      Serial2.write("Horns   "); // 12345678
      break;
    case 1:
      Serial2.write("Lacu/Bat");
      break;
    case 2:
      Serial2.write("Chan/Chn");
      break;
    case 3:
      Serial2.write("Vik/Road");
      break;
    case 4:
      Serial2.write("Haha/Icp");
      break;
    case 5:
      Serial2.write("Hrn/Trol"); // 12345678
      break;
    case 6:
      Serial2.write("Airhorns");
      break;
    case 7:
      Serial2.write("Hrn/Whip");
      break;
    case 8:
      Serial2.write("Rick/Dst");
      break;
    case 9:
      Serial2.write("Car/Ooga");
      break;
    case 10:
      Serial2.write("Trains  ");
      break;
    case 11:
      Serial2.write("KaBm/Trk");
      break;

  }
}


/*
   Heater control
*/
void setHeatLevel(int heatLevel) {
  analogWrite(PIN_MOSFET_HEATER, heatLevel);
}

/*
   Pump control
*/
void setPumpLevel(int pumpLevel) {
  analogWrite(PIN_MOSFET_PUMP, pumpLevel);
}


/*
   Change modes with switch
*/
void switchMode(uint16_t new_mode) {

  mode = new_mode;
  initModeBehavior(mode);
  //  lcdDisplayMode();
}


/*
   Initialize mode, either on startup on when switch is changed
*/
void initModeBehavior(uint16_t init_mode) {
  wTrig.stopAllTracks();
  delay(100);
  setBothFanSpeeds(0);
  stripColorSet(strip.Color(0, 0, 0));
  ringColorSet(strip.Color(0, 0, 0));

  switch (mode) {
    case 0: // Disabled
      break;

    case 1: case 4: // Turbine (default), xmas
      playEngineHum();

      //      lcdSetMessage(1);
      break;

    case 2: // Pod Racer
      //        updateRacer();

      break;

    case 3: // Horse Power

      break;

    case 8: // Development

      break;

    case 9: // Commute
      setCommuterLights();
      break;
  } //end switch(mode)
}

/*
   Set engine speed, incoming is 0-255
*/
void setEngineSpeed(int powerLevel) {

  fanPowerLevel = map(powerLevel, 0, 255, THROTTLE_MIN_RUNNING_NORMAL, THROTTLE_MAX_RUNNING_NORMAL);
  //  Serial.print("Fan Power:");Serial.println(fanPowerLevel);
}

/*
   Update fan based on setting, run with main loop
*/
void updateFanSpeed(byte isActive) {
  if (isActive && enableFan) {
    setScaledFanSpeeds(fanPowerLevel);
  } else {
    setBothFanSpeeds(0);
  }
}



/*
   POD RACER
   MODE 2 - Racer effects
*/

// Main racer behavior control, maintains whatever is triggered by controls/timing
void updateRacer() {
  if (_racerRunning) {
    _racerRunElapsedTime = millis() - _racerRunStartTime;
  }
  //  Serial.println(_racerRunElapsedTime);
  switch (racer_mode_index) {
    case 0: // Idle (impossible situation)
      break;
    case 1: // Starting

      // Update timing settings and check for status iteration
      updateRacerStartup();

      break;
    case 2: // Running

      // Manage switching engine loops, shifting, breaking, etc.

      break;
    case 3: // Shutting down

      // Update timing settings and check for status iteration
      break;
  }
}

// Runs during startup, actions mapped to engine start/accel audio
/*
   unsigned long racer_audio_startup_duration = 11935;
  unsigned long racer_audio_startup_step_0 = 832;
  unsigned long racer_audio_startup_step_1 = 2300;
  unsigned long racer_audio_startup_step_2 = 5000;
  unsigned long racer_audio_startup_step_3 = 7000;
*/
void updateRacerStartup() {
  Serial.println("updateRacerStartup()");
  if (_racerRunElapsedTime >= (racer_audio_startup_duration - 2000)) {
    setRacerMode(2);
    return;
  }
  switch (racer_mode_startup_index) {
    case 0: // Just started, no step yet
      if (_racerRunElapsedTime >= racer_audio_startup_step_0) {
        racer_mode_startup_index = 1;
        // Action 1
        Serial.println("1");
      }
      break;

    case 1:
      if (_racerRunElapsedTime >= racer_audio_startup_step_1) {
        racer_mode_startup_index = 2;
        Serial.println("2");
        // Action 2
      }
      break;

    case 2:
      if (_racerRunElapsedTime >= racer_audio_startup_step_2) {
        racer_mode_startup_index = 3;
        Serial.println("3");
        // Action 3
      }
      break;

    case 3:
      if (_racerRunElapsedTime >= racer_audio_startup_step_3) {
        racer_mode_startup_index = 4;
        Serial.println("4");
        // Action 4
      }
      break;
  }
}

/*
   Racer - set mode
*/
void setRacerMode(uint16_t newMode) {
  racer_mode_index = newMode;
  Serial.print("setRacerMode("); Serial.print(newMode); Serial.println(")");
  switch (newMode) {
    case 0: // Idle

      break;
    case 1: // Starting
      triggerRacerStartup();
      break;
    case 2: // Running
      triggerRacerRunning();
      break;
    case 3: // Shutting down
      triggerRacerShutdown();
      break;
  }
}

void triggerRacerStartup() {
  Serial.println("triggerRacerStartup()");
  if (_racerRunning == 1) return;
  _racerRunning = 1;

  setRacerMode(1);
  racer_mode_startup_index = 0;     // Reset startup sequence index

  fanOn();                          // Activate fan
  wTrig.stopAllTracks();            // Stop any tracks that are playing
  wTrig.trackGain(AUDIO_START, 0);  // Un-mute active loop track
  wTrig.trackPlayPoly(AUDIO_START); // Start playing active loop track

  delay(10);


  _racerRunStartTime = millis();
  _racerRunElapsedTime = 0;

  // Determine update frequency during loop, starts with this value
  Spinner.setFrequency(TURBINE_FREQUENCY_START); // How frequently is spinner updated
  Spinner.setColor(strip.Color(STRENGTH_MIN, 0, 0)); // Color used for spinner
  Spinner.start(1);

  Thruster.setStrength(1);
  Thruster.start(1);

  startSpray(pumpTimeLevel);

  /*
     lcdSetMessage(2);


    _engineRunning = 1;
    _engineStepIndex = 0;
    _engineStopStepIndex = 0;


  */
}

void triggerRacerRunning() {
  Serial.println("triggerRacerRunning()");
  racer_engine_loop_index = AUDIO_LOOP_STEADY1;
  wTrig.stopAllTracks();
  initRacerLoops();
  startRacerLoops();
  initRacerLoopIndex(AUDIO_LOOP_STEADY1);
}

void triggerRacerShutdown() {
  if (!_racerRunning) return;
  stopRacerLoops();
}

void initRacerLoops() {

  // Queue up engine loops
  wTrig.trackLoad(AUDIO_LOOP_CHOP1);
  wTrig.trackLoad(AUDIO_LOOP_CHOP2);
  wTrig.trackLoad(AUDIO_LOOP_CHOP3);
  wTrig.trackLoad(AUDIO_LOOP_CHOP4);

  wTrig.trackLoad(AUDIO_LOOP_STEADY_JET);
  wTrig.trackLoad(AUDIO_LOOP_STEADY1);
  wTrig.trackLoad(AUDIO_LOOP_STEADY2);
  wTrig.trackLoad(AUDIO_LOOP_STEADY3);
  wTrig.trackLoad(AUDIO_LOOP_STEADY4);

  wTrig.trackLoad(AUDIO_LOOP_VARIABLE1);
  wTrig.trackLoad(AUDIO_LOOP_VARIABLE2);

}

void startRacerLoops() {
  wTrig.resumeAllInSync();
  // Set engine loops to lowest volume
  wTrig.trackGain(AUDIO_LOOP_CHOP1, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_CHOP2, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_CHOP3, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_CHOP4, AUDIO_GAIN_MIN);

  wTrig.trackGain(AUDIO_LOOP_STEADY_JET, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_STEADY1, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_STEADY2, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_STEADY3, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_STEADY4, AUDIO_GAIN_MIN);

  wTrig.trackGain(AUDIO_LOOP_VARIABLE1, AUDIO_GAIN_MIN);
  wTrig.trackGain(AUDIO_LOOP_VARIABLE2, AUDIO_GAIN_MIN);

  //  wTrig.trackGain(racer_engine_loop_index, AUDIO_GAIN_MAX);

}

void initRacerLoopIndex(uint16_t newIndex) {
  // Fade in new track
  wTrig.trackFade(newIndex, AUDIO_GAIN_MAX, AUDIO_LOOP_CROSSOVER_TIME, 0);

  racer_engine_loop_index = newIndex;
}

void setRacerLoopIndex(uint16_t newIndex) {
  if (newIndex == racer_engine_loop_index) {
    return;
  }

  // Fade out current track
  wTrig.trackFade(racer_engine_loop_index, AUDIO_GAIN_MIN, AUDIO_LOOP_CROSSOVER_TIME, 0);

  // Fade in new track
  wTrig.trackFade(newIndex, AUDIO_GAIN_MAX, AUDIO_LOOP_CROSSOVER_TIME, 0);

  racer_engine_loop_index = newIndex;
}


void stopRacerLoops() {
  wTrig.trackStop(AUDIO_LOOP_CHOP1);
  wTrig.trackStop(AUDIO_LOOP_CHOP2);
  wTrig.trackStop(AUDIO_LOOP_CHOP3);
  wTrig.trackStop(AUDIO_LOOP_CHOP4);

  wTrig.trackStop(AUDIO_LOOP_STEADY_JET);
  wTrig.trackStop(AUDIO_LOOP_STEADY1);
  wTrig.trackStop(AUDIO_LOOP_STEADY2);
  wTrig.trackStop(AUDIO_LOOP_STEADY3);
  wTrig.trackStop(AUDIO_LOOP_STEADY4);

  wTrig.trackStop(AUDIO_LOOP_VARIABLE1);
  wTrig.trackStop(AUDIO_LOOP_VARIABLE2);
}

void setRacerLoopVariable(int index) {
  switch (index) {
    case 1:
      setRacerLoopIndex(AUDIO_LOOP_VARIABLE1);
      break;
    case 2:
      setRacerLoopIndex(AUDIO_LOOP_VARIABLE2);
      break;
  }
}

void setRacerLoopSteadyJet() {
  setRacerLoopIndex(AUDIO_LOOP_STEADY_JET);
}

void setRacerLoopSteady(int index) {
  switch (index) {
    case 1:
      setRacerLoopIndex(AUDIO_LOOP_STEADY1);
      break;
    case 2:
      setRacerLoopIndex(AUDIO_LOOP_STEADY2);
      break;
    case 3:
      setRacerLoopIndex(AUDIO_LOOP_STEADY3);
      break;
    case 4:
      setRacerLoopIndex(AUDIO_LOOP_STEADY4);
      break;
  }
}

void setRacerLoopChoppy(int index) {
  switch (index) {
    case 1:
      setRacerLoopIndex(AUDIO_LOOP_CHOP1);
      break;
    case 2:
      setRacerLoopIndex(AUDIO_LOOP_CHOP2);
      break;
    case 3:
      setRacerLoopIndex(AUDIO_LOOP_CHOP3);
      break;
    case 4:
      setRacerLoopIndex(AUDIO_LOOP_CHOP4);
      break;
  }
}




void manualSpeedDown(int stickValue) {
  int adjustment = map(stickValue, 15, 0, 1, 10);
  effect_manual_speed[1] -= adjustment;
  effect_manual_speed[0] -= adjustment;

  if (effect_manual_speed[1] < MANUAL_SPARKLE_MIN) {
    effect_manual_speed[1] = MANUAL_SPARKLE_MIN;
  }
  if (effect_manual_speed[0] < MANUAL_SPARKLE_MIN) {
    effect_manual_speed[0] = MANUAL_SPARKLE_MIN;
  }
  processSpeedChange();
}

void manualSpeedUp(int stickValue) {
  int adjustment = map(stickValue, 17, 31, 1, 10);
  effect_manual_speed[1] += adjustment;
  effect_manual_speed[0] += adjustment;
  if (effect_manual_speed[1] > MANUAL_SPEED_MAX) {
    effect_manual_speed[1] = MANUAL_SPEED_MAX;
  }
  if (effect_manual_speed[0] > MANUAL_SPEED_MAX) {
    effect_manual_speed[0] = MANUAL_SPEED_MAX;
  }
  processSpeedChange();
}


void manualSparkleDown(int adjustment) {
  Serial.print("Down: "); Serial.println(adjustment, DEC);
  //  int adjustment = map(stickValue, 31, 0, 1, 10);
  effect_manual_chanceSparkle[1] -= adjustment;
  effect_manual_chanceSparkle[0] -= adjustment;

  if (effect_manual_chanceSparkle[1] < MANUAL_SPARKLE_MIN) {
    effect_manual_chanceSparkle[1] = MANUAL_SPARKLE_MIN;
  }
  if (effect_manual_chanceSparkle[0] < MANUAL_SPARKLE_MIN) {
    effect_manual_chanceSparkle[0] = MANUAL_SPARKLE_MIN;
  }
  processManualSparkleChange();
}

void manualSparkleUp(int adjustment) {
  Serial.print("Up: "); Serial.println(adjustment, DEC);
  //  int adjustment = map(stickValue, 33, 63, 1, 10);
  effect_manual_chanceSparkle[1] += adjustment;
  effect_manual_chanceSparkle[0] += adjustment;
  if (effect_manual_chanceSparkle[1] > MANUAL_SPARKLE_MAX) {
    effect_manual_chanceSparkle[1] = MANUAL_SPARKLE_MAX;
  }
  if (effect_manual_chanceSparkle[0] > MANUAL_SPARKLE_MAX) {
    effect_manual_chanceSparkle[0] = MANUAL_SPARKLE_MAX;
  }
  processManualSparkleChange();
}


void processManualSparkleChange() {
  //  Serial.print(effect_manual_chanceSparkle[1]); Serial.print(", ");
  //  Serial.println(effect_manual_chanceSparkle[0]);
}

//#define BEAT_SPEED_MIN 200
//#define BEAT_SPEED_MAX 1000

void randomizeEffectSpeed() {
  effect_manual_speed[1] = random(MANUAL_SPEED_MIN, MANUAL_SPEED_MAX);
  effect_manual_speed[0] = random(MANUAL_SPEED_MIN, MANUAL_SPEED_MAX);
  processSpeedChange();
}

void applySpeedToEffects(int newSpeed) {
  effect_manual_speed[1] = newSpeed;
  effect_manual_speed[0] = newSpeed;
  processSpeedChange();
}


void processSpeedChange() {
  int sparkleDuration[2] = {0, 0};
  int travelDuration[2] = {0, 0};
  int pulseDuration[2] = {0, 0};
  int bloomTravelDuration[2] = {0, 0};
  int strobeFlashDuration[2] = {0, 0};
  int strobePauseDuration[2] = {0, 0};

  int spinTravelDuration[2] = {0, 0};
  int spinDirection[2]      = {0, 0};

  int pauseAtFull[2] = {0, 0};
  int pauseAtZero[2] = {0, 0};

  int ringTravelDuration[][3] = {{0, 0}, {0, 0}, {0, 0}};
  int ringPauseDuration[][3] = {{0, 0}, {0, 0}, {0, 0}};
  int ringPauseAtFull[][3] = {{0, 0}, {0, 0}, {0, 0}};
  int ringPauseAtZero[][3] = {{0, 0}, {0, 0}, {0, 0}};

  //int clockwise = (effect_spin_travel_direction[isLeft][idx] == 0);

  for (int isLeft = 1; isLeft > -1; isLeft--) {

    sparkleDuration[isLeft] = map(
                                effect_manual_speed[isLeft],
                                MANUAL_SPEED_MIN,
                                MANUAL_SPEED_MAX,
                                EFFECT_OPTION_SPARKLE_DURATION_MAX,
                                EFFECT_OPTION_SPARKLE_DURATION_MIN
                              );

    travelDuration[isLeft] = map(
                               effect_manual_speed[isLeft],
                               MANUAL_SPEED_MIN,
                               MANUAL_SPEED_MAX,
                               EFFECT_OPTION_SPIN_DURATION_MAX,
                               EFFECT_OPTION_SPIN_DURATION_MIN
                             );

    if (effect_manual_speed[isLeft] >= (MANUAL_SPEED_MAX / 2)) {
      spinDirection[isLeft] = isLeft ? 1 : 0;
      spinTravelDuration[isLeft] = map(
                                     effect_manual_speed[isLeft],
                                     (MANUAL_SPEED_MAX / 2),
                                     MANUAL_SPEED_MAX,
                                     EFFECT_OPTION_SPIN_DURATION_MAX,
                                     EFFECT_OPTION_SPIN_DURATION_MIN
                                   );
    } else {
      spinDirection[isLeft] = isLeft ? 0 : 1;
      spinTravelDuration[isLeft] = map(
                                     effect_manual_speed[isLeft],
                                     MANUAL_SPEED_MIN,
                                     (MANUAL_SPEED_MAX / 2) - 1,
                                     EFFECT_OPTION_SPIN_DURATION_MIN,
                                     EFFECT_OPTION_SPIN_DURATION_MAX
                                   );
    }



    pulseDuration[isLeft] = map(
                              effect_manual_speed[isLeft],
                              MANUAL_SPEED_MIN,
                              MANUAL_SPEED_MAX,
                              EFFECT_OPTION_PULSE_DURATION_MAX,
                              EFFECT_OPTION_PULSE_DURATION_MIN
                            );

    bloomTravelDuration[isLeft] = map(
                                    effect_manual_speed[isLeft],
                                    MANUAL_SPEED_MIN,
                                    MANUAL_SPEED_MAX,
                                    EFFECT_OPTION_BLOOM_DURATION_MAX,
                                    EFFECT_OPTION_BLOOM_DURATION_MIN
                                  );

    strobeFlashDuration[isLeft] = map(
                                    effect_manual_speed[isLeft],
                                    MANUAL_SPEED_MIN,
                                    MANUAL_SPEED_MAX,
                                    EFFECT_OPTION_STROBE_FLASH_DURATION_MAX,
                                    EFFECT_OPTION_STROBE_FLASH_DURATION_MIN
                                  );

    strobePauseDuration[isLeft] = map(
                                    effect_manual_speed[isLeft],
                                    MANUAL_SPEED_MIN,
                                    MANUAL_SPEED_MAX,
                                    EFFECT_OPTION_STROBE_PAUSE_DURATION_MAX,
                                    EFFECT_OPTION_STROBE_PAUSE_DURATION_MIN
                                  );

    effect_sparkle_eventDuration[isLeft] = sparkleDuration[isLeft];

    effect_strobe_flashDuration[isLeft] = strobeFlashDuration[isLeft];

    effect_strobe_flashPause[isLeft]    = strobePauseDuration[isLeft];

    effect_sparkleFade_travelDuration[isLeft] = travelDuration[isLeft];
    //    effect_sparkleFade_pauseDurationAtFull[isLeft] = 0;
    //    effect_sparkleFade_pauseDurationAtZero[isLeft] = 0;

    effect_pulse_travelDuration[isLeft] = pulseDuration[isLeft];
    //    effect_pulse_pauseDurationAtFull[isLeft] = pauseAtFull[isLeft];
    //    effect_pulse_pauseDurationAtZero[isLeft] = pauseAtZero[isLeft];


    effect_flower_travelDuration[isLeft] = bloomTravelDuration[isLeft];
    //    effect_flower_pauseDurationAtFull[isLeft] = 0;
    //    effect_flower_pauseDurationAtZero[isLeft] = 0;

    effect_boom_travelDuration[isLeft] = bloomTravelDuration[isLeft];

    for (int ringIdx = 0; ringIdx < 3; ringIdx++) {
      effect_sparkleFadeRing_travelDuration[isLeft][ringIdx]    = travelDuration[isLeft];
      effect_pulseRing_travelDuration[isLeft][ringIdx]          = pulseDuration[isLeft];
      effect_sparkleFadeRing_pauseDurationAtFull[isLeft][ringIdx] = getRingPauseDuration(isLeft, ringIdx, travelDuration[isLeft], 1);
      effect_sparkleFadeRing_pauseDurationAtZero[isLeft][ringIdx] = getRingPauseDuration(isLeft, ringIdx, travelDuration[isLeft], 0);
      effect_pulseRing_pauseDurationAtFull[isLeft][ringIdx]       = getRingPauseDuration(isLeft, ringIdx, travelDuration[isLeft], 1);
      effect_pulseRing_pauseDurationAtZero[isLeft][ringIdx]       = getRingPauseDuration(isLeft, ringIdx, travelDuration[isLeft], 0);
      effect_strobeRing_flashDuration[isLeft][ringIdx]          = strobeFlashDuration[isLeft];
      effect_strobeRing_flashPause[isLeft][ringIdx]             = strobePauseDuration[isLeft];

      effect_spin_travelDuration[isLeft][ringIdx]   = ringIdx == 1 ? (spinTravelDuration[isLeft] * 2) : spinTravelDuration[isLeft];
      effect_spin_travel_direction[isLeft][ringIdx] = spinDirection[isLeft];

      ringTravelDuration[isLeft][ringIdx] = getRingTravelDuration(isLeft, ringIdx, travelDuration[isLeft], 1);
      ringPauseDuration[isLeft][ringIdx]  = getRingPauseDuration(isLeft, ringIdx, travelDuration[isLeft], 1);
      ringPauseAtFull[isLeft][ringIdx]    = getRingPauseDuration(isLeft, ringIdx, travelDuration[isLeft], 1);
      ringPauseAtZero[isLeft][ringIdx]    = getRingPauseDuration(isLeft, ringIdx, travelDuration[isLeft], 0);

    }
  }

}


int getRingTravelDuration(int isLeft, int ringIdx, int value, int isFull) {
  return value / ringIdx;
}

int getRingPauseDuration(int isLeft, int ringIdx, int value, int isFull) {
  return value / ringIdx;
}



void manualHueDown(int adjust) {
  effect_manual_hue_current -= map(adjust, 31, 0, 1, 20);
  if (effect_manual_hue_current < 0) {
    effect_manual_hue_current = 255 + effect_manual_hue_current;
  }
}

void manualHueUp(int adjust) {
  effect_manual_hue_current += map(adjust, 33, 63, 1, 20);
  if (effect_manual_hue_current > 255) {
    effect_manual_hue_current -= 255;
  }
  //  setEffectOptionManualHue(effect_manual_hue_current);
  //  Serial.println(effect_manual_hue_current);
}

void handleJoystickData(byte joy_left_x, byte joy_left_y, byte joy_right_x, byte joy_right_y) {
  if (millis() >= (joystick_action_lastUpdate + joystick_action_update_interval)) {
    joystick_action_lastUpdate = millis();

    if (joystick_left_x_raw != 32) {
      if (joystick_left_x_raw < 32) {
        manualHueDown(joystick_left_x_raw);
      } else {
        manualHueUp(joystick_left_x_raw);
      }
    }

    if (joy_right_x != 16) {
      if (joy_right_x < 16) {
        manualSpeedDown(joy_right_x);
      } else if (joy_right_x > 16) {
        manualSpeedUp(joy_right_x);
      }
    }

    if (joystick_left_y_raw != 32) {
      if (joystick_left_y_raw < 32) {
        brightnessDown(map(joystick_left_y_raw, 31, 0, 1, 10));
      } else {
        brightnessUp(map(joystick_left_y_raw, 33, 63, 1, 10));
      }
    }

    if (joy_right_y != 16) {
      if (joy_right_y < 16) {
        manualSparkleUp(map(joystick_right_y, 15, 0, 1, 10));
      } else if (joy_right_y > 16) {
        manualSparkleDown(map(joystick_right_y, 17, 31, 1, 10));
      }

      if (inCommuterMode()) setCommuterLights();
    }
  }
}

void brightnessUp(int val) {
  effect_color_cycle_maxBrightness += val;
  if (effect_color_cycle_maxBrightness > 255) {
    effect_color_cycle_maxBrightness = 255;
  }
  lcdShowBrightness();
}

void brightnessDown(int val) {
  effect_color_cycle_maxBrightness -= val;
  if (effect_color_cycle_maxBrightness < 5) {
    effect_color_cycle_maxBrightness = 5;
  }
  lcdShowBrightness();
}

void effectsOn() {
  effects_active = 1;
}

void effectsOff() {
  effects_active = 0;
  lensClear(0);
  lensClear(1);
}




void resetBeatSampleData() {
  //  beat_sample_count = 0;
  beat_sample_data_first = millis();
  beat_sample_currNum = 0;
  beat_last_sampled = millis(); // Redundant when called from pressTempoButton()
  beat_sample_data_first = millis();
  for (int x = 0; x < beat_sample_max; x++) {
    beat_sample_data[x] = 0;
  }
}

void pressTempoButton() {
  if (beat_sample_data_first == 0 || (millis() >= (beat_last_sampled + beat_timeout))) {
    resetBeatSampleData();
  } else if (beat_sample_currNum < beat_sample_max) { // First 10 beats simply populate array, no need for updateTempoArray()
    beat_sample_data[beat_sample_currNum] = millis();
    beat_sample_currNum++;

    if (beat_sample_currNum >= beat_sample_min) {
      calculateTempo();
    }

  } else {
    updateTempoArray();
    calculateTempo();
  }
  beat_last_sampled = millis();
}

void updateTempoArray() {
  int q;
  beat_sample_data_first = beat_sample_data[0];
  for (q = 0; q < beat_sample_max; q++) {
    if (q != (beat_sample_max - 1)) {
      beat_sample_data[q] = beat_sample_data[q + 1];
    } else {
      beat_sample_data[q] = millis();
    }
  }
}

void calculateTempo() {
  //  Serial.println(beat_sample_currNum, DEC);
  int x;
  unsigned long total = 0;
  for (x = 0; x < beat_sample_currNum; x++ ) {
    total += (beat_sample_data[x] - (x == 0 ? beat_sample_data_first : beat_sample_data[x - 1]));
  }
  current_tempo_value = (total / beat_sample_currNum);
  current_tempo_bpm = (60000 / current_tempo_value);
  //  Serial.print(current_tempo_value);Serial.print(", ");
  //  Serial.println(current_tempo_bpm);
  updateEffectsWithTempo();
}


void calculateBeatData() {
  unsigned long sample_total = 0;
  for (int x = 1; x < beat_sample_min; x++) {
    sample_total += (beat_sample_data[x] - beat_sample_data[x - 1]);
  }
  beat_output = sample_total / 4;
  //  Serial.println(beat_output);
}




void readSpectrum(void) {
  if (millis() >= (spectrum_data_lastRead + spectrum_data_read_interval)) {
    spectrum_data_lastRead = millis();

    //reset the data
    digitalWrite(SPECTRUMSHIELD_PIN_RESET, HIGH);
    digitalWrite(SPECTRUMSHIELD_PIN_RESET, LOW);

    //loop thru all 7 bands
    for (int band = 0; band < 7; band++) {
      digitalWrite(SPECTRUMSHIELD_PIN_STROBE, LOW); // go to the next band
      delayMicroseconds(50); //gather some data
      left[band] = getBandValue(analogRead(SPECTRUMSHIELD_PIN_LEFT)); // store left band reading
      right[band] = getBandValue(analogRead(SPECTRUMSHIELD_PIN_RIGHT)); // store right band reading
      digitalWrite(SPECTRUMSHIELD_PIN_STROBE, HIGH); // reset the strobe pin
    }
    displaySpectrumData();
    updateTransmitDataWithSound();
  }
}

/*
   Add spectrum analyzed sound data to transmission data
*/
void updateTransmitDataWithSound() {
  for (int band = 0; band < 7; band++) {
    lightData.left[band] = map(left[band], 0, 1023, 0, 255);
    lightData.right[band] = map(right[band], 0, 1023, 0, 255);
  }
}



int getBandValue(int analogValue) {
  return (analogValue >= MIN_AUDIO_INPUT) ? map(analogValue, MIN_AUDIO_INPUT, 1023, 0, 1023) : 0;
}


void displaySpectrumData() {
  int i;
  for (i = 0; i < 7; i++) {
    printSpectrumLeft(i);
  }
  for (i = 0; i < 7; i++) {
    printSpectrumRight(i);
  }
}


void printSpectrumLeft(int index) {
  if (left[index] < 10) {
    Serial.print("   ");
  } else if (left[index] < 100) {
    Serial.print("  ");
  } else if (left[index] < 1000) {
    Serial.print(" ");
  }
  Serial.print(left[index], DEC);
  if (index == 6) {
    Serial.print(" - ");
  } else {
    Serial.print(",");
  }
}

void printSpectrumRight(int index) {
  if (right[index] < 10) {
    Serial.print("   ");
  } else if (right[index] < 100) {
    Serial.print("  ");
  } else if (right[index] < 1000) {
    Serial.print(" ");
  }

  if (index == 6) {
    Serial.println(right[index], DEC);
  } else {
    Serial.print(right[index], DEC);
    Serial.print(",");
  }
}



/*
   Control both fans
   0 -255
*/
void setBothFanSpeeds(int fanSpeed) {
  setSpeedFan1(fanSpeed);
  setSpeedFan2(fanSpeed);
}



/*
   Control fan 1
   0 -255
*/
void setSpeedFan1(int fanSpeed) {
  analogWrite(PIN_MOSFET_FAN1, fanSpeed);
}


/*
   Control fan 2
   0 -255
*/
void setSpeedFan2(int fanSpeed) {
  analogWrite(PIN_MOSFET_FAN2, fanSpeed);
}


/*
   Control both fans in sequence
   0 -255
*/
void setScaledFanSpeeds(int overallSpeed) {

  int speed1;
  int speed2;

  if (overallSpeed <= 63) {
    speed1 = map(overallSpeed, 0, 63, 0, 255);
    speed2 = 0;
  } else {
    speed1 = 255;
    speed2 = map(overallSpeed, 64, 255, 0, 255);
  }
  setSpeedFan1(speed1);
  setSpeedFan2(speed2);
}




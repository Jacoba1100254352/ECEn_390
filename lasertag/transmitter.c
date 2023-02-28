#include "buttons.h"
#include "mio.h"
#include "switches.h"
#include <stdio.h>
#include "utils.h"

// Uncomment for debug prints
// #define DEBUG

#if defined(DEBUG)
#include "xil_printf.h"
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else 
#include "transmitter.h"
#include "filter.h"
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0

#define PLAYER_1 0
#define PLAYER_2 1
#define PLAYER_3 2
#define PLAYER_4 3
#define PLAYER_5 4
#define PLAYER_6 5
#define PLAYER_7 6
#define PLAYER_8 7
#define PLAYER_9 8
#define PLAYER_10 9
#define TRANSMITTER_OUTPUT_PIN 13
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
#define PULSE_LENGTH 200
#define FIFTY_PERCENT_DUTY_CYCLE 1 / 2
#define RESET 0
#define TRANSMITTER_WAIT_IN_MS 300

#define Wait_for_startFlag_st_MSG "In wait_for_startFlag_st\n"
#define Low_st_MSG "In low_st\n"
#define High_st_MSG "In high_st\n"

// States for the controller state machine.
volatile enum transmitter_st_t {
  wait_for_startFlag_st,   // wait here till the start flag is raised
  low_st,                  // outputs the low signal 0
  high_st                  // outputs the high signal 1
} current_State = wait_for_startFlag_st; // start in init_st

volatile static bool firstPass;
volatile static uint32_t frequency_number;
volatile static bool startFlag;
volatile static bool runContinuous;
volatile static uint32_t pulse_cnt;
volatile static uint32_t freq_cnt;
volatile static uint64_t counter;

// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h

// Standard init function.
void transmitter_init() {
  mio_init(false); // false disables any debug printing if there is a system
                   // failure during init.
  mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN); // Configure the signal direction
                                              // of the pin to be an output.
  mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
  firstPass = true;
  frequency_number = 0;
  startFlag = false;
  runContinuous = false;
  pulse_cnt = 0;
  freq_cnt = 0;
  counter = 0;
}

// This is a debug state print routine. It will print the names of the states
// each time tick() is called. It only prints states if they are different than
// the previous state.
static void debugStatePrint() {
  static enum transmitter_st_t previousState = wait_for_startFlag_st; // Start it out different from the currentState
  if (previousState != current_State) {
    previousState = current_State;
    switch (current_State) {
    case wait_for_startFlag_st:
      printf(Wait_for_startFlag_st_MSG);
      fflush(stdout);
      break;
    case low_st:
      printf(Low_st_MSG);
      fflush(stdout);
      break;
    case high_st:
      printf(High_st_MSG);
      fflush(stdout);
      break;
    }
  }
}

// Standard tick function.
void transmitter_tick() {
  debugStatePrint();

    // Perform state update next
  switch (current_State) {
    case wait_for_startFlag_st:
      if (startFlag)
        current_State = low_st;
      else {
        mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
        current_State = wait_for_startFlag_st;
      }
      break;
      
    case low_st:
      if (pulse_cnt < PULSE_LENGTH) {
        if (freq_cnt >= (frequency_number * FIFTY_PERCENT_DUTY_CYCLE)) {
          mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE);
          freq_cnt = RESET;
          current_State = high_st;
        } else
          current_State = low_st;
      } else {
        pulse_cnt = RESET;
        freq_cnt = RESET;
        startFlag = false;
        mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
        current_State = wait_for_startFlag_st;
      }
    break;
  case high_st:
    if (pulse_cnt < PULSE_LENGTH) {
      if (freq_cnt >= (frequency_number * FIFTY_PERCENT_DUTY_CYCLE)) {
        mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
        freq_cnt = RESET;
        current_State = low_st;
      } else
        current_State = high_st;
    } else {
      pulse_cnt = RESET;
      freq_cnt = RESET;
        startFlag = false;
        mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
        current_State = wait_for_startFlag_st;
    }
    break;

  default:
    printf("transmitter_tick state update: hit default\n\r");
    break;
  }


  // Perform state action first
  switch (current_State) {
    case wait_for_startFlag_st:
      break;

    case low_st:
    case high_st:
      pulse_cnt++;
      freq_cnt++;
      break;

    default:
      printf("transmitter_tick state action: hit default\n\r");
      break;
  }
}

// Activate the transmitter.
void transmitter_run() { startFlag = true; }

// Returns true if the transmitter is still running.
bool transmitter_running() { return startFlag; }

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
  frequency_number = filter_frequencyTickTable[frequencyNumber];
  
}

// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() { return frequency_number; }

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, it
// transmits one burst and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until a 200 ms burst is complete. NOTE: while running continuously,
// the transmitter will only change frequencies in between 200 ms bursts.
void transmitter_setContinuousMode(bool continuousModeFlag) {
  runContinuous = continuousModeFlag;
}

/******************************************************************************
***** Test Functions
******************************************************************************/

// Prints out the clock waveform to stdio. Terminates when BTN3 is pressed.
// Does not use interrupts, but calls the tick function in a loop.
// Prints out one line of 1s and 0s that represent one period of the clock signal, in terms of ticks.
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
#define BOUNCE_DELAY 5
void transmitter_runTest() {
  printf("starting transmitter_runTest()\n");
  mio_init(false);
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  transmitter_init();                                     // init the transmitter.
  //transmitter_enableTestMode();                           // Prints diagnostics to stdio.
  while (!(buttons_read() & BUTTONS_BTN3_MASK)) {         // Run continuously until BTN1 is pressed.
    uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
    transmitter_run();                                    // Start the transmitter.
    while (transmitter_running()) {                       // Keep ticking until it is done.
      transmitter_tick();                                 // tick.
      utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
    }
    printf("completed one test period.\n");
  }
  //transmitter_disableTestMode();
  do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
  printf("exiting transmitter_runTest()\n");
}

// Tests the transmitter in non-continuous mode.
// The test runs until BTN3 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestNoncontinuous() {
  printf("starting transmitter_runTestNoncontinuous()\n");
  mio_init(false);
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  transmitter_init();  
  transmitter_setContinuousMode(false);
  while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
    uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
    transmitter_run();
    while (transmitter_running()) {                       // Keep ticking until it is done.
      transmitter_tick();                                 // tick.
      utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
    }
  }
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    printf("exiting transmitter_runTestNoncontinuous()\n");
}

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes in the slide switches.
// Test runs until BTN3 is pressed.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestContinuous() {
  printf("starting transmitter_runTestNoncontinuous()\n");
  mio_init(false);
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  transmitter_init();  
  transmitter_setContinuousMode(true);
  while (!(buttons_read() & BUTTONS_BTN3_MASK)) {
    uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
    transmitter_run();
    while (transmitter_running()) {                       // Keep ticking until it is done.
      transmitter_tick();                                 // tick.
    }
  }
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    printf("exiting transmitter_runTestNoncontinuous()\n");
}
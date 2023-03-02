#include "hitLedTimer.h"
#include "buttons.h"
#include "stdbool.h"
#include "stdio.h"
#include "mio.h"
#include "leds.h"
#include "utils.h"

// Uncomment for debug prints
// #define DEBUG
 
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h"
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif


/******************
*   DEFINITIONS   *
******************/

#define LED_ON 1
#define LED_OFF 0
#define STATE_UPDATE_ERR_MSG "Error in state update"
#define STATE_ACTION_ERR_MSG "Error in state action"
#define LED_ON_ST_MSG "In LED_ON_ST"
#define LED_OFF_ST_MSG "In LED_OFF_ST"
#define BOUNCE_DELAY 5


/********************************
*   GLOBAL VOLATILE VARIABLES   *
********************************/

volatile static uint64_t timer;
volatile static bool timer_enable;
volatile static bool timer_start;

// States for the controller state machine.
volatile static enum hitLedTimer_st_t {
	LED_ON_ST,
    LED_OFF_ST
} currentState = LED_OFF_ST;


/****************
*   FUNCTIONS   *
****************/

// Need to init things.
void hitLedTimer_init() {
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.
    leds_init(false);
    buttons_init();
    timer = 0;
    timer_enable = true; // Unsure if true or false...
    timer_start = false;
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum hitLedTimer_st_t previousState = LED_ON_ST; // Start it out different from the currentState
  if (previousState != currentState) { // only print if the state has changed
    previousState = currentState;     // update previous state
    switch(currentState) {            // print message based on state
      case LED_ON_ST:
        printf(LED_ON_ST_MSG);
        break;
      case LED_OFF_ST:
        printf(LED_OFF_ST_MSG);
        break;
     }
  }
}

// Standard tick function.
void hitLedTimer_tick() {
  //debugStatePrint();
  
      // Perform state update first.
  switch(currentState) {
    case LED_ON_ST:
        if (timer >= HIT_LED_TIMER_EXPIRE_VALUE) {
            timer = 0;
            hitLedTimer_turnLedOff();
            currentState = LED_OFF_ST;
        }
      break;
    case LED_OFF_ST:
        if (timer_start) {
            hitLedTimer_turnLedOn();
            currentState = LED_ON_ST;
        }
      break;
    default:
      printf(STATE_UPDATE_ERR_MSG);
      break;
  }
  
  // Perform state action next.
  switch(currentState) {
    case LED_ON_ST:
        timer++;
      break;
    case LED_OFF_ST:
      break;
     default:
      printf(STATE_ACTION_ERR_MSG);
      break;
  }  

}

// Calling this starts the timer.
void hitLedTimer_start() {
    timer_start = timer_enable;
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return (timer_start & timer_enable);
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_ON);
    leds_write(LED_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_OFF);
    leds_write(LED_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable() {
    timer_enable = false;
}

// Enables the hitLedTimer.
void hitLedTimer_enable() {
    timer_enable = true;
}

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {
  printf("starting HitLED timer test\n");
  while(!(buttons_read() & BUTTONS_BTN3_MASK)) {
    hitLedTimer_start();
    while(hitLedTimer_running())
    utils_msDelay(300);
  }
  do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
}
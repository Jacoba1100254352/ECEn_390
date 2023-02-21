#include "hitLedTimer.h"
#include "mio.h"
#include "leds.h"

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

#define HIT_LED_MIO_PIN 11
#define LED_ON 1
#define LED_OFF 0
#define TIMER_CYCLES 50000 // Cycles for .5s
#define STATE_UPDATE_ERR_MSG "Error in state update"
#define STATE_ACTION_ERR_MSG "Error in state action"
#define LED_ON_ST_MSG "In LED_ON_ST"
#define LED_OFF_ST_MSG "In LED_OFF_ST"

volatile static uint64_t timer = 0;
volatile static bool timer_enable;
volatile static bool timer_start;

// States for the controller state machine.
volatile enum hitLedTimer_st_t {
	LED_ON_ST,
    LED_OFF_ST
};
volatile static enum hitLedTimer_st_t currentState;

// Need to init things.
void hitLedTimer_init() {
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(HIT_LED_MIO_PIN);  // Configure the signal direction of the pin to be an output.
    currentState = LED_OFF_ST;
    leds_init(false);
    timer_enable = true; // Unsure if true or false...
    timer_start = false;
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum hitLedTimer_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
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
      // Perform state update first.
  switch(currentState) {
    case LED_ON_ST:
        if (timer >= TIMER_CYCLES) {
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
      print(STATE_UPDATE_ERR_MSG);
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
      print(STATE_ACTION_ERR_MSG);
      break;
  }  

}

// Calling this starts the timer.
void hitLedTimer_start() {
    if (timer_enable)
        timer_start = true;
    else
        timer_start = false;
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return (timer_start & timer_enable);
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    mio_writePin(HIT_LED_MIO_PIN, LED_ON);
    leds_write(LED_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    mio_writePin(HIT_LED_MIO_PIN, LED_ON);
    leds_write(LED_ON);
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

}
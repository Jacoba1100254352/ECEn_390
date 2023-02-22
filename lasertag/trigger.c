#include "trigger.h"
#include "stdbool.h"
#include "buttons.h"
#include "mio.h"

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

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10
#define GUN_TRIGGER_PRESSED 1
#define GUN_TRIGGER_RELEASED 0
#define STATE_UPDATE_ERR_MSG "Error in state update"
#define STATE_ACTION_ERR_MSG "Error in state action"
#define PRESSED_ST_MSG "In PRESSED_ST"
#define RELEASED_ST_MSG "In RELEASED_ST"

volatile static trigger_shotsRemaining_t trigger_shots_remaining;
volatile static bool ignoreGunInput;

// States for the controller state machine.
volatile enum trigger_st_t {
	PRESSED_ST,
    RELEASED_ST
};
volatile static enum trigger_st_t currentState;

// Trigger can be activated by either btn0 or the external gun that is attached to TRIGGER_GUN_TRIGGER_MIO_PIN
// Gun input is ignored if the gun-input is high when the init() function is invoked.
bool triggerPressed() {
	return ((!ignoreGunInput & (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) || 
        (buttons_read() & BUTTONS_BTN0_MASK));
}

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init() {
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(TRIGGER_GUN_TRIGGER_MIO_PIN);  // Configure the signal direction of the pin to be an output.
    ignoreGunInput = false;
    currentState = RELEASED_ST;
    buttons_init();
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
      case PRESSED_ST:
        printf(PRESSED_ST_MSG);
        break;
      case RELEASED_ST:
        printf(RELEASED_ST_MSG);
        break;
     }
  }
}

// Standard tick function.
void trigger_tick() {
  debugStatePrint();
  
      // Perform state update first.
  switch(currentState) {
    case PRESSED_ST:
        
      break;
    case RELEASED_ST:
        
      break;
    default:
      print(STATE_UPDATE_ERR_MSG);
      break;
  }
  
  // Perform state action next.
  switch(currentState) {
    case PRESSED_ST:
      break;
    case RELEASED_ST:
      break;
     default:
      print(STATE_ACTION_ERR_MSG);
      break;
  }      
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() {
    ignoreGunInput = false;
}

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() {
    ignoreGunInput = true;
}

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {
    return trigger_shots_remaining;
}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
    trigger_shots_remaining = count;
}

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest() {

}
#include "trigger.h"
#include "buttons.h"
#include "mio.h"
#include <stdbool.h>
#include "transmitter.h"
#include <stdio.h>
#include "utils.h"

// Uncomment for debug prints
 #define DEBUG

#if defined(DEBUG)
#include "xil_printf.h"
#include <stdio.h>
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10
#define GUN_TRIGGER_PRESSED 1
#define GUN_TRIGGER_RELEASED 0
#define DEBOUNCED_VALUE_TIME 5000 // 50 ms
#define STATE_UPDATE_ERR_MSG "Error in state update\n"
#define STATE_ACTION_ERR_MSG "Error in state action\n"
#define PRESSED_ST_MSG "In PRESSED_ST\n"
#define RELEASED_ST_MSG "In RELEASED_ST\n"
#define DEBOUNCE_ST_MSG "In DEBOUNCE_ST\n"
#define BOUNCE_DELAY 5

volatile static trigger_shotsRemaining_t trigger_shots_remaining;
volatile static bool ignoreGunInput;
volatile static uint64_t counter;
volatile static bool checkTriggerValue;
volatile static bool isFirstPress;

// States for the controller state machine.
volatile static enum trigger_st_t { PRESSED_ST, RELEASED_ST, DEBOUNCE_ST } currentState = RELEASED_ST;

// Trigger can be activated by either btn0 or the external gun that is attached
// to TRIGGER_GUN_TRIGGER_MIO_PIN Gun input is ignored if the gun-input is high
// when the init() function is invoked.
bool isTriggerPressed() {
  return ((!ignoreGunInput &
           (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) ||
          (buttons_read() & BUTTONS_BTN0_MASK));
}

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init() {
  mio_init(false); // false disables any debug printing if there is a system
                   // failure during init.
  mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
  ignoreGunInput = isTriggerPressed();
  currentState = RELEASED_ST;
  buttons_init();
  counter = 0;
  isFirstPress = false;
}

// This is a debug state print routine. It will print the names of the states
// each time tick() is called. It only prints states if they are different than
// the previous state.
static void debugStatePrint() {
  static enum trigger_st_t previousState = DEBOUNCE_ST; // Start it out different from the currentState
  if (previousState != currentState) {
    previousState = currentState;
    switch (currentState) {
    case PRESSED_ST:
      printf(PRESSED_ST_MSG);
      break;
    case RELEASED_ST:
      printf(RELEASED_ST_MSG);
      break;
    case DEBOUNCE_ST:
      printf(DEBOUNCE_ST_MSG);
      break;
    }
  }
}

// Standard tick function.
void trigger_tick() {
  static enum trigger_st_t previousState = RELEASED_ST;
  //debugStatePrint();

  // Perform state update first.
  switch (currentState) {
  case PRESSED_ST:
    // If the trigger is released, then we need to go to the debounce state
    if (!isTriggerPressed()) {
      previousState = PRESSED_ST;
      DPCHAR('D');
      DPCHAR('\n');
      checkTriggerValue = false;
      counter = 0;
      currentState = DEBOUNCE_ST;
    }
    break;
  case RELEASED_ST:
    // If the trigger is pressed, then we need to go to the debounce state
    if (isTriggerPressed()) {
      previousState = RELEASED_ST;
      DPCHAR('U');
      DPCHAR('\n');
      checkTriggerValue = true;
      counter = 0;
      currentState = DEBOUNCE_ST;
    }
    break;
  case DEBOUNCE_ST:
    // If the trigger value has changed, then we need to go back to the previous state
    if (checkTriggerValue != isTriggerPressed())
      currentState = previousState;

    // Check if the counter has reached the debounce time
    if (counter == DEBOUNCED_VALUE_TIME) {
      // Transition based on previous state
      if (previousState == RELEASED_ST) { 
        isFirstPress = true;
        currentState = PRESSED_ST;
      } else
        currentState = RELEASED_ST;
    }
    break;
  default:
    printf(STATE_UPDATE_ERR_MSG);
    break;
  }

  // Perform state action next.
  switch (currentState) {
  case PRESSED_ST:
    if (isFirstPress) { // Only decrement the number of shots remaining if this
                        // is the first press of the trigger.
      trigger_shots_remaining--;
      isFirstPress = false;
      transmitter_run();
    }
    break;
  case RELEASED_ST:
    break;
  case DEBOUNCE_ST:
    counter++;
    break;
  default:
    printf(STATE_ACTION_ERR_MSG);
    break;
  }
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() { ignoreGunInput = false; }

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() { ignoreGunInput = true; }

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
  while (!(buttons_read() & BUTTONS_BTN3_MASK)) // Check if BTN3 is pressed
    trigger_enable();

  // Wait for button release
  do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
}
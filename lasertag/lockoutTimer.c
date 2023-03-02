#include "lockoutTimer.h"
#include "../drivers/intervalTimer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "buttons.h"

/******************
*   DEFINITIONS   *
******************/

#define START 0
#define TIMER_NUM 1
#define BOUNCE_DELAY 5


/********************************
*   GLOBAL VOLATILE VARIABLES   *
********************************/

volatile static bool timerRunning;
volatile static uint32_t lockoutCounter;

volatile enum hitLedTimer_st_t {
  running_st,
  lockout_st,
} lockoutCurrentState = running_st;


/****************
*   FUNCTIONS   *
****************/

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
  timerRunning = false;
  lockoutCounter = START;
  lockoutCurrentState = running_st;
}

// Standard tick function.
void lockoutTimer_tick() {
  // State update.
  switch (lockoutCurrentState) {
    case running_st:
      if (timerRunning)
        lockoutCurrentState = lockout_st;
      break;
    case lockout_st:
      if (lockoutCounter >= LOCKOUT_TIMER_EXPIRE_VALUE) {
        lockoutCounter = START;
        timerRunning = false;
        lockoutCurrentState = running_st;
      }
      break;
    default:
      printf("lockoutTimer_tick state transition: hit default\n\r");
      break;
  }

  // State action.
  switch (lockoutCurrentState) {
    case running_st:
      break;
    case lockout_st:
      lockoutCounter++;
      break;
    default:
      printf("lockoutTimer_tick state action: hit default\n\r");
      break;
  }
}

// Calling this starts the timer.
void lockoutTimer_start() { timerRunning = true; }

// Returns true if the timer is running.
bool lockoutTimer_running() { return timerRunning; }

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
  // Initialize timer.
  intervalTimer_init(TIMER_NUM);
  intervalTimer_reset(TIMER_NUM);
  intervalTimer_start(TIMER_NUM);
  lockoutTimer_start();

  // Wait for lockout timer to expire.
  while (lockoutTimer_running());

  // Stop timer and print out duration.
  intervalTimer_stop(TIMER_NUM);
  printf("lockout duration: %f \n\r", intervalTimer_getTotalDurationInSeconds(TIMER_NUM));

  // Check if lockout timer expired after correct amount of time.
  do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
}
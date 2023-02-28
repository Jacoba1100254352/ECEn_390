#include "lockoutTimer.h"
#include "../drivers/intervalTimer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define START 0
#define TIMER_NUM 1

volatile static bool timerRunning;
volatile static uint32_t lockoutCounter;

volatile enum hitLedTimer_st_t {
  running_st,
  lockout_st,
} lockoutCurrentState = running_st;

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
  timerRunning = false;
  lockoutCounter = START;
  lockoutCurrentState = running_st;
}

// Standard tick function.
void lockoutTimer_tick() {
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
  intervalTimer_init(TIMER_NUM);
  intervalTimer_reset(TIMER_NUM);
  intervalTimer_start(TIMER_NUM);
  lockoutTimer_start();
  while (lockoutTimer_running())
    ;
  intervalTimer_stop(TIMER_NUM);
  printf("lockout duration: %f \n\r",
         intervalTimer_getTotalDurationInSeconds(TIMER_NUM));
  // TODO: Prints out pass/fail status and other info to console.
  // TODO: Returns true if passes, false otherwise.
}
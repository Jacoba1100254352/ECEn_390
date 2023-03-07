#include "isr.h"
#include "trigger.h"
#include "hitLedTimer.h"
#include "transmitter.h"
#include "lockoutTimer.h"
#include "buffer.h"

// Perform initialization for interrupt and timing related modules.
void isr_init() {
    transmitter_init();
    trigger_init();
    hitLedTimer_init();
    lockoutTimer_init();
    buffer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
// All tick functions may only be called from in this function
void isr_function() {
    trigger_tick();
    hitLedTimer_tick();
    transmitter_tick();
    lockoutTimer_tick();
    buffer_pushover(interrupts_getAdcData());
}
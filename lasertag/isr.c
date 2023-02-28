#include "isr.h"

// Perform initialization for interrupt and timing related modules.
void isr_init() {
    transmitter_init();
    trigger_init();
    hitLedTimer_init();
    lockoutTimer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
// All tick functions may only be called from in this function
void isr_function() {
    trigger_tick();
    hitLedTimer_tick();
    transmitter_tick();
    hitLedTimer_tick();
}
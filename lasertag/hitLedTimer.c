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
#define WRITE_LED_0_ON 1
#define WRITE_LED_0_OFF 0
#define MIO_LED_ON 1
#define MIO_LED_OFF 0

// Need to init things.
void hitLedTimer_init() {
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(HIT_LED_MIO_PIN);  // Configure the signal direction of the pin to be an output.
    // Other stuff...
}

// Standard tick function.
void hitLedTimer_tick() {

}

// Calling this starts the timer.
void hitLedTimer_start() {

}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {

}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    mio_writePin(HIT_LED_MIO_PIN, MIO_LED_ON);
    leds_write(WRITE_LED_0_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    mio_writePin(HIT_LED_MIO_PIN, MIO_LED_OFF);
    leds_write(WRITE_LED_0_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable() {

}

// Enables the hitLedTimer.
void hitLedTimer_enable() {
    
}

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {

}
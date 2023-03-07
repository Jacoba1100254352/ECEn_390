#include "detector.h"
#include "filter.h"
#include "buffer.h"
#include "interrupts.h"

// Uncomment for debug prints
// #define DEBUG
 
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h" // outbyte
#define DPRINTF(...) printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif

#define DETECTOR_MAX_HITS 10
#define FILTER_NUMBER 10
#define DECIMATION_VAL 10

volatile static detector_hitCount_t hitArray[FILTER_NUMBER];
volatile static bool hitDetectedFlag;
volatile static bool freqArray[FILTER_NUMBER];


// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void) {
    filter_init();
    for (int i = 0; i < DETECTOR_MAX_HITS; i++)
        hitArray[i] = 0;
    hitDetectedFlag = false;
}

// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]) {
    for (int i = 0; i < DETECTOR_MAX_HITS; i++)
        if (freqArray[i])
            hitArray[i] = 0;
    
}

// Runs the entire detector: decimating FIR-filter, IIR-filters,
// power-computation, hit-detection. If interruptsCurrentlyEnabled = true,
// interrupts are running. If interruptsCurrentlyEnabled = false you can pop
// values from the ADC buffer without disabling interrupts. If
// interruptsCurrentlyEnabled = true, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC buffer.
// 3. re-enable interrupts.
// Ignore hits on frequencies specified with detector_setIgnoredFrequencies().
// Assumption: draining the ADC buffer occurs faster than it can fill.
void detector(bool interruptsCurrentlyEnabled) {
    uint64_t elementCount = buffer_elements();
    for (uint64_t i = 0; i < elementCount; i++) {
        uint8_t decimationFactor = 0;
        if (interruptsCurrentlyEnabled) {
            interrupts_disableArmInts();
        }
        uint16_t rawAdcValue = buffer_pop();
        if (interruptsCurrentlyEnabled) {
            interrupts_enableArmInts();
        }
        double scaledAdcValue = ((double)rawAdcValue - 2047.5)/2047.5;
        DPRINTF("ADC value: %d, scaled ADC value: %f", rawAdcValue, scaledAdcValue);
        filter_addNewInput(scaledAdcValue);
        decimationFactor++;
        if (DECIMATION_VAL == decimationFactor) {
            filter_firFilter();
            for (uint8_t filterNumber = 0; filterNumber < FILTER_NUMBER; filterNumber++) {
                filter_iirFilter(filterNumber);
                filter_computePower(filterNumber, true, false); //Check if we want to compute from scratch each time
            }
        }
        if (lockoutTimer_running()) {
            uint16_t freqHit = detector_getFrequencyNumberOfLastHit();
            if (detector_hitDetected() && !freqArray[freqHit]) {
                lockoutTimer_start();
                hitLedTimer_start();
                hitArray[freqHit]++;
                hitDetectedFlag = true;
            }
        }
    }


}

// Returns true if a hit was detected.
bool detector_hitDetected(void) {

}

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void) {

}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void) {

}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) {

}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {

}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factor) {

}

// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void) {

}

/******************************************************
******************** Test Routines ********************
******************************************************/

// Students implement this as part of Milestone 3, Task 3.
// Create two sets of power values and call your hit detection algorithm
// on each set. With the same fudge factor, your hit detect algorithm
// should detect a hit on the first set and not detect a hit on the second.
void detector_runTest(void) {

}
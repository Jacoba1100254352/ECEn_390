#include "detector.h"
#include "filter.h"
#include "buffer.h"
#include "interrupts.h"

/*
When you invoke detector(), perform the following steps.

Query the ADC buffer to determine how many elements it contains. Use buffer_elements() for this. Call this amount elementCount.
Now, repeat the following steps elementCount times.
If interrupts are enabled (check to see if the interruptsEnabled argument == true), briefly disable interrupts by invoking interrupts_disableArmInts(). You must disable interrupts briefly while you pop an element from the ADC buffer. Otherwise, if an interrupt occurs while you are “popping” a value from the buffer, the interrupt routine and your detector() routine may simultaneously access the ADC buffer and may cause indexIn, indexOut, or some other field of the ADC buffer to be miscomputed. This kind of problem can be very difficult to track down because it is hard to reproduce, so it is best to avoid the problem in the first place.
Pop a value from the ADC buffer (use buffer_pop() for this). Place this value in a variable called rawAdcValue.
If the interruptsEnabled argument was true, re-enable interrupts by invoking interrupts_enableArmInts().
Scale the integer value contained in rawAdcValue to a double that is between -1.0 and 1.0. Store this value into a variable named scaledAdcValue. The ADC generates a 12-bit output that ranges from 0 to 4095. 0 would map to -1.0. 4095 maps to 1.0. Values in between 0 and 4095 map linearly to values between -1.0 and 1.0. Note: this is a common source of bugs. Carefully test the code that does this mapping.
Invoke filter_addNewInput(scaledAdcValue). This provides a new input to the FIR filter.
If filter_addNewInput() has been called 10 times since the last invocation of the FIR and IIR filters, run the FIR filter, IIR filter and power computation for all 10 channels. Remember to only invoke these filters and power computations after filter_addNewInput() has been called 10 times (decimation). If you have just run the filters and computed power, also do the following:
if the lockoutTimer is not running, run the hit-detection algorithm. If you detect a hit and the frequency with maximum power is not an ignored frequency, do the following:
start the lockoutTimer.
start the hitLedTimer.
increment detector_hitArray at the index of the frequency of the IIR-filter output where you detected the hit. Note that detector_hitArray is a 10-element integer array that simply holds the current number of hits, for each frequency, that have occurred to this point.
set detector_hitDetectedFlag to true.
You will implement the detector in detector.c. The header file detector.h is already provided with the laser tag project. Here is an overview of the required functions.

detector_init(): Initializes the detector module. By default, all frequencies are considered for hits. The function assumes the filter module is initialized previously.
detector_setIgnoredFrequencies(bool freqArray[]): You pass it a preinitialized array of 10 booleans. If freqArray[0] is true, for example, no hits should be registered for this frequency when you run the detector() function.
detector(bool interruptsCurrentlyEnabled): This runs the entire detector once each time 10 new inputs have been received, including the decimating FIR-filter, all 10 IIR-filters, power computation, and the previously-described hit-detection algorithm. detector() sets a boolean flag to true if a hit was detected. The interruptsCurrentlyEnabled flag will be set to true if interrupts are enabled and false otherwise.
detector_hitDetected(): Simply returns the boolean flag that was set by detector(). Example code provided in runningModes.c calls detector() and then calls detector_hitDetected() to determine if you have been hit.
detector_clear(): This function simply clears the aforementioned flag.
detector_getHitCounts(): This function simply copies the values from the detector_hitArray into the supplied argument. You provide an array of size 10 as the only argument. After invoking this function, the array will contain the same values as detector_hitArray.
detector_runTest(): When invoked, this function will test the functionality of your detector software. This test function is described below.
*/

// Uncomment for debug prints
// #define DEBUG
 
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h" // outbyte
#define DPRINTF(...) printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif

#define FILTER_NUMBER 10
#define DECIMATION_VAL 10

volatile static detector_hitCount_t hitArray[FILTER_NUMBER];
volatile static bool hitDetectedFlag;
volatile static bool freqArray[FILTER_NUMBER];
volatile static bool fudgeFactorIndex;

// Initialize the detector module.
// By default, all frequencies are considered for hits.
// Assumes the filter module is initialized previously.
void detector_init(void) {
    filter_init();
    for (int i = 0; i < FILTER_NUMBER; i++)
        hitArray[i] = 0;
    hitDetectedFlag = false;
    fudgeFactorIndex = 0;
}

// freqArray is indexed by frequency number. If an element is set to true,
// the frequency will be ignored. Multiple frequencies can be ignored.
// Your shot frequency (based on the switches) is a good choice to ignore.
void detector_setIgnoredFrequencies(bool freqArray[]) {
    for (int i = 0; i < FILTER_NUMBER; i++)
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
        if (interruptsCurrentlyEnabled)
            interrupts_disableArmInts();
        uint16_t rawAdcValue = buffer_pop();
        if (interruptsCurrentlyEnabled)
            interrupts_enableArmInts();
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
    return hitDetectedFlag;
}

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(void) {
    uint16_t freqHit = 0;
    for (uint8_t i = 0; i < FILTER_NUMBER; i++) {
        if (filter_getPower(i) > filter_getPower(freqHit))
            freqHit = i;
    }
    return freqHit;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(void) {
    hitDetectedFlag = false;
}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) {
    for (uint8_t i = 0; i < FILTER_NUMBER; i++)
        freqArray[i] = flagValue;
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t userHitArray[]) {
    for (uint8_t i = 0; i < DETECTOR_MAX_HITS; i++)
        userHitArray[i] = hitArray[i];
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t factor) {
    fudgeFactorIndex = factor;
}

// Returns the detector invocation count.
// The count is incremented each time detector is called.
// Used for run-time statistics.
uint32_t detector_getInvocationCount(void) {
    return filter_getInvocationCount();
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
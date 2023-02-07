#include "filter.h"
#include "queue.h"

const static double firCoefficients[FIR_FILTER_TAP_COUNT] = {
4.3579622275120866e-04, 
2.7155425450406482e-04, 
6.3039002645022389e-05, 
-1.9349227837935689e-04, 
-4.9526428865281219e-04, 
-8.2651441681321381e-04, 
-1.1538970332472540e-03, 
-1.4254746936265955e-03, 
-1.5744703111426981e-03, 
-1.5281041447445794e-03, 
-1.2208092333090719e-03, 
-6.1008312441271589e-04, 
3.0761698758506020e-04, 
1.4840192333212628e-03, 
2.8123077568332064e-03, 
4.1290616416556000e-03, 
5.2263464670258821e-03, 
5.8739882867061598e-03, 
5.8504032099208096e-03, 
4.9787419333799775e-03, 
3.1637974805960069e-03, 
4.2435139609132765e-04, 
-3.0844289197247210e-03, 
-7.0632027332701800e-03, 
-1.1078458037608587e-02, 
-1.4591395057493114e-02, 
-1.7004337345765962e-02, 
-1.7720830774014484e-02, 
-1.6213409845727566e-02, 
-1.2091458677988302e-02, 
-5.1609257765542595e-03, 
4.5319860006883522e-03, 
1.6679627700682677e-02, 
3.0718365411587255e-02, 
4.5861875593064996e-02, 
6.1160185621895728e-02, 
7.5579213982547147e-02, 
8.8092930943210607e-02, 
9.7778502396672365e-02, 
1.0390414346016495e-01, 
1.0600000000000000e-01, 
1.0390414346016495e-01, 
9.7778502396672365e-02, 
8.8092930943210607e-02, 
7.5579213982547147e-02, 
6.1160185621895728e-02, 
4.5861875593064996e-02, 
3.0718365411587255e-02, 
1.6679627700682677e-02, 
4.5319860006883522e-03, 
-5.1609257765542595e-03, 
-1.2091458677988302e-02, 
-1.6213409845727566e-02, 
-1.7720830774014484e-02, 
-1.7004337345765962e-02, 
-1.4591395057493114e-02, 
-1.1078458037608587e-02, 
-7.0632027332701800e-03, 
-3.0844289197247210e-03, 
4.2435139609132765e-04, 
3.1637974805960069e-03, 
4.9787419333799775e-03, 
5.8504032099208096e-03, 
5.8739882867061598e-03, 
5.2263464670258821e-03, 
4.1290616416556000e-03, 
2.8123077568332064e-03, 
1.4840192333212628e-03, 
3.0761698758506020e-04, 
-6.1008312441271589e-04, 
-1.2208092333090719e-03, 
-1.5281041447445794e-03, 
-1.5744703111426981e-03, 
-1.4254746936265955e-03, 
-1.1538970332472540e-03, 
-8.2651441681321381e-04, 
-4.9526428865281219e-04, 
-1.9349227837935689e-04, 
6.3039002645022389e-05, 
2.7155425450406482e-04, 
4.3579622275120866e-04};

const static double iirACoefficientConstants[FILTER_FREQUENCY_COUNT][IIR_A_COEFFICIENT_COUNT] = {
{-5.9637727070164015e+00, 1.9125339333078248e+01, -4.0341474540744173e+01, 6.1537466875368821e+01, -7.0019717951472188e+01, 6.0298814235238872e+01, -3.8733792862566290e+01, 1.7993533279581058e+01, -5.4979061224867651e+00, 9.0332828533799547e-01},
{-4.6377947119071443e+00, 1.3502215749461572e+01, -2.6155952405269755e+01, 3.8589668330738348e+01, -4.3038990303252632e+01, 3.7812927599537133e+01, -2.5113598088113793e+01, 1.2703182701888094e+01, -4.2755083391143520e+00, 9.0332828533800291e-01},
{-3.0591317915750960e+00, 8.6417489609637634e+00, -1.4278790253808875e+01, 2.1302268283304372e+01, -2.2193853972079314e+01, 2.0873499791105537e+01, -1.3709764520609468e+01, 8.1303553577932188e+00, -2.8201643879900726e+00, 9.0332828533800769e-01},
{-1.4071749185996747e+00, 5.6904141470697471e+00, -5.7374718273676217e+00, 1.1958028362868873e+01, -8.5435280598354382e+00, 1.1717345583835918e+01, -5.5088290876998407e+00, 5.3536787286077372e+00, -1.2972519209655518e+00, 9.0332828533799414e-01},
{8.2010906117760141e-01, 5.1673756579268559e+00, 3.2580350909220819e+00, 1.0392903763919172e+01, 4.8101776408668879e+00, 1.0183724507092480e+01, 3.1282000712126603e+00, 4.8615933365571822e+00, 7.5604535083144497e-01, 9.0332828533799658e-01},
{2.7080869856154512e+00, 7.8319071217995688e+00, 1.2201607990980744e+01, 1.8651500443681620e+01, 1.8758157568004549e+01, 1.8276088095999022e+01, 1.1715361303018897e+01, 7.3684394621253499e+00, 2.4965418284511904e+00, 9.0332828533800436e-01},
{4.9479835250075892e+00, 1.4691607003177602e+01, 2.9082414772101060e+01, 4.3179839108869331e+01, 4.8440791644688879e+01, 4.2310703962394342e+01, 2.7923434247706432e+01, 1.3822186510471010e+01, 4.5614664160654357e+00, 9.0332828533799958e-01},
{6.1701893352279846e+00, 2.0127225876810336e+01, 4.2974193398071684e+01, 6.5958045321253451e+01, 7.5230437667866596e+01, 6.4630411355739852e+01, 4.1261591079244127e+01, 1.8936128791950534e+01, 5.6881982915180291e+00, 9.0332828533799803e-01},
{7.4092912870072398e+00, 2.6857944460290135e+01, 6.1578787811202247e+01, 9.8258255839887312e+01, 1.1359460153696298e+02, 9.6280452143026082e+01, 5.9124742025776392e+01, 2.5268527576524203e+01, 6.8305064480743081e+00, 9.0332828533799969e-01},
{8.5743055776347692e+00, 3.4306584753117889e+01, 8.4035290411037053e+01, 1.3928510844056814e+02, 1.6305115418161620e+02, 1.3648147221895786e+02, 8.0686288623299745e+01, 3.2276361903872115e+01, 7.9045143816244696e+00, 9.0332828533799636e-01}
};

const static double iirBCoefficientConstants[FILTER_FREQUENCY_COUNT][IIR_B_COEFFICIENT_COUNT] = {
{9.0928661148194738e-10, 0.0000000000000000e+00, -4.5464330574097372e-09, 0.0000000000000000e+00, 9.0928661148194745e-09, 0.0000000000000000e+00, -9.0928661148194745e-09, 0.0000000000000000e+00, 4.5464330574097372e-09, 0.0000000000000000e+00, -9.0928661148194738e-10},
{9.0928661148185608e-10, 0.0000000000000000e+00, -4.5464330574092806e-09, 0.0000000000000000e+00, 9.0928661148185613e-09, 0.0000000000000000e+00, -9.0928661148185613e-09, 0.0000000000000000e+00, 4.5464330574092806e-09, 0.0000000000000000e+00, -9.0928661148185608e-10},
{9.0928661148182951e-10, 0.0000000000000000e+00, -4.5464330574091475e-09, 0.0000000000000000e+00, 9.0928661148182949e-09, 0.0000000000000000e+00, -9.0928661148182949e-09, 0.0000000000000000e+00, 4.5464330574091475e-09, 0.0000000000000000e+00, -9.0928661148182951e-10},
{9.0928661148210734e-10, 0.0000000000000000e+00, -4.5464330574105371e-09, 0.0000000000000000e+00, 9.0928661148210742e-09, 0.0000000000000000e+00, -9.0928661148210742e-09, 0.0000000000000000e+00, 4.5464330574105371e-09, 0.0000000000000000e+00, -9.0928661148210734e-10},
{9.0928661148197561e-10, 0.0000000000000000e+00, -4.5464330574098779e-09, 0.0000000000000000e+00, 9.0928661148197557e-09, 0.0000000000000000e+00, -9.0928661148197557e-09, 0.0000000000000000e+00, 4.5464330574098779e-09, 0.0000000000000000e+00, -9.0928661148197561e-10},
{9.0928661148179839e-10, 0.0000000000000000e+00, -4.5464330574089919e-09, 0.0000000000000000e+00, 9.0928661148179839e-09, 0.0000000000000000e+00, -9.0928661148179839e-09, 0.0000000000000000e+00, 4.5464330574089919e-09, 0.0000000000000000e+00, -9.0928661148179839e-10},
{9.0928661148193684e-10, 0.0000000000000000e+00, -4.5464330574096843e-09, 0.0000000000000000e+00, 9.0928661148193686e-09, 0.0000000000000000e+00, -9.0928661148193686e-09, 0.0000000000000000e+00, 4.5464330574096843e-09, 0.0000000000000000e+00, -9.0928661148193684e-10},
{9.0928661148195069e-10, 0.0000000000000000e+00, -4.5464330574097538e-09, 0.0000000000000000e+00, 9.0928661148195076e-09, 0.0000000000000000e+00, -9.0928661148195076e-09, 0.0000000000000000e+00, 4.5464330574097538e-09, 0.0000000000000000e+00, -9.0928661148195069e-10},
{9.0928661148190954e-10, 0.0000000000000000e+00, -4.5464330574095478e-09, 0.0000000000000000e+00, 9.0928661148190956e-09, 0.0000000000000000e+00, -9.0928661148190956e-09, 0.0000000000000000e+00, 4.5464330574095478e-09, 0.0000000000000000e+00, -9.0928661148190954e-10},
{9.0928661148206091e-10, 0.0000000000000000e+00, -4.5464330574103047e-09, 0.0000000000000000e+00, 9.0928661148206094e-09, 0.0000000000000000e+00, -9.0928661148206094e-09, 0.0000000000000000e+00, 4.5464330574103047e-09, 0.0000000000000000e+00, -9.0928661148206091e-10}
};

// Filtering routines for the laser-tag project.
// Filtering is performed by a two-stage filter, as described below.

// 1. First filter is a decimating FIR filter with a configurable number of taps
// and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10
// IIR filters. The characteristics of the IIR filter are fixed.

/******************************************************************************
***** Main Filter Functions
******************************************************************************/

#define QUEUE_INIT_VALUE 0.0
#define FILTER_IIR_FILTER_COUNT 10
#define FIR_B_COEFFICIENT_COUNT 81
#define IIR_A_COEFFICIENT_COUNT 10
#define IIR_B_COEFFICIENT_COUNT 11
#define Z_QUEUE_SIZE IIR_A_COEFFICIENT_COUNT
#define OUTPUT_QUEUE_SIZE IIR_A_COEFFICIENT_COUNT
static queue_t xQueue;	
static queue_t yQueue;	
static queue_t zQueue[FILTER_IIR_FILTER_COUNT];
static queue_t outputQueue[FILTER_IIR_FILTER_COUNT];	
 
static void initXQueue() {
    queue_init(&xQueue, FIR_B_COEFFICIENT_COUNT, "xQueue");
    for (uint32_t i = 0; i < FIR_B_COEFFICIENT_COUNT; i++)
        queue_overwritePush(&xQ, 0.0);
}

static void initYQueue() {
    queue_init(&yQueue, IIR_B_COEFFICIENT_COUNT, "yQueue"); 
    for (uint32_t j = 0; j < IIR_B_COEFFICIENT_COUNT; j++)
        queue_overwritePush(&yQueue, QUEUE_INIT_VALUE);
}
static void initZQueues() {
  for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
    queue_init(&(zQueue[i]), Z_QUEUE_SIZE, "zQueue");
    for (uint32_t j = 0; j < Z_QUEUE_SIZE; j++)
        queue_overwritePush(&(zQueue[i]), QUEUE_INIT_VALUE);
  }
}

static void initOutputQueues() {
  for (uint32_t i = 0; i < FILTER_IIR_FILTER_COUNT; i++) {
    queue_init(&(outputQueue[i]), OUTPUT_QUEUE_SIZE, "outputQueue");
    for (uint32_t j = 0; j < OUTPUT_QUEUE_SIZE; j++)
        queue_overwritePush(&(zQueue[i]), QUEUE_INIT_VALUE);
  }
}

// Must call this prior to using any filter functions.
void filter_init() {
  // Init queues and fill them with 0s.
  initXQueue();  // Call queue_init() on xQueue and fill it with zeros.
  initYQueue();  // Call queue_init() on yQueue and fill it with zeros.
  initZQueues(); // Call queue_init() on all of the zQueues and fill each z queue with zeros.
  initOutputQueues();  // Call queue_init() on all of the outputQueues and fill each outputQueue with zeros.
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x) {
    queue_overwritePush(&xQueue, x);
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter() {
    // Compute the next y using a for loop
    // += accumulates the result during the for-loop
    double y = 0.0;
    
    for (uint32_t i = 0; i < FIR_B_COEFFICIENT_COUNT; i++) { // iteratively adds the (b * input) products.
        y += queue_readElementAt(&xQueue, FIR_B_COEFFICIENT_COUNT-1-i) * firCoefficients[i];
    }
    queue_overwritePush(&yQueue, y); // Push the reuslt onto y
    return y;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber) {
    double z = 0.0;
    // This for-loop performs the identical computation to that shown above.
    for (uint32_t i = 0; i < IIR_B_COEFFICIENT_COUNT; i++) // iteratively adds the (b * input) products.
        z += queue_readElementAt(&yQueue, IIR_B_COEFFICIENT_COUNT-1-i) * iirBCoefficientConstants[filterNumber][i];

    for (uint32_t i = 0; i < IIR_A_COEFFICIENT_COUNT; i++) // iteratively adds the (b * input) products.
        z -= queue_readElementAt(&zQueue, IIR_A_COEFFICIENT_COUNT-1-i) * iirACoefficientConstants[filterNumber][i];

    queue_overwritePush(&zQueue, z); // Push the reuslt onto z
    return z;
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute power
// values the first time. After that, you can incrementally compute power values
// by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call
// this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
// (newest-value * newest-value). Note that this function will probably need an
// array to keep track of these values for each of the 10 output queues.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint) {

}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber) {

}

// Sets a current power value for a specific filter number.
// Useful in testing the detector.
void filter_setCurrentPowerValue(uint16_t filterNumber, double value) {

}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]) {

}

// Using the previously-computed power values that are currently stored in
// currentPowerValue[] array, copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
// The pointer argument indexOfMaxValue is used to return the index of the
// maximum value. If the maximum power is zero, make sure to not divide by zero
// and that *indexOfMaxValue is initialized to a sane value (like zero).
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t *indexOfMaxValue) {

}

/******************************************************************************
***** Verification-Assisting Functions
***** External test functions access the internal data structures of filter.c
***** via these functions. They are not used by the main filter functions.
******************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray() {
    return &firCoefficients;
}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount() {
    return FIR_B_COEFFICIENT_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {
    return &iirACoefficientConstants;
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() {
    return IIR_A_COEFFICIENT_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {
    return &iirBCoefficientConstants;
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() {
    return IIR_B_COEFFICIENT_COUNT;
}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() {
    return yQueue.size;
}

// Returns the decimation value.
uint16_t filter_getDecimationValue() {
    return FILTER_FIR_DECIMATION_FACTOR;
}

// Returns the address of xQueue.
queue_t *filter_getXQueue() {
    return &xQueue;
}

// Returns the address of yQueue.
queue_t *filter_getYQueue() {
    return &yQueue;
}

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) {
    return &zQueue[filterNumber];
}

// Returns the address of the IIR output-queue for a specific filter number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {
    return &outputQueue[filterNumber];
}
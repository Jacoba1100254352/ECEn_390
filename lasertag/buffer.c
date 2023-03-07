#include "buffer.h"
#include <stdint.h>
 
// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by the detector.
// The function of the buffer is similar to a queue or FIFO.
 
// Uncomment for debug prints
// #define DEBUG
 
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h" // outbyte
#define DPRINTF(...) printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif
 
#define BUFFER_SIZE 32768
 
typedef struct {
    uint32_t indexIn; // Points to the next open slot.
    uint32_t indexOut; // Points to the next element to be removed.
    uint32_t elementCount; // Number of elements in the buffer.
    buffer_data_t data[BUFFER_SIZE]; // Values are stored here.
} buffer_t;
 
volatile static buffer_t buf;
 
 
// Initialize the buffer to empty.
void buffer_init(void)
{
    buf.indexIn = 0;
    buf.indexOut = 0;
    buf.elementCount = 0;
    for (uint16_t i = 0; i < BUFFER_SIZE; i++)
        buf.data[i] = 0;
}
 
// Add a value to the buffer. Overwrite the oldest value if full.
void buffer_pushover(buffer_data_t value)
{
    buf.data[buf.indexIn] = value;
    buf.indexIn = (buf.indexIn + 1) % BUFFER_SIZE;
    if (buf.elementCount < BUFFER_SIZE)
        buf.elementCount++;
    else
        buf.indexOut = (buf.indexOut + 1) % BUFFER_SIZE;
}
 
// Remove a value from the buffer. Return zero if empty.
buffer_data_t buffer_pop(void)
{
    if (!buf.elementCount)
        return 0;
    buffer_data_t value = buf.data[buf.indexOut];
    buf.indexOut = (buf.indexOut + 1) % BUFFER_SIZE;
    buf.elementCount--;
    return value;
}
 
// Return the number of elements in the buffer.
uint32_t buffer_elements(void)
{
    return buf.elementCount;
}
 
// Return the capacity of the buffer in elements.
uint32_t buffer_size(void)
{
    return BUFFER_SIZE;
}
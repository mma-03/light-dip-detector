/*
flasher.h
Flashes the LED at a set frequency in its own background thread
Starts at 10 Hz, the frequency is limited to 0 (off) and the maximum.
Call Flasher_init() after Led_init(), and Flasher_cleanup() before Led_cleanup()
 */

#ifndef FLASHER_H
#define FLASHER_H

// Start the flashing thread at the starting frequency
void Flasher_init(void);

// Request a new frequency in Hz, values outside 0-max are clamped
// Does nothing if already at that frequency, safe from any thread
void Flasher_setFreq(int frequencyHz);

// Current frequency in Hz. Safe from any thread
int Flasher_getFreq(void);

// Stop the flashing thread and leave the LED off
void Flasher_cleanup(void);

#endif
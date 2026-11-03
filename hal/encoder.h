/*
encoder.h
HAL module for the rotary encoder on GPIO16 (A) and GPIO17 (B)
Reads both switches through libgpiod, with the built in pull-ups enabled
Call encoder_init() once before Encoder_readState(), and Encoder_cleanup() at shutdown
*/

#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>

// claim both encoder pins as inputs with pull-ups, return false on failure
bool Encoder_init(void);

// current switch state as a number (0-3), (A << 1) | b, return -1 on failure
int Encoder_readState(void);

// release the encoder pins, safe to call even if init fails
void Encoder_cleanup(void);

#endif
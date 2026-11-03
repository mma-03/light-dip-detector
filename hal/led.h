/*
led.h
HAL module for LED emitter on header pin 32 (GPIO12)
switches the LED on or off through libgpiod
call Led_init() once before Led_set(), and LED_cleanup() at shutdown
*/

#ifndef LED_H
#define LED_H

#include <stdbool.h>

// open chip, settings and config, request line, then free the settings and config
// if a step fails, release whatever was created and return false
bool Led_init(void);

// bool true = ACTIVE, false = INACTIVE
// set value and check for failure
void Led_set(bool on);

// release the request, close chip and set both pointers back to NULL
void Led_cleanup(void);

#endif
/*
knob.h
watches the rotary encoder in a background thread and changes the LED flasher frequency
by +1 Hz per clockwise rotation, -1 Hz per counter clockwise rotation.
call knob_init() after Encoder_init() and Flasher_init(), knob_cleanup at shutdown
*/

#ifndef KNOB_H
#define KNOB_H

// start watching the encoder
void Knob_init(void);

// stop watching the encoder
void Knob_cleanup(void);

#endif
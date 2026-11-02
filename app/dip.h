/*
interface for app/dip.c
A dip is when the voltage (light level) drops below a certain threshold 
Dip threshold = average - 0.10 V (dropping below this counts as a dip)
Re-arm level = average - 0.07 V (after a dip, the light must rise above this level before another dip is counted)
The count continues a dip that was in progress at the end of the last call and is not thread-safe (called from one thread only)
*/

#ifndef DIP_H
#define DIP_H

// to count the dips in one second, the function needs:
// the samples (history array), how many there are (size), and the average to measure from
int Dip_countDips (const double *samples, int size, double avg);

// returns the last count safe from any thread
int Dip_getLastCount(void);

#endif
/*
The Sampler Module:
- Samples the light level in its own background thread
- "History": the samples from the previous complete second
- Call Sampler_init() once at startup AFTER Adc_init(), Sampler_moveCurrentToHistory()
  once every second, and Sampler_cleanup() at shutdown  
*/

#ifndef SAMPLER_H
#define SAMPLER_H

// main calls Sampler_init() at startup
void Sampler_init(void);

// collect ~500 samples into "current", then at each 1-second mark
// move current's samples into history
// (A frozen copy of the previous second's samples)
void Sampler_moveCurrentToHistory(void);

// returns a copy of the history
// writes the number of samples into *size
// caller must free() the returned pointer
double *Sampler_getHistory(int *size);

// Average light level in volts
double Sampler_getAverage(void);

// How many samples are in history
int Sampler_getHistorySize(void);

// Total number of samples taken since startup
long long Sampler_getTotalSamples(void);

// main calls Sampler_cleanup at shutdown
void Sampler_cleanup(void);

#endif
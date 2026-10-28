/*
adc.h
HAL module for the MCP3208 8 channel, 12-bit ADC
Returns raw readings (0-4095), converting volts is up to the caller
Call Adc_init() once before any Adc_read() and Adc_cleanup()
*/

#ifndef ADC_H
#define ADC_H

#include <stdbool.h>

bool Adc_init(void);       // open + configure SPI, return false on failure

int Adc_read(int channel); // returns 0-4095, return -1 on error

void Adc_cleanup(void);    // close the device

#endif
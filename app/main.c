#include <stdio.h>
#include <time.h>
#include "adc.h"

#define LIGHT_CHANNEL 2
#define ADC_VREF_V 3.3
#define ADC_FULL_SCALE 4096.0
#define DELAY_MS 100
#define NS_PER_MS 1000000L

int main(void) {

    if(!Adc_init()) {
        fprintf(stderr, "Adc_init failed\n");
        return 1;
    }

    while(1) {
        struct timespec delay = {0, DELAY_MS * NS_PER_MS}; // DELAY_MS converted to ns
        nanosleep(&delay, NULL);

        int reading = Adc_read(LIGHT_CHANNEL);
        if (reading < 0) {
            fprintf(stderr, "Adc_read failed\n");
            continue;
        }

        double voltage = (reading * ADC_VREF_V)/(ADC_FULL_SCALE);

        printf("reading = %4d   voltage = %.3f V\n", reading, voltage);
    }
}
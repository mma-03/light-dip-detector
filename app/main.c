#include <stdio.h>
#include <time.h>
#include "adc.h"
#include "sampler.h"

#define ANALYSIS_PERIOD_MS 1000
#define NS_PER_MS 1000000L
#define MS_PER_S 1000

int main(void) {

    if(!Adc_init()) {
        fprintf(stderr, "Adc_init failed\n");
        return 1;
    }

    Sampler_init();

    while(1) {
        struct timespec delay = {
            ANALYSIS_PERIOD_MS / MS_PER_S, // whole seconds: 1000/1000 = 1
            (ANALYSIS_PERIOD_MS % MS_PER_S) * NS_PER_MS // left over ms to ns: 1000 % 1000 = 0
        }; 
        nanosleep(&delay, NULL);

        Sampler_moveCurrentToHistory();

        printf("History Size: %d, Average Voltage: %.3f V, Total Samples: %lld\n", Sampler_getHistorySize(), Sampler_getAverage(), Sampler_getTotalSamples());
    }
}
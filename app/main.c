#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "adc.h"
#include "sampler.h"
#include "periodTimer.h"
#include "dip.h"

#define ANALYSIS_PERIOD_MS 1000
#define NS_PER_MS 1000000L
#define MS_PER_S 1000

int main(void) {

    if(!Adc_init()) {
        fprintf(stderr, "Adc_init failed\n");
        return 1;
    }

    Period_init();
    Sampler_init();

    while(1) {
        struct timespec delay = {
            ANALYSIS_PERIOD_MS / MS_PER_S, // whole seconds
            (ANALYSIS_PERIOD_MS % MS_PER_S) * NS_PER_MS // left over ms to ns
        }; 
        nanosleep(&delay, NULL);

        Sampler_moveCurrentToHistory();

        Period_statistics_t stats;
        Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, &stats);

        printf("History Size: %d, Average Voltage: %.3f V, Total Samples: %lld\n", Sampler_getHistorySize(), Sampler_getAverage(), Sampler_getTotalSamples());
        printf("Smpl ms[%.3f, %.3f] avg %.3f/%d\n", stats.minPeriodInMs, stats.maxPeriodInMs, stats.avgPeriodInMs, stats.numSamples);

        int size;
        double *history = Sampler_getHistory(&size);
        double avg = Sampler_getAverage();

        int dipCount = Dip_countDips(history, size, avg);
        printf("dips: %d\n", dipCount);

        free(history);
    }
}
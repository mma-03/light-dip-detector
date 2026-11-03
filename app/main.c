#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "adc.h"
#include "sampler.h"
#include "periodTimer.h"
#include "dip.h"
#include "udpServer.h"
#include "led.h"
#include "flasher.h"
#include "encoder.h"
#include "knob.h"

#define ANALYSIS_PERIOD_MS 1000
#define NS_PER_MS 1000000L
#define MS_PER_S 1000
#define NUM_SAMPLES_TO_SHOW 10

// Helpers
static void printStatus(const double *history, int size, int flashHz, double avg, int dipCount, const Period_statistics_t *stats) {
    printf("#Smpl/s = %4d Flash @ %3dHz avg = %6.3fV dips = %3d Smpl ms[%6.3f, %6.3f] avg %6.3f/%4d\n", size, flashHz, avg, dipCount, stats->minPeriodInMs, stats->maxPeriodInMs, stats->avgPeriodInMs, stats->numSamples);
    
    int count = (size < NUM_SAMPLES_TO_SHOW) ? size : NUM_SAMPLES_TO_SHOW;
    for(int i = 0; i < count; i++) {
        int index = i * size / count;
        printf("%4d:%6.3f  ", index, history[index]);
    }
    printf("\n");
}

int main(void) {
    
    if(!Adc_init()) {
        fprintf(stderr, "Adc_init failed\n");
        return 1;
    }

    if (!Led_init()) {
        fprintf(stderr, "Led_init failed\n");
        Adc_cleanup();
        return 1;
    }

    if (!Encoder_init()) {
        fprintf(stderr, "Encoder_init failed\n");
        Led_cleanup();
        Adc_cleanup();
        return 1;
    }
    
    Period_init();
    Sampler_init();
    Flasher_init();
    Knob_init();

    if (!UdpServer_init()) {
        fprintf(stderr, "UdpServer_init failed\n");
        Knob_cleanup();
        Flasher_cleanup();
        Sampler_cleanup();
        Period_cleanup();
        Encoder_cleanup();
        Led_cleanup();
        Adc_cleanup();
        return 1;
    }

    while(!UdpServer_isStopRequested()) {
        struct timespec delay = {
            ANALYSIS_PERIOD_MS / MS_PER_S, // whole seconds
            (ANALYSIS_PERIOD_MS % MS_PER_S) * NS_PER_MS // left over ms to ns
        }; 
        nanosleep(&delay, NULL);

        Sampler_moveCurrentToHistory();

        Period_statistics_t stats;
        Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, &stats);

        int size;
        double *history = Sampler_getHistory(&size);
        double avg = Sampler_getAverage();

        int dipCount = Dip_countDips(history, size, avg);

        int flashHz = Flasher_getFreq();

        printStatus(history, size, flashHz, avg, dipCount, &stats);

        free(history);
    }

    UdpServer_cleanup();
    Knob_cleanup();
    Flasher_cleanup();
    Sampler_cleanup();
    Period_cleanup();
    Encoder_cleanup();
    Led_cleanup();
    Adc_cleanup();

    return 0;
}
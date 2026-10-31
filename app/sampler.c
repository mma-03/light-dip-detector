#include "sampler.h"
#include "adc.h"
#include "periodTimer.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

// constants
#define LIGHT_CHANNEL 2
#define ADC_VREF_V 3.3
#define ADC_FULL_SCALE 4096.0
#define MAX_SAMPLES_PER_SECOND 1000
#define SAMPLE_PERIOD_MS 1
#define NS_PER_MS 1000000L
#define AVERAGE_WEIGHT_PREVIOUS 0.999

// state

// holds sample taken this second
static double s_current[MAX_SAMPLES_PER_SECOND];

// how many are in s_current
static int s_currentSize;

// previous second's samples
static double s_history[MAX_SAMPLES_PER_SECOND];

// how many are in s_history
static int s_historySize;

// running average in volts
static double s_average;

// true until first sample arrives
static bool s_isFirstSample = true;

// count since startup
static long long s_totalSamples;

// thread loops while true
static bool s_isRunning = false;

static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;

// the sampler thread
static pthread_t s_thread;

static bool isRunning(void) {
    pthread_mutex_lock(&s_lock);
    bool running = s_isRunning;
    pthread_mutex_unlock(&s_lock);
    return running;
}

static void *samplerThread(void *arg) {
    (void)arg;

    while(isRunning()){
        int reading = Adc_read(LIGHT_CHANNEL);
        if (reading >= 0) {
            double voltage = (reading * ADC_VREF_V)/(ADC_FULL_SCALE);

            pthread_mutex_lock(&s_lock);

            if (s_currentSize < MAX_SAMPLES_PER_SECOND) {
                s_current[s_currentSize] = voltage;
                s_currentSize++;
            }

            if (s_isFirstSample) {
                s_average = voltage;
                s_isFirstSample = false;
            } else {
                s_average = AVERAGE_WEIGHT_PREVIOUS * s_average + (1 - AVERAGE_WEIGHT_PREVIOUS) * voltage;
            }

            s_totalSamples++;

            pthread_mutex_unlock(&s_lock);

            Period_markEvent(PERIOD_EVENT_SAMPLE_LIGHT);
        }

        struct timespec delay = {0, SAMPLE_PERIOD_MS * NS_PER_MS};
        nanosleep(&delay, NULL);
    }

    return NULL;
}

void Sampler_init(void) {
    s_isRunning = true;
    if (pthread_create(&s_thread, NULL, samplerThread, NULL) != 0) {
        fprintf(stderr, "pthread_create failed\n");
    }
}

void Sampler_moveCurrentToHistory(void) {
    pthread_mutex_lock(&s_lock);

    for (int i = 0; i < s_currentSize; i++) {
        s_history[i] = s_current[i];
    }

    s_historySize = s_currentSize;
    s_currentSize = 0;

    pthread_mutex_unlock(&s_lock);
}

double *Sampler_getHistory(int *size) {
    double *history = malloc(MAX_SAMPLES_PER_SECOND * sizeof(double));
    if (history == NULL) {
        fprintf(stderr, "History: Out of memory\n");
        *size = 0;
        return NULL;
    }

    pthread_mutex_lock(&s_lock);

    for(int i = 0; i < s_historySize; i++) {
        history[i] = s_history[i];
    }

    *size = s_historySize;

    pthread_mutex_unlock(&s_lock);

    return history;
}

double Sampler_getAverage(void) {
    pthread_mutex_lock(&s_lock);

    double average = s_average;

    pthread_mutex_unlock(&s_lock);

    return average;
}

int Sampler_getHistorySize(void) {
    pthread_mutex_lock(&s_lock);

    int historySize = s_historySize;

    pthread_mutex_unlock(&s_lock);

    return historySize;
}

long long Sampler_getTotalSamples(void) {
    pthread_mutex_lock(&s_lock);

    long long totalSamples = s_totalSamples;

    pthread_mutex_unlock(&s_lock);

    return totalSamples;
}

void Sampler_cleanup(void) {
    pthread_mutex_lock(&s_lock);

    s_isRunning = false;

    pthread_mutex_unlock(&s_lock);
    pthread_join(s_thread, NULL);
}
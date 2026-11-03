#include "flasher.h"
#include "led.h"
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define INIT_FREQ_HZ 10
#define MIN_FREQ_HZ 0
#define MAX_FREQ_HZ 500
#define IDLE_MS 10
#define NS_PER_MS 1000000
#define NS_PER_S 1000000000L

static int s_frequency = INIT_FREQ_HZ;
static bool s_isRunning = false;
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_thread;

static bool isRunning(void) {
    pthread_mutex_lock(&s_lock);
    bool running = s_isRunning;
    pthread_mutex_unlock(&s_lock);
    return running;
}

static void *flasherThread(void *arg) {
    (void)arg;

    bool ledOn = false;

    while(isRunning()) {
        int frequency = Flasher_getFreq();
        if (frequency == 0) {
            Led_set(false);
            ledOn = false;
            struct timespec delay = {0, IDLE_MS * NS_PER_MS};
            nanosleep(&delay, NULL);
            continue;
        } else {
            ledOn = !ledOn;
            Led_set(ledOn);
            int halfPeriodNs = NS_PER_S / (2 * frequency);
            struct timespec delay = {0, halfPeriodNs};
            nanosleep(&delay, NULL);
        }
    }

    return NULL;
}

void Flasher_init(void) {
    s_isRunning = true;
    if (pthread_create(&s_thread, NULL, flasherThread, NULL) != 0) {
        fprintf(stderr, "pthread_create failed\n");
    }
}

void Flasher_setFreq(int frequencyHz) {
    if (frequencyHz < MIN_FREQ_HZ) {
        frequencyHz = MIN_FREQ_HZ;
    } else if (frequencyHz > MAX_FREQ_HZ) {
        frequencyHz = MAX_FREQ_HZ;
    }

    pthread_mutex_lock(&s_lock);
    if (frequencyHz == s_frequency) {
        pthread_mutex_unlock(&s_lock);
        return;
    } else {
        s_frequency = frequencyHz;
        pthread_mutex_unlock(&s_lock);
    }
}

int Flasher_getFreq(void) {
    pthread_mutex_lock(&s_lock);
    int frequency = s_frequency;
    pthread_mutex_unlock(&s_lock);
    return frequency;
}

void Flasher_cleanup(void) {
    pthread_mutex_lock(&s_lock);
    s_isRunning = false;
    pthread_mutex_unlock(&s_lock);
    pthread_join(s_thread, NULL);
    Led_set(false);
}
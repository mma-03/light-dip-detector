#include "knob.h"
#include "flasher.h"
#include "encoder.h"
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define STATE_A0_B0 0 // both switches off
#define STATE_A0_B1 1 // A off, B on
#define STATE_A1_B0 2 // A on, B off
#define STATE_A1_B1 3 // both on = rest, between clicks

#define POLL_MS 1
#define NS_PER_MS 1000000
#define STEP_THRESHOLD 2

static bool s_isRunning = false;
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_thread;

static bool isRunning(void) {
    pthread_mutex_lock(&s_lock);
    bool running = s_isRunning;
    pthread_mutex_unlock(&s_lock);
    return running;
}

static int stepDirection(int previous, int current) {
    // one step clockwise
    if ((previous == STATE_A1_B1 && current == STATE_A0_B1) ||
        (previous == STATE_A0_B1 && current == STATE_A0_B0) ||
        (previous == STATE_A0_B0 && current == STATE_A1_B0) ||
        (previous == STATE_A1_B0 && current == STATE_A1_B1)) {
            return 1;
        }
    
    // one step counter clockwise
    if ((previous == STATE_A1_B1 && current == STATE_A1_B0) ||
        (previous == STATE_A1_B0 && current == STATE_A0_B0) ||
        (previous == STATE_A0_B0 && current == STATE_A0_B1) ||
        (previous == STATE_A0_B1 && current == STATE_A1_B1)) {
            return -1;
        }
    
    // no change or direction unkown
    return 0;
}

static void *knobThread(void *arg) {
    (void)arg;

    int previous = Encoder_readState();
    int steps = 0;

    while (isRunning()) {
        int current = Encoder_readState();
        if (current >= 0 && current != previous) {
            if (previous >= 0) {
                steps += stepDirection(previous, current);
            }

            previous = current;

            if(current == STATE_A1_B1) {
                if (steps >= STEP_THRESHOLD) {
                    Flasher_setFreq(Flasher_getFreq() + 1);
                } else if (steps <= -STEP_THRESHOLD) {
                    Flasher_setFreq(Flasher_getFreq() - 1);
                }
                steps = 0;
            }
        }
        struct timespec delay = {0, POLL_MS * NS_PER_MS};
        nanosleep(&delay, NULL);
    }

    return NULL;
}

void Knob_init(void) {
    s_isRunning = true;
    if (pthread_create(&s_thread, NULL, knobThread, NULL) != 0) {
        fprintf(stderr, "pthread_create() failed\n");
    }
}

void Knob_cleanup(void) {
    pthread_mutex_lock(&s_lock);
    s_isRunning = false;
    pthread_mutex_unlock(&s_lock);
    pthread_join(s_thread, NULL);
}
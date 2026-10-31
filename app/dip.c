#include "dip.h"
#include <stdbool.h>

#define DIP_THRESHOLD_V 0.10
#define HYSTERESIS_V 0.03
#define REARM_OFFSET_V (DIP_THRESHOLD_V - HYSTERESIS_V)

static bool s_inDip = false;

int Dip_countDips(const double *samples, int size, double avg) {
    int dipCount = 0;

    for (int i = 0; i < size; i++) {
        if (!s_inDip) {
            if (samples[i] <= avg - DIP_THRESHOLD_V) {
                s_inDip = true;
                dipCount++;
            }
        } else {
            if (samples[i] >= avg - REARM_OFFSET_V) {
                s_inDip = false;
            }
        }
    }

    return dipCount;
}
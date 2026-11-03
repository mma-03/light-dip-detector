#include "encoder.h"
#include <stdlib.h>
#include <gpiod.h>
#include <stdio.h>

#define ENCODER_LINE_A 7
#define ENCODER_LINE_B 8
#define ENCODER_CHIP "/dev/gpiochip3"

static struct gpiod_chip *s_chip = NULL;
static struct gpiod_line_request *s_request = NULL;

bool Encoder_init(void) {
    s_chip = gpiod_chip_open(ENCODER_CHIP);
    if (s_chip == NULL) {
        perror("Error opening gpiochip3");
        return false;
    }

    struct gpiod_line_settings *settings = NULL;
    settings = gpiod_line_settings_new();
    if (settings == NULL) {
        perror("Error making settings");
        gpiod_chip_close(s_chip);
        s_chip = NULL;
        return false;
    }
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT) == -1) {
        perror("Error setting line direction");
        gpiod_line_settings_free(settings);
        gpiod_chip_close(s_chip);
        s_chip = NULL;
        return false;
    }

    if (gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP) == -1) {
        perror("Error setting pull-up");
        gpiod_line_settings_free(settings);
        gpiod_chip_close(s_chip);
        s_chip = NULL;
        return false;
    }

    struct gpiod_line_config *lineCfg = NULL;
    lineCfg = gpiod_line_config_new();
    if (lineCfg == NULL) {
        perror("Error making configurations");
        gpiod_line_settings_free(settings);
        gpiod_chip_close(s_chip);
        s_chip = NULL;
        return false;
    }

    unsigned int offsets[] = {ENCODER_LINE_A, ENCODER_LINE_B};
    if (gpiod_line_config_add_line_settings(lineCfg, offsets, 2, settings) == -1) {
        perror("Error adding settings to configuration");
        gpiod_line_config_free(lineCfg);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(s_chip);
        s_chip = NULL;
        return false;
    }

    s_request = gpiod_chip_request_lines(s_chip, NULL, lineCfg);
    gpiod_line_config_free(lineCfg);
    gpiod_line_settings_free(settings);
    if (s_request == NULL) {
        perror("Error requesting line");
        gpiod_chip_close(s_chip);
        s_chip = NULL;
        return false;
    }

    return true;
}

int Encoder_readState(void) {
    if(s_request == NULL) {
        return -1;
    }

    int result_a;
    result_a = gpiod_line_request_get_value(s_request, ENCODER_LINE_A);

    int result_b;
    result_b = gpiod_line_request_get_value(s_request, ENCODER_LINE_B);

    if (result_a == -1 || result_b == -1) {
        perror("Error reading pin");
        return -1;
    }

    return (result_a << 1) | result_b;
}

void Encoder_cleanup(void) {
    if (s_request != NULL) {
        gpiod_line_request_release(s_request);
        s_request = NULL;
    }

    if (s_chip != NULL) {
        gpiod_chip_close(s_chip);
        s_chip = NULL;
    }
}
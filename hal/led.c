#include "led.h"
#include <stdlib.h>
#include <gpiod.h>
#include <stdio.h>

#define LED_LINE_NUMBER 16
#define LED_CHIP "/dev/gpiochip3"

static struct gpiod_chip *s_chip = NULL;
static struct gpiod_line_request *s_request = NULL;

bool Led_init(void) {
    s_chip = gpiod_chip_open(LED_CHIP);
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
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) == -1) {
        perror("Error setting line direction");
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

    unsigned int led_lineNumber = LED_LINE_NUMBER;
    if (gpiod_line_config_add_line_settings(lineCfg, &led_lineNumber, 1, settings) == -1) {
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

void Led_set(bool on) {
    if(s_request == NULL) {
        return;
    }

    int result;
    if (on) {
        result = gpiod_line_request_set_value(s_request, LED_LINE_NUMBER, GPIOD_LINE_VALUE_ACTIVE);
    } else {
        result = gpiod_line_request_set_value(s_request, LED_LINE_NUMBER, GPIOD_LINE_VALUE_INACTIVE);
    }

    if (result == -1) {
        perror("Error setting LED value");
        return;
    }
}

void Led_cleanup(void) {
    if (s_request != NULL) {
        gpiod_line_request_release(s_request);
        s_request = NULL;
    }

    if (s_chip != NULL) {
        gpiod_chip_close(s_chip);
        s_chip = NULL;
    }
}
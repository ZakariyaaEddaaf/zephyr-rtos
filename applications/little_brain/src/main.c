/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "led_utils.h"
#include "switch_utils.h"
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 500

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static void board_init();
static void board_run();

void board_init() {
    // setup led0 and led1
    led_setup(&led0);
    led_setup(&led1);
    // setup switch0
    switch_setup(&sw0);
}

void board_run() {

    static bool leds_active = false;
    static bool was_pressed = false;

    bool pressed = switch_state(&sw0);

    if (was_pressed && !pressed) {
        leds_active = !leds_active;
        if (!leds_active) {
            led_off(&led0);
            led_off(&led1);
        }
    }
    was_pressed = pressed;

    if (leds_active) {
        led_toggle(&led0);
        led_toggle(&led1);
        k_msleep(500);
    } else {
        k_msleep(20);
    }
}

int main(void) {
    board_init();
    while (1) {
        board_run();
    }
    return 0;
}
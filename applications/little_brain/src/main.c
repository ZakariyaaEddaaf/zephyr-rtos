/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define MY_STACK_SIZE 1024
#define PRIORITY 5

#define LED0_SLEEP_MS 500
#define LED1_SLEEP_MS 1000

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

static void toggle_led0(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    if (!gpio_is_ready_dt(&led0)) {
        LOG_ERR("led0 GPIO port not ready");
        return;
    }

    if (gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE) < 0) {
        LOG_ERR("failed to configure led0");
        return;
    }

    while (1) {
        gpio_pin_toggle_dt(&led0);
        k_msleep(LED0_SLEEP_MS);
    }
}

static void toggle_led1(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    if (!gpio_is_ready_dt(&led1)) {
        LOG_ERR("led1 GPIO port not ready");
        return;
    }

    if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE) < 0) {
        LOG_ERR("failed to configure led1");
        return;
    }

    while (1) {
        gpio_pin_toggle_dt(&led1);
        k_msleep(LED1_SLEEP_MS);
    }
}

K_THREAD_DEFINE(toggle_led0_tid, MY_STACK_SIZE, toggle_led0, NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(toggle_led1_tid, MY_STACK_SIZE, toggle_led1, NULL, NULL, NULL,
                PRIORITY, 0, 0);
#include "led_utils.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(leds, LOG_LEVEL_INF);

#define LED_TICK_MS 100
#define SLOW_TICKS 5
#define FAST_TICKS 1

enum led_id { LED_RED, LED_ORANGE, LED_GREEN, LED_NUM };
enum led_mode { M_OFF, M_ON, M_SLOW, M_FAST };

static const struct gpio_dt_spec leds[LED_NUM] = {
    [LED_RED] = GPIO_DT_SPEC_GET(DT_NODELABEL(led_red), gpios),
    [LED_ORANGE] = GPIO_DT_SPEC_GET(DT_NODELABEL(led_orange), gpios),
    [LED_GREEN] = GPIO_DT_SPEC_GET(DT_NODELABEL(led_green), gpios),
};

static const uint8_t patterns[LED_PATTERN_COUNT][LED_NUM] = {
    [LED_PATTERN_OFF] = {M_OFF, M_OFF, M_OFF},
    [LED_PATTERN_WAIT_POWER] = {M_OFF, M_SLOW, M_OFF},
    [LED_PATTERN_STARTING] = {M_OFF, M_ON, M_OFF},
    [LED_PATTERN_RUNNING] = {M_OFF, M_OFF, M_ON},
    [LED_PATTERN_WARNING] = {M_OFF, M_SLOW, M_ON},
    [LED_PATTERN_FAULT] = {M_ON, M_OFF, M_OFF},
    [LED_PATTERN_LOCKOUT] = {M_FAST, M_OFF, M_OFF},
    [LED_PATTERN_CONFIG_ERROR] = {M_SLOW, M_SLOW, M_OFF},
};

static atomic_t current_pattern = ATOMIC_INIT(LED_PATTERN_OFF);
static uint32_t tick;

static void led_timer_handler(struct k_timer *timer) {
    ARG_UNUSED(timer);

    enum led_pattern p = (enum led_pattern)atomic_get(&current_pattern);
    bool slow_on = ((tick / SLOW_TICKS) & 1U) != 0U;
    bool fast_on = ((tick / FAST_TICKS) & 1U) != 0U;

    tick++;

    for (int i = 0; i < LED_NUM; i++) {
        bool on;

        switch (patterns[p][i]) {
        case M_ON:
            on = true;
            break;
        case M_SLOW:
            on = slow_on;
            break;
        case M_FAST:
            on = fast_on;
            break;
        default:
            on = false;
            break;
        }
        gpio_pin_set_dt(&leds[i], on ? 1 : 0);
    }
}

K_TIMER_DEFINE(led_timer, led_timer_handler, NULL);

int leds_init(void) {
    for (int i = 0; i < LED_NUM; i++) {
        if (!gpio_is_ready_dt(&leds[i])) {
            LOG_ERR("LED %d GPIO not ready", i);
            return -ENODEV;
        }

        int ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
        if (ret < 0) {
            LOG_ERR("LED %d config failed (%d)", i, ret);
            return ret;
        }
    }

    k_timer_start(&led_timer, K_NO_WAIT, K_MSEC(LED_TICK_MS));
    return 0;
}

void leds_set_pattern(enum led_pattern pattern) {
    if (pattern < LED_PATTERN_COUNT) {
        atomic_set(&current_pattern, pattern);
    }
}
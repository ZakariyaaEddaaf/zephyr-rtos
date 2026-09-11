#include "led_utils.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

void led_setup(const struct gpio_dt_spec *led) {
    if (!gpio_is_ready_dt(led)) {
        return;
    }
    gpio_pin_configure_dt(led, GPIO_OUTPUT_INACTIVE);
}

void led_on(const struct gpio_dt_spec *led) {
    gpio_pin_set_dt(led, 1);
}

void led_off(const struct gpio_dt_spec *led) {
    gpio_pin_set_dt(led, 0);
}

void led_toggle(const struct gpio_dt_spec *led) {
    gpio_pin_toggle_dt(led);
}
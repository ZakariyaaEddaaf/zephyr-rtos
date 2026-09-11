#ifndef LED_UTILS_H_
#define LED_UTILS_H_

#include <zephyr/drivers/gpio.h>

void led_setup(const struct gpio_dt_spec *led);
void led_on(const struct gpio_dt_spec *led);
void led_off(const struct gpio_dt_spec *led);
void led_toggle(const struct gpio_dt_spec *led);

#endif // LED_UTILS_H_
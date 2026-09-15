#ifndef _LED_UTILS_H_
#define _LED_UTILS_H_

#include <zephyr/drivers/gpio.h>

void led_setup(const struct gpio_dt_spec *led);
void led_on(const struct gpio_dt_spec *led);
void led_off(const struct gpio_dt_spec *led);
void led_toggle(const struct gpio_dt_spec *led);

#endif // _LED_UTILS_H_
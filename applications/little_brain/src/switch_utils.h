#ifndef _SWITCH_UTILS_H_
#define _SWITCH_UTILS_H_

#include <stdbool.h>
#include <zephyr/drivers/gpio.h>

void switch_setup(const struct gpio_dt_spec *sw);
bool switch_state(const struct gpio_dt_spec *sw);

#endif // _SWITCH_UTILS_H_
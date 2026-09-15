#include "switch_utils.h"

#include <zephyr/kernel.h>

void switch_setup(const struct gpio_dt_spec *sw) {
    if (!gpio_is_ready_dt(sw)) {
        return;
    }
    gpio_pin_configure_dt(sw, GPIO_INPUT);
}

bool switch_state(const struct gpio_dt_spec *sw) {
    return gpio_pin_get_dt(sw) ==
           1; /* 1 = pressed (active level from devicetree) */
}
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

static const struct gpio_dt_spec led0 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios); /* green  */
static const struct gpio_dt_spec led1 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios); /* red    */
static const struct gpio_dt_spec led2 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios); /* orange */

int main(void) {

    printk("LTC4281 HotSwap Controller : %s\n", CONFIG_BOARD);

    if (!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) ||
        !gpio_is_ready_dt(&led2)) {
        printk("LEDs not ready\n");
        return 0;
    }

    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);

    while (1) {
        gpio_pin_toggle_dt(&led0);
        gpio_pin_toggle_dt(&led1);
        gpio_pin_toggle_dt(&led2);
        k_msleep(200);
    }

    return 0;
}
/*
* Copyright (c) 2016 Intel Corporation
*
* SPDX-License-Identifier: Apache-2.0
*/

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led1)
#define SW0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec led    = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static struct gpio_callback button_cb_data;

static void button_changed(const struct device *dev,
			   struct gpio_callback *cb, uint32_t pins)
{
	/* gpio_pin_get_dt returns the logical level:
	 * 1 = pressed, 0 = released (GPIO_ACTIVE_LOW is handled for you) */
	int pressed = gpio_pin_get_dt(&button);
	gpio_pin_set_dt(&led, pressed);
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button) || !gpio_is_ready_dt(&led)) {
		printk("GPIO not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		printk("Failed to configure LED: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Failed to configure button: %d\n", ret);
		return 0;
	}

	/* fire on both press and release */
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		printk("Failed to configure interrupt: %d\n", ret);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_changed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	printk("Ready — hold the button to light the LED\n");

	while (1) {
		k_msleep(100);
	}
	return 0;
}
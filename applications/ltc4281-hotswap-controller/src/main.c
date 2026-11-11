#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "adc_utils.h"

int main(void) {
    int32_t vin, vout;

    if (adc_setup()) {
        printk("ADC setup failed\n");
        return -1;
    }

    while (1) {
        if (adc_vin_mon(&vin) == 0 && adc_vout_mon(&vout) == 0) {
            printk("VIN = %d mV  VOUT = %d mV\n", vin, vout);
        } else {
            printk("ADC read failed\n");
        }
        k_msleep(1000);
    }

    return 0;
}
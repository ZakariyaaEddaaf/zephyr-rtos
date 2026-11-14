#include "adc_utils.h"
#include "hotswap_controller.hpp"
#include "tmp101_utils.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

static HotswapController hotswap;

static void app_init() {
    if (hotswap.Init() < 0) {
        LOG_ERR("Hotswap init failed");
    }
}

static void app_run() {

    int32_t vin, vout, temp;

    if (hotswap.GetVinMv(&vin) == 0) {
        LOG_INF("VIN:  %d mV", vin);
    }

    if (hotswap.GetVoutMv(&vout) == 0) {
        LOG_INF("VOUT: %d mV", vout);
    }

    if (hotswap.GetTempC(&temp) == 0) {
        LOG_INF("TEMP: %d C", temp);
    }

    k_sleep(K_MSEC(1000));
}

int main(void) {
    app_init();

    while (1) {
        app_run();
    }

    return 0;
}
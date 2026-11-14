// HotswapController.cpp
#include "hotswap_controller.hpp"
#include "adc_utils.h"
#include "tmp101_utils.h"
#include <errno.h>
#include <stdlib.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hotswap, LOG_LEVEL_DBG);

static const struct gpio_dt_spec hs_on =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), hs_on_gpios); /* hotswap ON/OFF */

static const struct gpio_dt_spec cfg_pins[3] = {
    GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), cfg_gpios,
                            0), /* CFG0 - PA15 */
    GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), cfg_gpios,
                            1), /* CFG1 - PA14 */
    GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), cfg_gpios,
                            2), /* CFG2 - PA13 */
};

#define CFG_LOW_POWER 0b001  /* CFG0 high, CFG1 low,  CFG2 low */
#define CFG_HIGH_POWER 0b010 /* CFG0 low,  CFG1 high, CFG2 low */

int HotswapController::Init() {

    if (adc_setup() < 0) {
        LOG_ERR("ADC setup failed");
    }

    if (tmp101_setup() < 0) {
        LOG_ERR("TMP101 setup failed");
    }

    if (!gpio_is_ready_dt(&hs_on)) {
        LOG_ERR("HS_ON GPIO not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&hs_on, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("HS_ON config failed (%d)", ret);
        return ret;
    }

    LOG_INF("Hotswap controller initialized (OFF)");
    return 0;
}

int HotswapController::EnableHotswap() {
    int ret = gpio_pin_set_dt(&hs_on, 1);
    if (ret < 0) {
        LOG_ERR("Failed to turn hotswap ON (%d)", ret);
        return ret;
    }
    LOG_INF("Hotswap ON");
    return 0;
}

int HotswapController::DisableHotswap() {
    int ret = gpio_pin_set_dt(&hs_on, 0);
    if (ret < 0) {
        LOG_ERR("Failed to turn hotswap OFF (%d)", ret);
        return ret;
    }
    LOG_INF("Hotswap OFF");
    return 0;
}

int HotswapController::GetVinMv(int32_t *vin_mv) {
    int32_t mv;

    int ret = adc_vin_mon(&mv);
    if (ret < 0) {
        LOG_ERR("VIN read failed (%d)", ret);
        return ret;
    }

    LOG_DBG("VIN: %d mV", mv);
    *vin_mv = mv;
    return 0;
}

int HotswapController::GetVoutMv(int32_t *vout_mv) {
    int32_t mv;

    int ret = adc_vout_mon(&mv);
    if (ret < 0) {
        LOG_ERR("VOUT read failed (%d)", ret);
        return ret;
    }

    LOG_DBG("VOUT: %d mV", mv);
    *vout_mv = mv;
    return 0;
}

int HotswapController::GetTempC(int32_t *temp_c) {
    int32_t temp_mc;

    int ret = tmp101_read(&temp_mc);
    if (ret < 0) {
        LOG_ERR("Temp read failed (%d)", ret);
        return ret;
    }

    int32_t whole = temp_mc / 1000;
    int32_t tenth = abs(temp_mc % 1000) / 100;

    LOG_DBG("Temp: %s%d.%d °C", (temp_mc < 0 && whole == 0) ? "-" : "", whole,
            tenth);

    *temp_c = whole;
    return 0;
}

int HotswapController::DetectPowerMode() {
    int cfg = 0;

    for (int i = 0; i < 3; i++) {
        if (!gpio_is_ready_dt(&cfg_pins[i])) {
            LOG_ERR("CFG%d GPIO device not ready", i);
            return -ENODEV;
        }

        int ret = gpio_pin_configure_dt(&cfg_pins[i], GPIO_INPUT);
        if (ret < 0) {
            LOG_ERR("CFG%d configure failed (%d)", i, ret);
            return ret;
        }

        int bit = gpio_pin_get_dt(&cfg_pins[i]);
        if (bit < 0) {
            LOG_ERR("CFG%d read failed (%d)", i, bit);
            return bit;
        }

        LOG_DBG("CFG%d (PA%d) = %s", i, cfg_pins[i].pin, bit ? "HIGH" : "LOW");
        cfg |= (bit << i);
    }

    switch (cfg) {
    case CFG_LOW_POWER:
        LOG_INF("Board config: LOW power (CFG2..0 = 0x%x)", cfg);
        return LOW_POWER;

    case CFG_HIGH_POWER:
        LOG_INF("Board config: HIGH power (CFG2..0 = 0x%x)", cfg);
        return HIGH_POWER;

    default:
        LOG_WRN("Unknown board config: CFG2..0 = %d%d%d", (cfg >> 2) & 1,
                (cfg >> 1) & 1, cfg & 1);
        return -EINVAL;
    }
}
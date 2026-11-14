#include "tmp101_utils.h"

#include <errno.h>
#include <stdbool.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(tmp101_utils, LOG_LEVEL_INF);

static const struct device *const tmp = DEVICE_DT_GET(DT_NODELABEL(tmp101));

static bool tmp_ready;

int tmp101_setup(void) {
    tmp_ready = device_is_ready(tmp);
    if (!tmp_ready) {
        LOG_ERR("TMP101 not ready");
        return -ENODEV;
    }

    LOG_INF("TMP101 ready");
    return 0;
}

int tmp101_read(int32_t *temp_mc) {
    struct sensor_value val;
    int ret;

    if (!tmp_ready) {
        return -ENODEV;
    }

    ret = sensor_sample_fetch(tmp);
    if (ret) {
        LOG_ERR("sample fetch failed: %d", ret);
        return ret;
    }

    ret = sensor_channel_get(tmp, SENSOR_CHAN_AMBIENT_TEMP, &val);
    if (ret) {
        LOG_ERR("channel get failed: %d", ret);
        return ret;
    }

    *temp_mc = (int32_t)sensor_value_to_milli(&val);
    LOG_DBG("temp: %d mC", *temp_mc);
    return 0;
}
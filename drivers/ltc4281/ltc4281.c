#define DT_DRV_COMPAT adi_ltc4281

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include "ltc4281.h"
#include "ltc4281_regs.h"

LOG_MODULE_REGISTER(ltc4281, LOG_LEVEL_DBG);

#define LTC4281_MUX_SETTLE_MS 200

struct ltc4281_config {
    struct i2c_dt_spec i2c;
    struct gpio_dt_spec on_gpio;
    struct gpio_dt_spec wp_gpio;
    struct gpio_dt_spec alert_gpio;
};

struct ltc4281_data {
    struct k_mutex lock;
    bool ready;
};

static int ltc4281_read16(const struct device *dev, uint8_t reg,
                          uint16_t *val) {
    const struct ltc4281_config *cfg = dev->config;
    uint8_t buf[2];
    int ret;

    ret = i2c_burst_read_dt(&cfg->i2c, reg, buf, sizeof(buf));
    if (ret < 0) {
        LOG_ERR("Read reg 0x%02x failed (%d)", reg, ret);
        return ret;
    }

    *val = sys_get_be16(buf);
    return 0;
}

int ltc4281_init(const struct device *dev) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    uint8_t reg_val;
    int ret;

    if (!i2c_is_ready_dt(&cfg->i2c)) {
        return -ENODEV;
    }

    if (cfg->on_gpio.port != NULL) {
        if (!gpio_is_ready_dt(&cfg->on_gpio)) {
            LOG_ERR("ON GPIO not ready");
            return -ENODEV;
        }
        ret = gpio_pin_configure_dt(&cfg->on_gpio, GPIO_OUTPUT_INACTIVE);
        if (ret < 0) {
            LOG_ERR("Failed to configure ON GPIO (%d)", ret);
            return ret;
        }
    }

    if (cfg->wp_gpio.port != NULL) {
        if (!gpio_is_ready_dt(&cfg->wp_gpio)) {
            LOG_ERR("WP GPIO not ready");
            return -ENODEV;
        }
        ret = gpio_pin_configure_dt(&cfg->wp_gpio, GPIO_OUTPUT_ACTIVE);
        if (ret < 0) {
            LOG_ERR("Failed to configure WP GPIO (%d)", ret);
            return ret;
        }
    }

    if (cfg->alert_gpio.port != NULL) {
        if (!gpio_is_ready_dt(&cfg->alert_gpio)) {
            LOG_ERR("ALERT GPIO not ready");
            return -ENODEV;
        }
        ret = gpio_pin_configure_dt(&cfg->alert_gpio, GPIO_INPUT);
        if (ret < 0) {
            LOG_ERR("Failed to configure ALERT GPIO (%d)", ret);
            return ret;
        }
    }

    k_mutex_lock(&data->lock, K_FOREVER);
    data->ready = false;

    reg_val = LTC4281_CTRL_ON_FAULT_MASK | LTC4281_CTRL_ON_ACTIVE_HIGH |
              LTC4281_CTRL_OV_RETRY | LTC4281_CTRL_UV_RETRY;
    ret = i2c_reg_write_byte_dt(&cfg->i2c, LTC4281_REG_CONTROL_LSB, reg_val);
    if (ret < 0) {
        LOG_ERR("Failed to write CONTROL_LSB register, err: %d", ret);
        goto out;
    }

    reg_val = FIELD_PREP(LTC4281_CTRL_VIN_MODE, LTC4281_VIN_MODE_24V);
    ret = i2c_reg_write_byte_dt(&cfg->i2c, LTC4281_REG_CONTROL_MSB, reg_val);
    if (ret < 0) {
        LOG_ERR("Failed to write CONTROL_MSB register, err: %d", ret);
        goto out;
    }

    reg_val = FIELD_PREP(LTC4281_ILIM_FOLDBACK_MODE, LTC4281_VIN_MODE_24V);
    ret = i2c_reg_update_byte_dt(&cfg->i2c, LTC4281_REG_ILIM_ADJUST,
                                 LTC4281_ILIM_FOLDBACK_MODE, reg_val);
    if (ret < 0) {
        LOG_ERR("Failed to write ILIM_ADJUST register, err: %d", ret);
        goto out;
    }

    ret = i2c_reg_write_byte_dt(&cfg->i2c, LTC4281_REG_FAULT_LOG, 0);
    if (ret < 0) {
        LOG_ERR("Failed to write FAULT_LOG register, err: %d", ret);
        goto out;
    }

    ret = i2c_reg_write_byte_dt(&cfg->i2c, LTC4281_REG_ADC_ALERT_LOG, 0);
    if (ret < 0) {
        LOG_ERR("Failed to write ADC_ALERT_LOG register, err: %d", ret);
        goto out;
    }

    data->ready = true;
    LOG_INF("Init done at 0x%02x", cfg->i2c.addr);

out:
    k_mutex_unlock(&data->lock);
    return ret;
}

int ltc4281_on(const struct device *dev) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = data->ready ? gpio_pin_set_dt(&cfg->on_gpio, 1) : -EPERM;
    k_mutex_unlock(&data->lock);

    return ret;
}

int ltc4281_off(const struct device *dev) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = data->ready ? gpio_pin_set_dt(&cfg->on_gpio, 0) : -EPERM;
    k_mutex_unlock(&data->lock);

    return ret;
}

int ltc4281_fet_on(const struct device *dev) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = data->ready
              ? i2c_reg_update_byte_dt(&cfg->i2c, LTC4281_REG_CONTROL_LSB,
                                       LTC4281_CTRL_FET_ON, LTC4281_CTRL_FET_ON)
              : -EPERM;
    k_mutex_unlock(&data->lock);

    return ret;
}

int ltc4281_fet_off(const struct device *dev) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = data->ready
              ? i2c_reg_update_byte_dt(&cfg->i2c, LTC4281_REG_CONTROL_LSB,
                                       LTC4281_CTRL_FET_ON, 0)
              : -EPERM;
    k_mutex_unlock(&data->lock);

    return ret;
}

int ltc4281_get_vsource(const struct device *dev, int32_t *mv) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    uint8_t ilim;
    uint16_t code;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);

    if (!data->ready) {
        ret = -EPERM;
        goto out;
    }

    ret = i2c_reg_read_byte_dt(&cfg->i2c, LTC4281_REG_ILIM_ADJUST, &ilim);
    if (ret < 0) {
        goto out;
    }
    if (!(ilim & LTC4281_ILIM_VSOURCE_SELECT)) {
        ret = i2c_reg_write_byte_dt(&cfg->i2c, LTC4281_REG_ILIM_ADJUST,
                                    ilim | LTC4281_ILIM_VSOURCE_SELECT);
        if (ret < 0) {
            goto out;
        }
        k_sleep(K_MSEC(LTC4281_MUX_SETTLE_MS));
    }

    ret = ltc4281_read16(dev, LTC4281_REG_VSOURCE, &code);
    if (ret < 0) {
        goto out;
    }

    *mv = (int32_t)(((uint64_t)code * LTC4281_VFS_24V_MV) /
                    LTC4281_ADC_MAX_16BIT);

out:
    k_mutex_unlock(&data->lock);
    return ret;
}

int ltc4281_get_vdd(const struct device *dev, int32_t *mv) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    uint8_t ilim;
    uint16_t code;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);

    if (!data->ready) {
        ret = -EPERM;
        goto out;
    }

    ret = i2c_reg_read_byte_dt(&cfg->i2c, LTC4281_REG_ILIM_ADJUST, &ilim);
    if (ret < 0) {
        goto out;
    }
    if (ilim & LTC4281_ILIM_VSOURCE_SELECT) {
        ret = i2c_reg_write_byte_dt(&cfg->i2c, LTC4281_REG_ILIM_ADJUST,
                                    ilim & ~LTC4281_ILIM_VSOURCE_SELECT);
        if (ret < 0) {
            goto out;
        }
        k_sleep(K_MSEC(LTC4281_MUX_SETTLE_MS));
    }

    ret = ltc4281_read16(dev, LTC4281_REG_VSOURCE, &code);
    if (ret < 0) {
        goto out;
    }

    *mv = (int32_t)(((uint64_t)code * LTC4281_VFS_24V_MV) /
                    LTC4281_ADC_MAX_16BIT);

out:
    k_mutex_unlock(&data->lock);
    return ret;
}

int ltc4281_get_vsense(const struct device *dev, int32_t *uv) {
    struct ltc4281_data *data = dev->data;
    uint16_t code;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);

    ret = ltc4281_read16(dev, LTC4281_REG_VSENSE, &code);
    if (ret == 0) {
        *uv = (int32_t)(((uint64_t)code * LTC4281_VSENSE_FS_UV) /
                        LTC4281_ADC_MAX_16BIT);
    }

    k_mutex_unlock(&data->lock);
    return ret;
}

int ltc4281_set_ilim_adjust(const struct device *dev, uint8_t ilim) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    int ret;

    if (ilim > 7U) {
        return -EINVAL;
    }

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = i2c_reg_update_byte_dt(&cfg->i2c, LTC4281_REG_ILIM_ADJUST,
                                 LTC4281_ILIM_LIMIT,
                                 FIELD_PREP(LTC4281_ILIM_LIMIT, ilim));
    k_mutex_unlock(&data->lock);

    return ret;
}

int ltc4281_get_current(const struct device *dev, uint32_t rsense_uohm,
                        int32_t *ma) {
    int32_t uv;
    int ret;

    if (rsense_uohm == 0U) {
        return -EINVAL;
    }

    ret = ltc4281_get_vsense(dev, &uv);
    if (ret < 0) {
        return ret;
    }

    /* I [mA] = Vsense [uV] * 1000 / Rsense [uOhm] */
    *ma = (int32_t)(((int64_t)uv * 1000) / rsense_uohm);
    return 0;
}

int ltc4281_get_error_flags(const struct device *dev, uint8_t *flags) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = i2c_reg_read_byte_dt(&cfg->i2c, LTC4281_REG_FAULT_LOG, flags);
    k_mutex_unlock(&data->lock);

    if (ret < 0) {
        LOG_ERR("Failed to read FAULT_LOG register, err: %d", ret);
        return ret;
    }

    if (*flags != 0U) {
        LOG_DBG("FAULT_LOG = 0x%02x", *flags);
        if (*flags & LTC4281_FAULT_OV) {
            LOG_DBG("  - overvoltage");
        }
        if (*flags & LTC4281_FAULT_UV) {
            LOG_DBG("  - undervoltage");
        }
        if (*flags & LTC4281_FAULT_OC) {
            LOG_DBG("  - overcurrent");
        }
        if (*flags & LTC4281_FAULT_POWER_BAD) {
            LOG_DBG("  - power bad");
        }
        if (*flags & LTC4281_FAULT_FET_SHORT) {
            LOG_DBG("  - FET short");
        }
        if (*flags & LTC4281_FAULT_FET_BAD) {
            LOG_DBG("  - FET bad");
        }
    }

    return 0;
}

int ltc4281_get_status_register(const struct device *dev, uint16_t *status) {
    struct ltc4281_data *data = dev->data;
    uint16_t val;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = ltc4281_read16(dev, LTC4281_REG_STATUS_LSB, &val);
    k_mutex_unlock(&data->lock);

    if (ret < 0) {
        return ret;
    }

    *status = BSWAP_16(val);

    LOG_DBG("STATUS = 0x%04x", *status);
    if (*status & LTC4281_STATUS_OV) {
        LOG_DBG("  - overvoltage (input above OV threshold)");
    }
    if (*status & LTC4281_STATUS_UV) {
        LOG_DBG("  - undervoltage (input below UV threshold)");
    }
    if (*status & LTC4281_STATUS_OC_COOLDOWN) {
        LOG_DBG("  - overcurrent cool-down");
    }
    if (*status & LTC4281_STATUS_POWER_GOOD) {
        LOG_DBG("  - power good");
    }
    if (*status & LTC4281_STATUS_ON_PIN) {
        LOG_DBG("  - ON pin high");
    }
    if (*status & LTC4281_STATUS_FET_SHORT) {
        LOG_DBG("  - FET short");
    }
    if (*status & LTC4281_STATUS_FET_BAD_COOLDOWN) {
        LOG_DBG("  - FET bad cool-down");
    }
    if (*status & LTC4281_STATUS_ON) {
        LOG_DBG("  - FET commanded on");
    }

    return 0;
}

int ltc4281_clear_faults(const struct device *dev) {
    const struct ltc4281_config *cfg = dev->config;
    struct ltc4281_data *data = dev->data;
    int ret;

    k_mutex_lock(&data->lock, K_FOREVER);
    ret = i2c_reg_write_byte_dt(&cfg->i2c, LTC4281_REG_FAULT_LOG, 0);
    k_mutex_unlock(&data->lock);

    if (ret < 0) {
        LOG_ERR("Failed to write FAULT_LOG register, err: %d", ret);
    }
    return ret;
}

#define LTC4281_DEFINE(inst)                                                   \
    static struct ltc4281_data ltc4281_data_##inst = {                         \
        .lock = Z_MUTEX_INITIALIZER(ltc4281_data_##inst.lock),                 \
    };                                                                         \
    static const struct ltc4281_config ltc4281_config_##inst = {               \
        .i2c = I2C_DT_SPEC_INST_GET(inst),                                     \
        .on_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, on_gpios, {0}),              \
        .wp_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, wp_gpios, {0}),              \
        .alert_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, alert_gpios, {0}),        \
    };                                                                         \
    DEVICE_DT_INST_DEFINE(inst, NULL, NULL, &ltc4281_data_##inst,              \
                          &ltc4281_config_##inst, POST_KERNEL, 80, NULL);

DT_INST_FOREACH_STATUS_OKAY(LTC4281_DEFINE)
#include "adc_utils.h"

#include <errno.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(adc_utils, LOG_LEVEL_INF);

#define VIN_DIV_FULL_OHMS 101000  /* R69 91K + R70 10K */
#define VIN_DIV_OUT_OHMS 10000    /* R70 */
#define VOUT_DIV_FULL_OHMS 101000 /* R67 91K + R68 10K */
#define VOUT_DIV_OUT_OHMS 10000   /* R68 */

#define ADC_SAMPLES 8

static const struct adc_dt_spec vin_adc =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), vin);
static const struct adc_dt_spec vout_adc =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), vout);

static int adc_read_mv(const struct adc_dt_spec *spec, int32_t full,
                       int32_t out, int32_t *mv) {
    int16_t raw;
    int32_t sum = 0;
    struct adc_sequence seq = {
        .buffer = &raw,
        .buffer_size = sizeof(raw),
    };
    int ret;

    /* SAM0: setup selects the input mux, so do it before every read */
    ret = adc_channel_setup_dt(spec);
    if (ret) {
        LOG_ERR("ch%d: channel setup failed: %d", spec->channel_id, ret);
        return ret;
    }

    ret = adc_sequence_init_dt(spec, &seq);
    if (ret) {
        LOG_ERR("ch%d: sequence init failed: %d", spec->channel_id, ret);
        return ret;
    }

    /* Discard first sample after mux switch */
    ret = adc_read_dt(spec, &seq);
    if (ret) {
        LOG_ERR("ch%d: dummy read failed: %d", spec->channel_id, ret);
        return ret;
    }

    for (int i = 0; i < ADC_SAMPLES; i++) {
        ret = adc_read_dt(spec, &seq);
        if (ret) {
            LOG_ERR("ch%d: read %d failed: %d", spec->channel_id, i, ret);
            return ret;
        }
        sum += (raw < 0) ? 0 : raw;
    }

    int32_t val = sum / ADC_SAMPLES;

    LOG_DBG("ch%d: raw avg %d", spec->channel_id, val);

    ret = adc_raw_to_millivolts_dt(spec, &val);
    if (ret) {
        LOG_ERR("ch%d: raw to mV failed: %d", spec->channel_id, ret);
        return ret;
    }

    *mv = (int32_t)(((int64_t)val * full) / out);

    LOG_DBG("ch%d: pin %d mV, scaled %d mV", spec->channel_id, val, *mv);
    return 0;
}

int adc_setup(void) {
    if (!adc_is_ready_dt(&vin_adc)) {
        LOG_ERR("VIN ADC not ready");
        return -ENODEV;
    }

    if (!adc_is_ready_dt(&vout_adc)) {
        LOG_ERR("VOUT ADC not ready");
        return -ENODEV;
    }

    LOG_INF("ADC ready");
    return 0;
}

int adc_vin_mon(int32_t *mv) {
    return adc_read_mv(&vin_adc, VIN_DIV_FULL_OHMS, VIN_DIV_OUT_OHMS, mv);
}

int adc_vout_mon(int32_t *mv) {
    return adc_read_mv(&vout_adc, VOUT_DIV_FULL_OHMS, VOUT_DIV_OUT_OHMS, mv);
}
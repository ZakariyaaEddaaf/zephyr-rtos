#include "adc_utils.h"

#include <errno.h>
#include <zephyr/drivers/adc.h>

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
        return ret;
    }

    ret = adc_sequence_init_dt(spec, &seq);
    if (ret) {
        return ret;
    }

    /* Discard first sample after mux switch */
    ret = adc_read_dt(spec, &seq);
    if (ret) {
        return ret;
    }

    for (int i = 0; i < ADC_SAMPLES; i++) {
        ret = adc_read_dt(spec, &seq);
        if (ret) {
            return ret;
        }
        sum += (raw < 0) ? 0 : raw;
    }

    int32_t val = sum / ADC_SAMPLES;

    ret = adc_raw_to_millivolts_dt(spec, &val);
    if (ret) {
        return ret;
    }

    *mv = (int32_t)(((int64_t)val * full) / out);
    return 0;
}

int adc_setup(void) {
    if (!adc_is_ready_dt(&vin_adc) || !adc_is_ready_dt(&vout_adc)) {
        return -ENODEV;
    }
    return 0;
}

int adc_vin_mon(int32_t *mv) {
    return adc_read_mv(&vin_adc, VIN_DIV_FULL_OHMS, VIN_DIV_OUT_OHMS, mv);
}

int adc_vout_mon(int32_t *mv) {
    return adc_read_mv(&vout_adc, VOUT_DIV_FULL_OHMS, VOUT_DIV_OUT_OHMS, mv);
}
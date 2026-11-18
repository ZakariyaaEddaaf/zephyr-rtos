#ifndef LTC4281_H_
#define LTC4281_H_

#include <stdint.h>
#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

int ltc4281_init(const struct device *dev);
int ltc4281_on(const struct device *dev);
int ltc4281_off(const struct device *dev);
int ltc4281_fet_on(const struct device *dev);
int ltc4281_fet_off(const struct device *dev);
int ltc4281_get_vsource(const struct device *dev, int32_t *mv);
int ltc4281_get_vdd(const struct device *dev, int32_t *mv);
int ltc4281_get_vsense(const struct device *dev, int32_t *uv);
int ltc4281_set_ilim_adjust(const struct device *dev, uint8_t ilim);
int ltc4281_get_current(const struct device *dev, uint32_t rsense_uohm,
                        int32_t *ma);
int ltc4281_get_error_flags(const struct device *dev, uint8_t *flags);
int ltc4281_get_status_register(const struct device *dev, uint16_t *status);
int ltc4281_clear_faults(const struct device *dev);

#ifdef __cplusplus
}
#endif

#endif /* LTC4281_H_ */
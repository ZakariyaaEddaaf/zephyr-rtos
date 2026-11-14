#ifndef ADC_UTILS_H_
#define ADC_UTILS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int adc_setup(void);
int adc_vin_mon(int32_t *mv);
int adc_vout_mon(int32_t *mv);

#ifdef __cplusplus
}
#endif

#endif /* ADC_UTILS_H_ */
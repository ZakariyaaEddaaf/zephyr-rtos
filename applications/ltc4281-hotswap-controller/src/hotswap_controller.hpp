#ifndef HOTSWAP_CONTROLLER_HPP
#define HOTSWAP_CONTROLLER_HPP

#include <stdint.h>

class HotswapController {
  public:
    enum PowerMode {
        LOW_POWER = 0,
        HIGH_POWER = 1,
    };
    HotswapController() = default;
    int Init();
    int EnableHotswap();
    int DisableHotswap();
    int GetVinMv(int32_t *vin_mv);
    int GetVoutMv(int32_t *vout_mv);
    int GetTempC(int32_t *temp_c);
    int DetectPowerMode();
};

#endif /* HOTSWAP_CONTROLLER_HPP */
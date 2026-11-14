#ifndef TMP101_UTILS_H
#define TMP101_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int tmp101_setup(void);
int tmp101_read(int32_t *temp_mc);

#ifdef __cplusplus
}
#endif

#endif /* TMP101_UTILS_H */
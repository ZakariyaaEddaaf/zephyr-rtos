#ifndef LED_UTILS_H_
#define LED_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

enum led_pattern {
    LED_PATTERN_OFF,
    LED_PATTERN_WAIT_POWER,   /* orange slow blink */
    LED_PATTERN_STARTING,     /* orange on */
    LED_PATTERN_RUNNING,      /* green on */
    LED_PATTERN_WARNING,      /* green on + orange slow blink */
    LED_PATTERN_FAULT,        /* red on */
    LED_PATTERN_LOCKOUT,      /* red fast blink */
    LED_PATTERN_CONFIG_ERROR, /* red + orange slow blink */
    LED_PATTERN_COUNT
};

int leds_init(void);
void leds_set_pattern(enum led_pattern pattern);

#ifdef __cplusplus
}
#endif

#endif /* LED_UTILS_H_ */
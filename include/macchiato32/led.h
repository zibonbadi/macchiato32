#ifndef M32_LED_H
#define M32_LED_H

#include "driver/ledc.h"

typedef struct m32_simple_led {
    uint8_t gpio;
    uint16_t duty;  // Read-only - Overwritten by the driver
    const uint8_t max_brightness;
    uint8_t brightness;
    // "Private" attributes. Don't touch these if you just wanna develop games (unless you're crazy)
    ledc_channel_config_t _channel_config;
} m32_simple_led_t;

typedef struct m32_rgb_led {
    // More of a convenience. RGB LEDs are literally just three LEDs glued together
    m32_simple_led_t r;
    m32_simple_led_t g;
    m32_simple_led_t b;
} m32_rgb_led_t;

m32_response_t m32_simple_led_init(m32_simple_led_t* led);
m32_response_t m32_simple_led_set(m32_simple_led_t* led, uint8_t brightness);
m32_response_t m32_rgb_led_init(m32_rgb_led_t* led);
m32_response_t m32_rgb_led_set(m32_rgb_led_t* led, uint8_t br_r, uint8_t br_g, uint8_t br_b);
#endif
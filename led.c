
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "macchiato32/m32_common.h"

#include "macchiato32/led.h"

m32_response_t m32_simple_led_init(m32_simple_led_t *led){
    led->_channel_config.channel = LEDC_CHANNEL_0;
    gpio_config_t gpio_cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << led->gpio
    };
    gpio_config(&gpio_cfg);
    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };

    // Configure LED Channel. Keep it in the struct, we need it later
    led->_channel_config.channel = LEDC_CHANNEL_0;
    led->_channel_config.duty = 0;
    led->_channel_config.gpio_num = led->gpio;
    led->_channel_config.speed_mode = LEDC_LOW_SPEED_MODE;
    led->_channel_config.timer_sel = LEDC_TIMER_0;

    ledc_timer_config(&timer);
    ledc_channel_config(&led->_channel_config);
    ledc_fade_func_install(0);

    return M32_OK;
};

m32_response_t m32_simple_led_set(m32_simple_led_t* led, uint8_t brightness){
        
    if(brightness >= led->max_brightness){ brightness = led->max_brightness; }
    
    led->duty = ((1 << LEDC_TIMER_13_BIT) - 1) -(81*(led->max_brightness-brightness));

    if(brightness == 0){ led->duty = 0; }

    ledc_set_duty(led->_channel_config.speed_mode, led->_channel_config.channel, led->duty);
    ledc_update_duty(led->_channel_config.speed_mode, led->_channel_config.channel);

    return M32_OK;
};

m32_response_t m32_rgb_led_init(m32_rgb_led_t* led){
    /*
     * Keep it simple - Treat RGB LEDs as three
     * individual m32_simple_led types
     */

    m32_simple_led_init(&led->r);
    m32_simple_led_init(&led->g);
    m32_simple_led_init(&led->b);

    return M32_OK;
};

m32_response_t m32_rgb_led_set(m32_rgb_led_t* led, uint8_t br_r, uint8_t br_g, uint8_t br_b){
    /*
     * Keep it simple - Treat RGB LEDs as three
     * individual m32_simple_led types
     */
    
    m32_simple_led_set(&led->r, br_r);
    m32_simple_led_set(&led->g, br_g);
    m32_simple_led_set(&led->b, br_b);

    return M32_OK;
};
#include <stdio.h>  // Remove later

#include <math.h>
#include "string.h"

// LCD,GPIO,SPI
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

// macchiato32 includes
#include "macchiato32/m32_common.h"
#include "macchiato32/display.h"
#include "macchiato32/graphic.h"
#include "macchiato32/led.h"

#include "macchiato32/display/st7789_commands.h"

static m32_display_t display = {
    .pixel_format = M32_PIXEL_FORMAT_RGB565,
    .width = CONFIG_M32_DISPLAY_WIDTH,
    .height = CONFIG_M32_DISPLAY_HEIGHT,
    .backlight_max_brightness = 100,  // Hardcode this elsewhere. *DON'T* the the user change this
};
static esp_lcd_panel_handle_t st7789_panel_handle;
static esp_lcd_panel_io_handle_t io_handle = NULL;

#ifdef CONFIG_M32_DISPLAY_USE_BACKLIGHT
static m32_simple_led_t backlight = {
    .gpio = CONFIG_M32_DISPLAY_PIN_BACKLIGHT,
    .max_brightness = 100,
    .brightness = 0
};
#endif
/*
 * Static/"Private" utility functions
*/

#ifdef CONFIG_M32_DISPLAY_USE_BACKLIGHT
static void _st7789_backlight_init(){
    // Use existing LED driver
    m32_simple_led_init(&backlight);
    m32_display_backlight_set(0);
}
#endif

/*
 *  "Public" API functions
*/


// SPI-LCD driver version
m32_display_t m32_display_init(){
    
    esp_err_t ret;                                         
    esp_lcd_panel_io_spi_config_t io_config = {                                             
        .dc_gpio_num = CONFIG_M32_DISPLAY_PIN_DC,
        .cs_gpio_num = CONFIG_M32_DISPLAY_PIN_SPI_CS,
        .pclk_hz = CONFIG_M32_DISPLAY_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = 8,      // Command size. For the ST7789 it's 8 Bits
        .lcd_param_bits = 8,    // Parameter size. The ST7789 takes multiple singular bytes in sequence => 8 Bit
        .spi_mode = 0,
        .trans_queue_depth = 10,    // Reserve 10 transactions to queue
        //.on_color_trans_done = example_notify_lvgl_flush_ready,
        //.user_ctx = &disp_drv,    // Userdata passed to the callback
    };

    // Device config
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = CONFIG_M32_DISPLAY_PIN_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };

    // Attach the LCD to the SPI bus
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);
    ESP_ERROR_CHECK(ret);

    // Use IDF-supplied ST7789 helper
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &st7789_panel_handle);

    ESP_ERROR_CHECK(esp_lcd_panel_reset(st7789_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(st7789_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(st7789_panel_handle, false, false));

    #ifdef CONFIG_M32_DISPLAY_INVERT_COLOR
    esp_lcd_panel_invert_color(st7789_panel_handle, true);
    #endif

    esp_lcd_panel_disp_on_off(st7789_panel_handle, true);
    
    #ifdef CONFIG_M32_DISPLAY_USE_BACKLIGHT
    // Set up the lighting
    _st7789_backlight_init();
    #endif


    uint16_t to_draw[CONFIG_M32_DISPLAY_WIDTH] = {0};
    
    for(unsigned int y=0; y<display.height; y++){
        esp_lcd_panel_io_tx_param(io_handle, ST7789_CASET, (uint8_t[]){0, 0, display.width>>8, display.width&0xff}, 4);
        esp_lcd_panel_io_tx_param(io_handle, ST7789_RASET, (uint8_t[]){y>>8, y&0xFF, (y+2)>>8, (y+2)&0xff}, 4);
        esp_lcd_panel_io_tx_color(io_handle, ST7789_RAMWR, &to_draw, display.width*sizeof(uint16_t));
        
    }

    #ifdef CONFIG_M32_DISPLAY_USE_BACKLIGHT
    // Turn on the light
    m32_display_backlight_set(50);
    #endif
    
    printf("ST7789 display initialized.\n");

    return display;
    
};


static m32_response_t _convert_palette(uint16_t* dest){

    m32_color_t* pal = m32_get_palette();

    for(int i=0; i<256; i++){
        // Ignore alpha - this is a display function
        uint8_t palcol_r = (pal[i] & 0x00FF0000) >> 16;
        uint8_t palcol_g = (pal[i] & 0x0000FF00) >> 8;
        uint8_t palcol_b = (pal[i] & 0x000000FF);

        // First truncate unneeded bits, then shift into place for RBG565
        // Works because i>>n == 1/(2**n)
        uint8_t dest_r = (palcol_r >> 3);
        uint8_t dest_g = (palcol_g >> 3);
        uint8_t dest_b = (palcol_b >> 2);

        dest[i] = (dest_r << 11) | (dest_b << 5) | dest_g;
    }

    return M32_OK;
}

m32_response_t m32_display_draw(m32_graphic_t* image)
{
    // Sanity check. Can we even fill the screen?
    if(image->width < display.width || image->height < display.height){ return M32_DISPLAY_ERROR_BAD_DIMENSIONS; }
    
    //printf("Dimensions check PASSED\n");
    
    static uint16_t pal_rgb565[256] = { 0 };
    //printf("RGB565 palette allocation PASSED\n");

    // Scanline slices instead of whole images
    static uint16_t to_draw[2][CONFIG_M32_DISPLAY_WIDTH*CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT] = {0};
    uint8_t current_line = 0;
    //printf("DISPLAY DRAW INFO:\n\tRGB565 PALETTE ptr = %p\n\tBACKBUFFER ptr = %p\n", pal_rgb565, to_draw);

    _convert_palette(pal_rgb565);

    //printf("Palette conversion PASSED\n");
    static uint16_t y = 0;
    
    #ifdef CONFIG_M32_DISPLAY_ST7789_CMD_DRAW
    esp_lcd_panel_io_tx_param(io_handle, ST7789_CASET, (uint8_t[]){0, 0, display.width>>8, display.width&0xff}, 4);
    #endif
        

    for(; y<display.height; y+=(2*CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT)){
        // Solution: Scanline slices
        for(int x=0; x < display.width*CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT; x++){
            to_draw[current_line][x] = pal_rgb565[image->buffer[image->width*y+x]];
        }
        #ifdef CONFIG_M32_DISPLAY_ST7789_BITMAP_DRAW
        esp_lcd_panel_draw_bitmap(st7789_panel_handle, 0, y, display.width, y+CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT, to_draw[current_line]);
        #endif

        #ifdef CONFIG_M32_DISPLAY_ST7789_CMD_DRAW
        //esp_lcd_panel_io_tx_param(io_handle, ST7789_CASET, (uint8_t[]){0, 0, display.width>>8, display.width&0xff}, 4);
        esp_lcd_panel_io_tx_param(io_handle, ST7789_RASET, (uint8_t[]){
            y>>8, y&0xFF,
            (y+CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT)>>8,
            (y+CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT)&0xff},
            4);
        esp_lcd_panel_io_tx_color(io_handle, ST7789_RAMWR, &to_draw[current_line], display.width*CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT*sizeof(uint16_t));
        #endif
        
        current_line = (current_line&1)?0:1;
    }

    y = (y%display.height >= CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT)?0:CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT;

    
    //printf("Backbuffer draw PASSED\n");

    return M32_OK;
};

#ifdef CONFIG_M32_DISPLAY_USE_BACKLIGHT
m32_response_t m32_display_backlight_set(uint8_t brightness){
    if(brightness >= 100){ brightness = 100; }
    return m32_simple_led_set(&backlight, brightness);
};
#else
m32_response_t m32_display_backlight_set(uint8_t brightness){
    printf("Backlight support is not enabled on this device.\nIf you are a developer, please check your `sdkconfig`.\n");
    return M32_OK;
};
#endif

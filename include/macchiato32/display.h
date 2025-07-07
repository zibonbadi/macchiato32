#ifndef M32_DISPLAY_H
#define M32_DISPLAY_H

#include <stdint.h>

#include "macchiato32/palette.h"
#include "macchiato32/graphic.h"


typedef struct m32_display{
    // Everything in this struct is read-only. It's even declared const in the driver to put it into DROM
    // Core stuff
    m32_pixel_format_t pixel_format;
    m32_coord_t width, height;	// Never <= 0 but matches data type
    
    // Display-dependent, but still generically addressable
    uint8_t backlight_max_brightness;
} m32_display_t;




#ifdef CONFIG_M32_DISPLAY_TYPE_VGA
#error "M32_DISPLAY_TYPE VGA is not supported."
//#include "display/vga.h"
#endif

#ifdef CONFIG_M32_DISPLAY_USE_SPI

/* SPI Display drivers */
#ifdef CONFIG_M32_DISPLAY_TYPE_ST7789
#include "display/st7789.h"
#endif
#ifdef CONFIG_M32_DISPLAY_TYPE_WS2812
#include "display/ws2812.h"
#endif

#endif
m32_display_t m32_display_init();	// Returns display info
m32_response_t m32_display_draw(m32_graphic_t* image); // Image must fill the screen.
m32_response_t m32_display_backlight_set(uint8_t brightness); // Backlight may get initialized during display_init()

#ifdef CONGIG_M32_USE_EMULATED_TILE_MAPPING
m32_display_draw_tilemap(m32_tilemap_t* src, m32_coord_t left_offset, m32_coord_t top_offset, bool loop_x, bool loop_y);
#endif


#endif

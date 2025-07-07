#ifndef M32_COMMON_H
#define M32_COMMON_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*
 * Generic base API definitions
 */
 typedef enum m32_response{
    M32_ERROR_GENERIC = -1,
    M32_OK = 0,
    M32_OUT_OF_RAM,
    M32_MEMORY_ACCESS_ERROR,
    M32_ERROR_INVALID_POINTER,
    M32_FILE_ACCESS_ERROR,
    M32_DISPLAY_ERROR,
    M32_DISPLAY_ERROR_BAD_DIMENSIONS,

} m32_response_t;

// Screen Coordinates
typedef int16_t m32_coord_t;
typedef struct m32_rect{
    m32_coord_t u;
    m32_coord_t v;
    m32_coord_t uw;
    m32_coord_t vh;
} m32_rect_t;


typedef enum m32_pixel_format{
    /* 8 Bit */
    M32_PIXEL_FORMAT_MONO8, // 8-Bit grayscale
    M32_PIXEL_FORMAT_RGB332, // 256 color display mode
    /* 16 Bit */
    M32_PIXEL_FORMAT_ARGB1555, // Symmetrical 16-Bit High Color w/ alpha bit
    M32_PIXEL_FORMAT_ARGB4444, // Symmetrical 16-Bit Highcolor w/ alpha
    M32_PIXEL_FORMAT_RGB565, // Asymmetrical 16-Bit High Color
    /* 32 Bit */
    M32_PIXEL_FORMAT_ARGB8888, // 8 bpc True Color w/ 8-bit alpha
} m32_pixel_format_t;


// Internal color types (useful for working with color conversions)
typedef uint8_t m32_color8_t;
typedef uint16_t m32_color16_t;
typedef uint32_t m32_color32_t;

// General color type.
// Default pixel format - M32_PIXEL_FORMAT_ARGB8888
typedef m32_color32_t m32_color_t;

// Macro function to improve binary size
#define m32_xy_to_linear(x,y,width) (y*width+x)

bool m32_rect_collide(m32_rect_t a, m32_rect_t b);
m32_rect_t m32_rect_intersect(m32_rect_t a, m32_rect_t b);


#endif
#ifndef M32_GRAPHIC_H
#define M32_GRAPHIC_H

#include <stdint.h>

#include "macchiato32/palette.h"

/*
 *	Graphic objects
 */
typedef struct m32_graphic{
	bool use_colorkey;
	uint16_t width;
	uint16_t height;
	int16_t left_offset;
	int16_t top_offset;
	#ifdef CONFIG_M32_EXT_ROTOZOOM
	uint8_t pivot_u;
	uint8_t pivot_v;
	#endif
	uint8_t colorkey;
	uint8_t* buffer;	// length == width*height
} m32_graphic_t;


m32_graphic_t m32_new_graphic(m32_coord_t width, m32_coord_t height);
m32_graphic_t m32_load_graphic_from_file(const char* filename);
m32_response_t m32_free_graphic(m32_graphic_t* m32_graphic);

m32_response_t m32_graphic_draw(m32_graphic_t* dest, m32_graphic_t* src, m32_coord_t x, m32_coord_t y, bool flip_x, bool flip_y);
m32_graphic_t m32_graphic_copy(m32_graphic_t* src, m32_rect_t* crop);	// Either copies or converts depending on the case. Overwrites dest

m32_response_t m32_graphic_draw_rect(m32_graphic_t* dest, m32_rect_t rect, uint8_t pcolor);
m32_response_t m32_graphic_fill_rect(m32_graphic_t* dest, m32_rect_t rect, uint8_t pcolor);
#ifdef CONFIG_M32_EXT_ROTOZOOM
m32_response_t m32_graphic_copy_graphic_rotozoom(m32_graphic_t* dest, m32_graphic_t* src, m32_coord_t x, m32_coord_t y, float rad, float scale);
#endif

#endif 


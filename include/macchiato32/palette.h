#ifndef M32_PALETTE_H
#define M32_PALETTE_H

#include "macchiato32/m32_common.h"

m32_response_t m32_palette_write_rgb332();
m32_color_t* m32_get_palette();
m32_response_t m32_load_palette_from_file(const char* filename);
uint8_t m32_palette_quantize_color(m32_color_t needle);	// Finds the *closest* color in-palette
m32_response_t m32_palette_write_linear_gradient(uint8_t index_start, uint8_t index_last, m32_color_t rcolor_start, m32_color_t rcolor_end);	// Linear gradient helper

#endif

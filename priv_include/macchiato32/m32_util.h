#ifndef M32_UTIL_H
#define M32_UTIL_H

#include "macchiato32/m32_common.h"

// Min/Max utilities since C doesn't deliver.
// Implementation found in m32.c
m32_coord_t m32_coord_min(m32_coord_t x, m32_coord_t y);
m32_coord_t m32_coord_max(m32_coord_t x, m32_coord_t y);

#endif
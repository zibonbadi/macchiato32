#ifndef M32_H
#define M32_H

#include "macchiato32/m32_common.h"

#include "macchiato32/display.h"
#include "macchiato32/graphic.h"

#ifdef CONFIG_M32_USE_EMULATED_TILE_MAPPING
#include "macchiato32/tiling.h"
#endif

m32_response_t m32_init();

#endif
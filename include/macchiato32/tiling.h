#ifndef M32_TILING_H
#define M32_TILING_H

#include "macchiato32/m32_common.h"
#include "macchiato32/graphic.h"

// Constant size tilesets to keep array indices predictable.
typedef struct{
    bool use_colorkey;
    uint8_t colorkey;
    // Image is an array of 64 16*16 pixel arrays (serialized tile-by-tile, then row-by-row)
    // MSB 7 and 6 are used for Y/X-flipping
    uint8_t* image;
} m32_tileset_t;


// Basically a 2D array referencing a tileset
typedef struct{
    uint8_t width, height; // Measured in *tiles*, not pixels 
    m32_tileset_t* tileset;
    uint8_t* map;  // Simple malloc() array. 256 tiles per-tileset should be enough.

} m32_tilemap_t;


m32_tileset_t m32_new_tileset();
m32_tileset_t m32_load_tileset_from_file(const char* filename);
m32_response_t m32_free_tileset(m32_tileset_t* set);

m32_response_t m32_tileset_draw_tile_to_graphic(m32_graphic_t* dest, m32_tileset_t* tileset, uint8_t src, m32_coord_t dest_x, m32_coord_t dest_y);


m32_tilemap_t m32_new_tilemap(m32_tileset_t* tileset, uint8_t width, uint8_t height);
m32_tilemap_t m32_load_tilemap_from_file(const char* filename, m32_tileset_t* tileset, uint8_t layer, m32_rect_t* tile_crop);
m32_response_t m32_free_tilemap(m32_tilemap_t* map);

m32_response_t m32_tilemap_draw_to_graphic(m32_graphic_t* dest, m32_tilemap_t* tilemap, m32_coord_t dest_x, m32_coord_t dest_y, bool loop_x, bool loop_y);

#endif
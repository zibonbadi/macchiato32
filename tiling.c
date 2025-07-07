#include <stdio.h>

#include "macchiato32/m32_common.h"
#include "macchiato32/m32_util.h"

#include "macchiato32/graphic.h"
#include "macchiato32/tiling.h"

m32_tileset_t m32_new_tileset(){

    m32_tileset_t out = {
        .use_colorkey = false,
        .colorkey = 0,
        .image = calloc(64, 16*16*sizeof(uint8_t)),
    };

    return out;
}

m32_tileset_t m32_load_tileset_from_file(const char* filename){
    m32_tileset_t ret = m32_new_tileset();

    bool flip_endian32 = false;
    bool flip_endian16 = false;

    uint32_t bom = 0;
    uint8_t use_colorkey = 0;
    uint8_t colorkey = 0;

    FILE* f = fopen(filename, "r");

    if(f == NULL){ return ret; };

    // Read header
    fread(&bom, sizeof(uint32_t), 1, f);

    flip_endian16 = ((bom&0xFFFF) == 0xFFFE) || ((bom&0xFFFF0000) == 0xFFFE0000);
    flip_endian32 = ((bom&0xFFFF) == 0x0000);

    printf("[TILESET LOAD] BOM 0x%08lX => FE16:%d FE32:%d\n", bom, flip_endian16, flip_endian32);

    fread(&use_colorkey, sizeof(uint8_t), 1, f);
    fread(&colorkey, sizeof(uint8_t), 1, f);

    ret.use_colorkey = (use_colorkey != 0);
    ret.colorkey = colorkey;

    printf("[TILESET LOAD] USE_CKEY %d | COLORKEY: %03d\n", use_colorkey, colorkey);

    // Skip padding bytes
    fseek(f, 2, SEEK_CUR);

    fread(ret.image, sizeof(uint8_t), 16*16*64, f);

    fclose(f);

    return ret;
}

m32_response_t m32_free_tileset(m32_tileset_t *tileset){

    free(tileset->image);
    //free(tileset);

    return M32_OK;
}

m32_response_t m32_tileset_draw_tile_to_graphic(m32_graphic_t* dest, m32_tileset_t* tileset, uint8_t src, m32_coord_t dest_x, m32_coord_t dest_y){
    
    // Get draw area rect
    m32_rect_t src_r =  { .u = dest_x, .v = dest_y, .uw = 16, .vh = 16 };
    m32_rect_t dest_r = { .u = 0, .v = 0, .uw = dest->width, .vh = dest->height };
    m32_rect_t draw_area = m32_rect_intersect(src_r, dest_r);

    // Nothing to do -> leave early
    if(draw_area.uw < 1 || draw_area.vh < 1){ return M32_OK; }

    // Starting offset of src
    uint32_t tile_start = (src&0x3F)*16*16;
    m32_coord_t src_v_off = ((src&0x80)>0) ? src_r.v-draw_area.v : draw_area.v-src_r.v;

    for(int y=0; y<draw_area.vh; y++){
        
        uint8_t dy = draw_area.v+y;
        uint8_t v = y;
        // Do we need to flip Y?
        if( (src&0x80) > 0 ){ v = 15-y; }
       
        m32_coord_t src_u_off = ((src&0x40)>0) ? src_r.u-draw_area.u : draw_area.u-src_r.u;

        for(int x=0; x<draw_area.uw; x++){
            
            uint8_t dx = draw_area.u+x;
            uint8_t u = x;
            
            // Do we need to flip X?
            if( (src&0x40) > 0 ){ u = 15-x; }

            if(!tileset->use_colorkey || 
                tileset->image[tile_start+(16*(v+src_v_off))+u+src_u_off] != tileset->colorkey
            ){
                dest->buffer[dest->width*dy+dx] = tileset->image[tile_start+(16*(v+src_v_off))+u+src_u_off];
            }
        }
    }

    return M32_OK;
}


m32_tilemap_t m32_new_tilemap(m32_tileset_t* tileset, uint8_t width, uint8_t height){
    
    // Tilesets are arrays of 64 paletted graphics of size 16*16
    // Why only 64? Because two MSBs are designated flipper bits
    // Bit 14 is used for X and  Bit 15 for Y-flipping.
    // This shrinks our memory footprint to 25%

    m32_tilemap_t out = {
        .width = width,
        .height = height,
        .tileset = tileset,
        .map = calloc(width*height, sizeof(uint8_t))
    };
    return out;
};


m32_tilemap_t m32_load_tilemap_from_file(const char* filename, m32_tileset_t* tileset, uint8_t layer, m32_rect_t* tile_crop){
    uint8_t width = 0;
    uint8_t height = 0;
    uint8_t pages = 0;

    FILE* f = fopen(filename, "r");

    if(f == NULL){ return m32_new_tilemap(NULL, 0, 0); };

    // Read header
    fread(&width, sizeof(uint8_t), 1, f);
    fread(&height, sizeof(uint8_t), 1, f);
    fread(&pages, sizeof(uint8_t), 1, f);

    printf("[TILEMAP LOAD] TILEMAP TOTAL SIZE %03d * %03d | Pages: %03d\n", width, height, pages+1);

    m32_rect_t map_r    = {.u=0, .v=0, .uw=width, .vh=height};
    m32_rect_t param_r  = map_r;
    
    // If crop == NULL -> Take entire map
    if(tile_crop != NULL){
        param_r.u   =    tile_crop->u;
        param_r.v   =    tile_crop->v;
        param_r.uw  =    tile_crop->uw;
        param_r.vh  =    tile_crop->vh;
    }
    
    m32_rect_t sample_r  = m32_rect_intersect(map_r, param_r);
    m32_tilemap_t ret    = m32_new_tilemap(tileset, sample_r.uw, sample_r.vh);

    printf("[TILEMAP LOAD] TILEMAP CROP AREA (%03d,%03d) @ %03d * %03d\n", sample_r.u, sample_r.v, sample_r.uw, sample_r.vh);
    
    // Skip padding byte + jump to start of layer
    fseek(f, 1+(width*height*layer), SEEK_CUR);
    fpos_t page_start;
    fgetpos(f, &page_start);

    for(int line=0; line < sample_r.vh; line++){
        // Seek row
        fsetpos(f, &page_start);
        fseek(f, ((sample_r.v+line)*map_r.uw)+sample_r.u, SEEK_CUR);
        fread(ret.map+(line*sample_r.uw), sizeof(uint8_t), sample_r.uw, f);
    }

    fclose(f);

    return ret;
}


m32_response_t m32_free_tilemap(m32_tilemap_t* tilemap){
    
    tilemap->width = 0;
    tilemap->height = 0;
    
    free(tilemap->map);
    //free(tilemap);

    return M32_OK;
};

m32_response_t m32_tilemap_draw_to_graphic(m32_graphic_t* dest, m32_tilemap_t* tilemap, m32_coord_t dest_x, m32_coord_t dest_y, bool loop_x, bool loop_y){
    
    // Loop support - we recurse
    if(loop_y){
        m32_coord_t y_start = dest_y;
        m32_coord_t y_end = dest_y;
        
        // Extend tilemap rect across the entirety of dest
        while(y_start > 0){         y_start -= tilemap->height*16; }
        while(y_end < dest->height){ y_end   += tilemap->height*16; }
        
        for(int y = y_start; y < y_end; y+=tilemap->height*16){
            m32_tilemap_draw_to_graphic(dest, tilemap, dest_x, y, loop_x, false);
        }

        return M32_OK;
    }
    if(loop_x){
        m32_coord_t x_start = dest_x;
        m32_coord_t x_end = dest_x;
        
        // Extend tilemap rect across the entirety of dest
        while(x_start > 0){         x_start -= tilemap->width*16; }
        while(x_end < dest->width){ x_end   += tilemap->width*16; }
        
        for(int x = x_start; x < x_end; x+=tilemap->width*16){
            m32_tilemap_draw_to_graphic(dest, tilemap, x, dest_y, false, false);
        }

        return M32_OK;
    }



    // There's two ways we can render a tilemap:
    //
    // 1. Line-by-line according to dest space
    // 2. Tile-by-tile using m32_tileset_draw_tile_to_graphic()
    //
    // This function renders tilemaps tile-by-tile

    // Steps (non-looping):
    // 1. Calculate coarse rect for tile collisions w/ dest
    // 2. Align rect to cover quantized edge cases
    // 3. Render tile-by-tile


    // Get draw area rect (projected to dest space)
    m32_rect_t src_r =  { .u = dest_x, .v = dest_y, .uw = tilemap->width*16, .vh = tilemap->height*16 };
    m32_rect_t dest_r = { .u = 0, .v = 0, .uw = dest->width, .vh = dest->height };
    m32_rect_t draw_area = m32_rect_intersect(src_r, dest_r);
    // Also project graphic onto tilemap to get tile area
    src_r.u = 0;
    src_r.v = 0;
    dest_r.u = -dest_x;
    dest_r.v = -dest_y;
    m32_rect_t tile_area_px = m32_rect_intersect(src_r, dest_r);

    // Nothing to do/something went wrong -> leave early
    if(draw_area.uw < 1 || draw_area.vh < 1){ return M32_OK; }
    
    int8_t sign_u = (tile_area_px.u<0)?-1:1;
    int8_t sign_v = (tile_area_px.v<0)?-1:1;

    // Index-wise tilemap intersection
    uint8_t src_u_tile  =  m32_coord_min(tile_area_px.u/16, (tile_area_px.u+15*sign_u)/16);
    uint8_t src_v_tile  =  m32_coord_min(tile_area_px.v/16, (tile_area_px.v+15*sign_v)/16);
    uint8_t uw_tile     =  (tile_area_px.uw+15)   /   16;  // +1 offset to be cover screen edges
    uint8_t vh_tile     =  (tile_area_px.vh+15)   /   16;
    
    // Render Tile-by-tile
    for(m32_coord_t ty=0; ty<vh_tile; ty++){

        uint8_t tv = src_v_tile+ty;

        for(m32_coord_t tx=0; tx<uw_tile; tx++){
            // Render the actual tile. The function's internal clip will clean up for us.
            
            uint8_t tu = src_u_tile+tx;

            m32_tileset_draw_tile_to_graphic(
                dest,
                tilemap->tileset,
                tilemap->map[((tv%tilemap->height) * tilemap->width) + (tu%tilemap->width)],  // src (with loop)
                dest_x+(16*tu),
                dest_y+(16*tv)
            );
        }
    } 

    return M32_OK;
};

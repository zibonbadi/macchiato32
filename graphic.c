#include "stdio.h"

#include "macchiato32/m32_common.h"
#include "macchiato32/m32_util.h"

#include "macchiato32/graphic.h"
#include "macchiato32/palette.h"

m32_response_t m32_graphic_draw(m32_graphic_t* dest,  m32_graphic_t* src, m32_coord_t x, m32_coord_t y, bool flip_x, bool flip_y)
{

    // Flip along offset point
    m32_coord_t off_x = (flip_x) ? src->width  - src->left_offset : src->left_offset;
    m32_coord_t off_y = (flip_y) ? src->height - src->top_offset  : src->top_offset;

    // Get draw area rect
    m32_rect_t src_r =  { .u = x-off_x, .v = y-off_y, .uw = src->width, .vh = src->height };
    m32_rect_t dest_r = { .u = 0, .v = 0, .uw = dest->width, .vh = dest->height };
    m32_rect_t draw_area = m32_rect_intersect(src_r, dest_r);


    m32_coord_t src_v_off = (flip_y) ? src_r.v-draw_area.v : draw_area.v-src_r.v;

    // Nothing to do -> leave early
    if(draw_area.uw < 1 || draw_area.vh < 1){ return M32_OK; }

    for(int y=0; y<draw_area.vh; y++){
        
        uint8_t dy = draw_area.v+y;
        uint8_t v = y;
        // Do we need to flip Y?
        if( flip_y ){ v = src->width-1-y; }
            
        m32_coord_t src_u_off = (flip_x) ? src_r.u-draw_area.u : draw_area.u-src_r.u;

        for(m32_coord_t x=0; x<draw_area.uw; x++){
            
            m32_coord_t dx = draw_area.u+x;
            m32_coord_t u = x;
            
            // Do we need to flip X?
            if( flip_x ){ u = src->width-1-x; }

            if(!src->use_colorkey || 
                src->buffer[(src->width*(v+src_v_off))+u+src_u_off] != src->colorkey
            ){
                dest->buffer[dest->width*dy+dx] = src->buffer[(src->width*(v+src_v_off))+u+src_u_off];
            }
        }
    }

    return M32_OK;
};

m32_graphic_t m32_new_graphic(m32_coord_t width, m32_coord_t height){
    m32_graphic_t ret;
    ret.width = width;
    ret.height = height;
    ret.use_colorkey = false;
    ret.colorkey = 0;
    ret.left_offset = 0;
    ret.top_offset = 0;
    ret.buffer = calloc(height*width, sizeof(uint8_t));
    return ret;
}

m32_response_t m32_free_graphic(m32_graphic_t* dest){
    dest->width = 0;
    dest->height = 0;
    dest->use_colorkey = false;
    dest->colorkey = 0;
    dest->left_offset = 0;
    dest->top_offset = 0;
    free(dest->buffer);

    return M32_OK;
}


m32_graphic_t m32_load_graphic_from_file(const char* filename){
    bool flip_endian32 = false;
    bool flip_endian16 = false;

    uint32_t bom = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    int16_t left_offset = 0;
    int16_t top_offset = 0;
    uint8_t use_colorkey = 0;
    uint8_t colorkey = 0;

    FILE* f = fopen(filename, "r");

    if(f == NULL){ return m32_new_graphic(0,0); };

    // Read header
    fread(&bom, sizeof(uint32_t), 1, f);

    flip_endian16 = ((bom&0xFFFF) == 0xFFFE) || ((bom&0xFFFF0000) == 0xFFFE0000);
    flip_endian32 = ((bom&0xFFFF) == 0x0000);

    printf("[GFX LOAD] BOM 0x%08lX => FE16:%d FE32:%d\n", bom, flip_endian16, flip_endian32);

    fread(&width, sizeof(uint16_t), 1, f);
    fread(&height, sizeof(uint16_t), 1, f);
    fread(&left_offset, sizeof(uint16_t), 1, f);
    fread(&top_offset, sizeof(uint16_t), 1, f);

    if(flip_endian16){
        width = (width>>8)|(width<<8);
        height = (height>>8)|(height<<8);
        left_offset = (left_offset>>8)|(left_offset<<8);
        top_offset = (top_offset>>8)|(top_offset<<8);
    }
    
    // Read Colorkey info
    fread(&use_colorkey, sizeof(uint8_t), 1, f);
    fread(&colorkey, sizeof(uint8_t), 1, f);

    // Finally create graphic
    m32_graphic_t ret = m32_new_graphic(width, height);

    ret.left_offset = left_offset;
    ret.top_offset = top_offset;

    ret.use_colorkey = (use_colorkey != 0);
    ret.colorkey = colorkey;

    printf("[GFX LOAD] New Graphic!\n\tWIDTH %d\n\tHEIGHT %d\n\tLEFT OFFSET %d\n\tTOP OFFSET %d\n\t\n\tUSE_CKEY %d\n\tCOLORKEY: %03d\n", width, height, left_offset, top_offset, use_colorkey, colorkey);

    fread(ret.buffer, sizeof(uint8_t), width*height, f);

    fclose(f);

    return ret;
}

m32_graphic_t m32_graphic_copy(m32_graphic_t* src, m32_rect_t* crop){
    // Unlike m32_graphic_draw(), this function recreates dest
    // entirely based on the previously-specified pixel format and type.

    // If rect is not given, copy the entire source graphic
    m32_rect_t dest_r = {.u=0, .v=0, .uw=src->width, .vh=src->height};
    m32_rect_t param_r = dest_r;

    if(crop != NULL){
        param_r.u   =    crop->u;
        param_r.v   =    crop->v;
        param_r.uw  =    crop->uw;
        param_r.vh  =    crop->vh;
    }

    m32_rect_t sample_r = m32_rect_intersect(dest_r, param_r);

    m32_graphic_t ret = m32_new_graphic(sample_r.uw, sample_r.vh);

    // Overwrite metadata
    ret.use_colorkey = src->use_colorkey;
    ret.left_offset = src->left_offset - sample_r.u;
    ret.top_offset = src->top_offset - sample_r.v;

    if(sample_r.uw == src->width){
        // Full-width copy wanted -> Use all the DMA
        memcpy(ret.buffer, src->buffer+(sample_r.v*src->width), src->width*sample_r.vh);
    }else{
        for(int line=0; line<sample_r.vh; line++){
            // Slice-based memcpy *should* work, but doesn't
            memcpy(ret.buffer+(line*ret.width), src->buffer+( (sample_r.v+line)*dest_r.uw+sample_r.u ), sample_r.uw);
        }
    }

    return ret;
}


m32_response_t m32_graphic_fill_rect(m32_graphic_t* dest, m32_rect_t rect, uint8_t pcolor){

    m32_rect_t dest_r = {.u=0, .v=0, .uw=dest->width, .vh=dest->height};
    m32_rect_t ret_r = m32_rect_intersect(dest_r, rect);

    for(int y=ret_r.v; y<ret_r.v+ret_r.vh; y++){
        // Slice-based memcpy
        memset(dest->buffer+(y*dest->width+ret_r.u), pcolor, ret_r.uw);
    }

    return M32_OK;
}


m32_response_t m32_graphic_draw_rect(m32_graphic_t* dest, m32_rect_t rect, uint8_t pcolor){

    printf("DRAWING RECT\n");
    m32_rect_t dest_r = {.u=0, .v=0, .uw=dest->width, .vh=dest->height};
    m32_rect_t ret_r = m32_rect_intersect(dest_r, rect);

    // First line and Last line before vertical lines. We wanna use the DMA if we can
    memset(dest->buffer+ m32_xy_to_linear(ret_r.u, ret_r.v, dest->width) , pcolor, ret_r.uw);
    memset(dest->buffer+ m32_xy_to_linear(ret_r.u, (ret_r.v+ret_r.vh-1), dest->width) , pcolor, ret_r.uw);
    
    // Vertical lines - Draw first and last pixel
    for(int y=ret_r.v; y<ret_r.v+ret_r.vh ;y++){
        dest->buffer[y*dest->width+ret_r.u] = pcolor;
        dest->buffer[y*dest->width+ret_r.u+ret_r.uw-1] = pcolor;
    }

    return M32_OK;
}
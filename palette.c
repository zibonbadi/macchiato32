#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>

#include "macchiato32/m32_common.h"

#include "macchiato32/palette.h"

static m32_color_t palette[256] = { 0xFF000000 };

m32_response_t m32_palette_write_rgb332(){
    // Fill palette with RGB332 conversions by default
    for(uint16_t i=0;i<256;i++){
        // MSB transfer with repeat
        uint8_t r = (i&0xE0)      | (i&0xE0) >> 3 | (i&0xE0) >> 6;
        uint8_t g = (i&0x1C) << 3 | (i&0x1C)      | (i&0x1C) >> 3;
        uint8_t b = (i&0x03) << 6 | (i&0x03) << 4 | (i&0x03) << 2 | (i&0x03);

        palette[i] = 0xFF000000 | (r << 16) | (g << 8) | (b);
    }

    printf("Filled palette with RGB332 values.\n");

    return M32_OK;
}

m32_color_t* m32_get_palette(){
    return palette;
};


m32_response_t m32_load_palette_from_file(const char* filename){

    char format[5] = "....";

    FILE* f = fopen(filename, "r");

    if(f == NULL){ return M32_FILE_ACCESS_ERROR; };

    // Read header
    fread(&format, sizeof(uint32_t), 1, f);

    uint8_t pos_a = strchr(format, 'A')-format;
    uint8_t pos_r = strchr(format, 'R')-format;
    uint8_t pos_g = strchr(format, 'G')-format;
    uint8_t pos_b = strchr(format, 'B')-format;

    if(strcmp(format, "BGRA") == 0)
        fread(palette, sizeof(uint32_t), 256, f);
    else{
        uint32_t head = 0;
        for(int i=0; i<256; i++){
            fread(&head, sizeof(uint32_t), 1, f);
            palette[i] =  ( head & (0xFF>>(pos_a*8)) ) << 24 |
                          ( head & (0xFF>>(pos_r*8)) ) << 16 |
                          ( head & (0xFF>>(pos_g*8)) ) << 8  |
                          ( head & (0xFF>>(pos_b*8)) );
        }
    }

    fclose(f);

    return M32_OK;
}

uint8_t m32_palette_quantize_color(m32_color_t needle){
    // Finds the *closest* color in-palette

    uint32_t dist_out = INT_MAX,  dist_i = INT_MAX;
    uint8_t out = 0;   // This is why I use a pointer

    // Assume ARGB8888 for now.
    m32_color_t needle_red = (needle & 0xFF0000) >> 16;
    m32_color_t needle_green = (needle & 0x00FF00) >> 8;
    m32_color_t needle_blue = needle & 0x0000FF;

    // Yes it's wasteful to keep looking. And yes, it's also more reliable.
    for(int i = 0; i < 256; i++){

        // Assume ARGB8888 for now.
        m32_color_t i_color = palette[i];

        m32_color_t i_red = (i_color & 0xFF0000) >> 16;
        m32_color_t i_green = (i_color & 0x00FF00) >> 8;
        m32_color_t i_blue = i_color & 0x0000FF;
    
        
        // Calculate distean distance
        dist_i = sqrt(
                        (i_red-needle_red)*(i_red-needle_red) +
                        (i_green-needle_green)*(i_green-needle_green) +
                        (i_blue-needle_blue)*(i_blue-needle_blue) 
                    );
        bool i_closer = (dist_i) < (dist_out);

        if( i_closer){
            dist_out = dist_i;
            out = i;
        }
    }
    
    return out;
};

m32_response_t m32_palette_write_linear_gradient(uint8_t index_start, uint8_t index_end, m32_color_t pcolor_start, m32_color_t pcolor_end){
    uint8_t start_r = (pcolor_start&0xFF0000) >> 16;
    uint8_t start_g = (pcolor_start&0x00FF00) >> 8;
    uint8_t start_b = (pcolor_start&0x0000FF);

    uint8_t end_r = (pcolor_end&0xFF0000) >> 16;
    uint8_t end_g = (pcolor_end&0x00FF00) >> 8;
    uint8_t end_b = (pcolor_end&0x0000FF);

    // Simple linear interpolation for three channels
    for(int i=0; i <= index_end-index_start; i++){
        float scalar = (float) (i)/(index_end-index_start);
        uint8_t rcolor_r = scalar*end_r + (1-scalar)*start_r;
        uint8_t rcolor_g = scalar*end_g + (1-scalar)*start_g;
        uint8_t rcolor_b = scalar*end_b + (1-scalar)*start_b;
        palette[index_start+i] = 0xFF000000 |       // Alpha
                                (rcolor_r << 16) |  // Red
                                (rcolor_g << 8) |   // Green
                                (rcolor_b);         // Blue
    }
    return M32_OK;
};
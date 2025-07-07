
#include "macchiato32/m32_common.h"
#include "macchiato32/io/spi.h"
#include "macchiato32/palette.h"

m32_response_t m32_init(){
    m32_response_t ret;

    #ifdef CONFIG_M32_USE_SPI
    ret = m32_spi_init();
    assert(ret == M32_OK);
    #endif

    ret = m32_palette_write_rgb332();
    assert(ret == M32_OK);

    return M32_OK;
};

// Borrowed from m32_util.h
m32_coord_t m32_coord_min(m32_coord_t x, m32_coord_t y){ return x > y ? y : x; }
m32_coord_t m32_coord_max(m32_coord_t x, m32_coord_t y){ return x < y ? y : x; }


bool m32_rect_collide(m32_rect_t a, m32_rect_t b){
    bool intersect_x = (a.u >= b.u && a.u < (b.u+b.uw)) || (b.u >= a.u && b.u < (a.u+a.uw));
    bool intersect_y = (a.v >= b.v && a.v < (b.v+b.vh)) || (b.v >= a.v && b.v < (a.v+a.vh));

    return (intersect_x && intersect_y);
}

m32_rect_t m32_rect_intersect(m32_rect_t a, m32_rect_t b){
    

    m32_rect_t out = {0,0,0,0};

    if(m32_rect_collide(a,b)){
        out.u  = m32_coord_max(a.u, b.u);
        out.v  = m32_coord_max(a.v, b.v);
        // Calculate uw and vh as distance from top left of out
        out.uw = m32_coord_min((a.u+a.uw)-out.u, (b.u+b.uw)-out.u);
        out.vh = m32_coord_min((a.v+a.vh)-out.v, (b.v+b.vh)-out.v);;
    }

    return out;
}
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#define PI 3.14159265

// ESP system stuff
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

// GPIO etc. for demo code
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"

#include "macchiato32/m32.h"

typedef enum scene {
    SCENE_LANDSCAPE,
    SCENE_FIRE,
    SCENE_TILES,
    SCENE_IMAGE,
    SCENE_MAX,
} scene_t;

static void load_scene_landscape(m32_graphic_t* image, m32_tilemap_t* tmap, m32_tileset_t* tset){

    m32_load_palette_from_file("/rom/palette.gdp");

    // Sky
    for (int y = 0; y < 132; y++) {
        for (int x = 0; x < image->width; x++) {
            image->buffer[y*image->width+x] = (x&8)^(y&8)? 159:153;
        }
    }

    // Sun/Moon - yes it's overdraw but who cares if it's precalc
    for (int y = 0; y < 132; y++) {
        for (int x = 0; x < image->width; x++) {
            image->buffer[y*image->width+x] = ((x-120)*(x-120)+(y-132)*(y-132))<(60*60) ? 0 : image->buffer[y*image->width+x] ;
        }
    }

    // Emptiness
    for (int i = image->width*179; i < image->height*image->width; i++) {
        image->buffer[i] = 0;
    }

    // Clean up old tilemap
    m32_free_tilemap(tmap);
    
    // Load tilemap from flash
    *tmap = m32_load_tilemap_from_file("/rom/landscape.gdm", tset, 0, NULL);

    assert(tmap->map != NULL);
    
}

static void load_scene_fire(m32_graphic_t* dest){
    // Write fire palette
    m32_palette_write_linear_gradient(0,255, 0xFF000000, 0xFFFF3f00);

    // Reset image
    for (int i = 0; i < dest->height * dest->width; i++) {
        dest->buffer[i] = 0;
    }
}

static void load_scene_tiles(m32_graphic_t* dest, m32_tilemap_t* tmap, m32_tileset_t* tset){
    // Clean up old tilemap
    m32_free_tilemap(tmap);

    // Set appropriate palette colors

    m32_load_palette_from_file("/rom/palette.gdp");

    // Load from flash
    *tmap = m32_load_tilemap_from_file("/rom/checkers.gdm", tset, 0, NULL);
    
    assert(tmap->map != NULL);

}

static void load_scene_image(m32_graphic_t* displaybuf, m32_graphic_t* ball){
    // Reset palette
    m32_load_palette_from_file("/rom/palette.gdp");

    m32_rect_t star_rect = {.u=6, .v=7, .uw=20, .vh=21};
    //m32_rect_t star_rect = {.u=-6, .v=-6, .uw=40, .vh=40};
    m32_graphic_t star = m32_graphic_copy(ball, &star_rect);
    //m32_graphic_t star = m32_graphic_copy(ball, NULL);
    star.colorkey = 54;

    // Reset image
    for (int i = 0; i < displaybuf->height * displaybuf->width; i++) {
        displaybuf->buffer[i] = 31;
    }
    m32_graphic_draw(displaybuf, ball, displaybuf->width/2, displaybuf->height/2, false, false);

    m32_graphic_fill_rect(displaybuf, (m32_rect_t){.u =   (displaybuf->width/4)-star.width/2, .v =   (displaybuf->height/4)-star.height/2, .uw=star.width, .vh=star.height}, 152);
    m32_graphic_fill_rect(displaybuf, (m32_rect_t){.u =   (displaybuf->width/4)-star.width/2, .v = 3*(displaybuf->height/4)-star.height/2, .uw=star.width, .vh=star.height}, 152);
    m32_graphic_fill_rect(displaybuf, (m32_rect_t){.u = 3*(displaybuf->width/4)-star.width/2, .v = 3*(displaybuf->height/4)-star.height/2, .uw=star.width, .vh=star.height}, 152);
    m32_graphic_fill_rect(displaybuf, (m32_rect_t){.u = 3*(displaybuf->width/4)-star.width/2, .v =   (displaybuf->height/4)-star.height/2, .uw=star.width, .vh=star.height}, 152);

    m32_graphic_draw(displaybuf, &star, (displaybuf->width/4)*1, (displaybuf->height/4)*1, false, false);
    m32_graphic_draw(displaybuf, &star, (displaybuf->width/4)*1, (displaybuf->height/4)*3, false, true);
    m32_graphic_draw(displaybuf, &star, (displaybuf->width/4)*3, (displaybuf->height/4)*1, true, false);
    m32_graphic_draw(displaybuf, &star, (displaybuf->width/4)*3, (displaybuf->height/4)*3, true, true);

    m32_graphic_draw(displaybuf, &star, displaybuf->width/2, displaybuf->height/2, false, true);

    m32_graphic_draw_rect(displaybuf, (m32_rect_t){.u =   (displaybuf->width/4)-star.left_offset, .v =   (displaybuf->height/4)-star.top_offset, .uw=star.width, .vh=star.height}, 35);
    m32_graphic_draw_rect(displaybuf, (m32_rect_t){.u =   (displaybuf->width/4)-star.left_offset, .v = 3*(displaybuf->height/4)-star.top_offset, .uw=star.width, .vh=star.height}, 35);
    m32_graphic_draw_rect(displaybuf, (m32_rect_t){.u = 3*(displaybuf->width/4)-star.left_offset, .v = 3*(displaybuf->height/4)-star.top_offset, .uw=star.width, .vh=star.height}, 35);
    m32_graphic_draw_rect(displaybuf, (m32_rect_t){.u = 3*(displaybuf->width/4)-star.left_offset, .v =   (displaybuf->height/4)-star.top_offset, .uw=star.width, .vh=star.height}, 35);
}

void app_main(void){

    /*  ======== ESP32 Macchiato32 demo ========
     *              by Zibon Badi 
     *
     * This Demo consists of four "scenes", which can be
     * switched between using a button attached to GPIO pin 4.
     *
     * 1. A landscape mixing techniques to render an average game scene.
     *    Pressing Buttons attached to GPIO pins 19 and 5 moves
          a sprite right/left using the technique "dirty rectangles".
     * 2. A fire effect which overwrites every pixel once per frame.
     * 3. A simple full-screen tilemap scroller.
     * 4. A static image - rendered once and only displayed since.
     *    Used to benchmark the maximum possible framerate.
     *
     *      All assets were compiled using M32Tool:
     *      <https://github.com/zibonbadi/m32tool>
     *
     * Default KConfig/menuconfig/sdkconfig settings:
     *
     *      // General Setup
     *      CONFIG_M32_USE_SPI=y
     *      CONFIG_M32_USE_EMULATED_TILE_MAPPING=y
     *      // SPI Settings
     *      CONFIG_M32_SPI_PIN_SCLK=0
     *      CONFIG_M32_SPI_PIN_MOSI=20
     *      CONFIG_M32_SPI_PIN_MISO=21
     *      // Display setting
     *      CONFIG_M32_DISPLAY_WIDTH=240
     *      CONFIG_M32_DISPLAY_HEIGHT=240
     *      CONFIG_M32_DISPLAY_TYPE_ST7789=y
     *      // ST7789 display settings
     *      CONFIG_M32_DISPLAY_PIN_SPI_CS=1
     *      CONFIG_M32_DISPLAY_PIN_DC=2
     *      CONFIG_M32_DISPLAY_PIN_RST=3
     *      CONFIG_M32_DISPLAY_PIN_BACKLIGHT=10
     *      CONFIG_M32_DISPLAY_PIXEL_CLOCK_HZ=16000000
     *      CONFIG_M32_DISPLAY_USE_BACKLIGHT=y
     *      CONFIG_M32_DISPLAY_INVERT_COLOR=y
     *      CONFIG_M32_DISPLAY_ST7789_BITMAP_DRAW=y
     *      CONFIG_M32_DISPLAY_INTERLACE_LINE_HEIGHT=2
     */

    esp_vfs_spiffs_conf_t fs_conf = {
        .base_path = "/rom",
        .max_files = 3,
        .partition_label = 0,
        .format_if_mount_failed = false
    };
    esp_vfs_spiffs_register(&fs_conf);

    m32_init();

    m32_display_t display = m32_display_init(); // Get screen
        
    printf("DISPLAY INFO:\n\tPIXEL_FORMAT: %d\n\tWIDTH: %d\n\tHEIGHT: %d\n\tMAX_BRIGHTNESS: %d\n", display.pixel_format, display.width, display.height, display.backlight_max_brightness);

    // By default, Macchiato32 maps it's color palette to RGB332.
    // You can easily write your own color palette, though.
    m32_color_t* pal = m32_get_palette();
    // Colors based on the Indigo Peach palette: <https://lospec.com/palette-list/indigo-peach>

    // Get screen-sized buffer.
    m32_graphic_t displaybuf = m32_new_graphic(display.width, display.height);
    m32_graphic_t ball = m32_load_graphic_from_file("/rom/6ball.gdg");
    m32_graphic_t ball_drect = m32_new_graphic(0,0);
    
    printf("Graphics allocated.\n");

    // Load Tileset from flash 
    m32_tileset_t tset = m32_load_tileset_from_file("/rom/tileset.gdt");

    m32_tilemap_t tmap = m32_new_tilemap(&tset, 1,1);
    m32_tilemap_t clouds = m32_load_tilemap_from_file("/rom/checkers.gdm", &tset, 1, NULL);

    printf("Tiles defined.\n");
    
    // Let there be light
    m32_display_backlight_set(95);

    // "Game loop"

    printf("Precalculating sine table...\n");

    // Pre-calc sine table for water effect
    int8_t sinetable[179-132];
    for(int i=0; i<179-132;i++){
        // Amplitude from -20 to 20
        // 1 Phase for the entire y span of the image
        sinetable[i] = sin(i*(179-132)*(PI/360))*20;
    }

    printf("Entering display loop...\n");

    uint8_t tile_off_x = 0;
    uint8_t tile_off_y = 0;
    m32_coord_t ball_x = displaybuf.width/2;
    uint32_t frame = 0;
    uint32_t tick_old = 0;
    uint32_t tick = 0;

    // Cheap FPS counter
    struct timeval tv_veryold;
    struct timeval tv_old;
    struct timeval tv_new;
    gettimeofday(&tv_new, NULL);


    // GPIO initialization
    gpio_set_direction(4, GPIO_MODE_INPUT); // Scene switch
    gpio_set_direction(18, GPIO_MODE_INPUT); // Move right
    gpio_set_direction(5, GPIO_MODE_INPUT); // Move left
    bool gpio_edge = false;

    scene_t scene = SCENE_LANDSCAPE;
    load_scene_landscape(&displaybuf, &tmap, &tset);

    m32_rect_t dirtyrect = {.u=ball_x-ball.left_offset, .v=132-ball.top_offset, .uw=ball.width, .vh=ball.height};
    ball_drect = m32_graphic_copy(&displaybuf, &dirtyrect);
    ball_drect.left_offset = 0;
    ball_drect.top_offset = 0;
    
    while(1){
        
        switch(scene){
            case SCENE_LANDSCAPE:{
   
                // Update dirty rect
                m32_graphic_draw(&displaybuf, &ball_drect, dirtyrect.u, dirtyrect.v, false,false);
                m32_free_graphic(&ball_drect);

                dirtyrect.u=ball_x-ball.left_offset;
                dirtyrect.v=132-ball.top_offset;
                dirtyrect.uw=ball.width;
                dirtyrect.vh=ball.height;
                
                ball_drect = m32_graphic_copy(&displaybuf, &dirtyrect);
                ball_drect.left_offset = 0;
                ball_drect.top_offset = 0;
                if(dirtyrect.u < 0){dirtyrect.u = 0;}   // Lock dirty rect to screen border
                
                // Draw actual sprite before water to get a responsive mirror effect
                m32_graphic_draw(&displaybuf, &ball, ball_x, 132, ((tick&0x4) > 0), false);

                // Sun/Moon mirror - slightly squashed for perspective. Reverse-sampled so we can regenerate it per-frame
                for (int y = 132; y < 179; y++) {
                    int8_t offset_x = sinetable[(y+tick)%(179-132)];
                    for (int x = 0; x < displaybuf.width; x++) {
                        displaybuf.buffer[y*displaybuf.width+x] = displaybuf.buffer[(131-((y-132)*2))*displaybuf.width+((x+offset_x)%displaybuf.width)];
                        if(displaybuf.buffer[y*displaybuf.width+x] != 0){ displaybuf.buffer[y*displaybuf.width+x] = displaybuf.buffer[y*displaybuf.width+x] - 4; }
                    }
                }

                // Scroll tilemap
                tile_off_x = (tile_off_x-3*(tick-tick_old))&0x7F;
                // Draw tilemap
                m32_tilemap_draw_to_graphic(&displaybuf, &tmap, tile_off_x, 179, true, false);

                if(gpio_get_level(18) == 1 && (ball_x<display.width) ){ ball_x += (tick-tick_old); }    // Move right
                if(gpio_get_level(5) == 1 && (ball_x>0) ){ ball_x -= (tick-tick_old); }    // Move left

                pal[0] = pal[((tick>>3)%4)+80]; // Animate something pretty

                break;
            }
            case SCENE_FIRE:{
                // Good ol' demofire
                for(int y=0; y<displaybuf.height; y++){
                    for(int x=0; x<displaybuf.width; x++){
                        uint8_t above = (y==0)?0:displaybuf.buffer[(y-1)*displaybuf.width + x];
                        uint8_t below = (y==displaybuf.height-1)? (rand()%256) : displaybuf.buffer[(y+1)*displaybuf.width + x];
                        //uint8_t below = (y==displaybuf.height-1)? ( x>((displaybuf.width/2)-sinetable[frame%(179-132)]+20) && x<((displaybuf.width/2)+sinetable[frame%(179-132)]+20) )? 255:0 : displaybuf.buffer[(y+1)*displaybuf.width + x];
                        
                        //((x>(displaybuf.width/2)+sinetable[(frame%(179-132)]+20) && x>(displaybuf.width/2)+sinetable[(frame%(179-132)]+20))? 255:0;

                        displaybuf.buffer[y*display.width + x] = ((above+below)/2)-1;
                    }
                }
                break;
            }
            case SCENE_TILES:{
                // Scroll our checker tilemap
                tile_off_x = (tile_off_x-(1*(tick-tick_old)))%128;
                tile_off_y = (tile_off_y-(3*(tick-tick_old)))%128;
                m32_tilemap_draw_to_graphic(&displaybuf, &tmap, tile_off_x, 0, true, true);
                m32_graphic_draw(&displaybuf, &ball, displaybuf.width/2+(sinetable[(tick>>2)%(179-132)]), displaybuf.height/2+(sinetable[(tick>>1)%(179-132)]), false, false);
                m32_tilemap_draw_to_graphic(&displaybuf, &clouds, 0, tile_off_y, true, true);
                break;
            }
            case SCENE_IMAGE:{
                // Do literally nothing but draw the image as fast as we can
                break;
            }
            default:{ break; }
        }

        m32_display_draw(&displaybuf);
        
        // Hybrid tickrate implementation
        frame++;
        tv_old = tv_new;
        gettimeofday(&tv_new, NULL);
        tick_old = tick;
        tick += ((tv_new.tv_sec*1000000+tv_new.tv_usec)-(tv_old.tv_sec*1000000+tv_old.tv_usec))/33333;

        // FPS measure every 100 frames
        if( (frame%100) == 0){
            int timediff = (tv_new.tv_sec*1000000+tv_new.tv_usec)-(tv_veryold.tv_sec*1000000+tv_veryold.tv_usec);
            printf("[FPS @ %lld | %ld] %d avg FPS = 100 frames / %d usecs\n", tv_new.tv_sec, tick, 100000000/timediff, timediff);
            tv_veryold = tv_new;
        }

        // GPIO test w/ edge guard per-frame
        if(gpio_get_level(4) == 1 ){ gpio_edge = true; }
        if(gpio_get_level(4) == 0 && gpio_edge){
            gpio_edge = false;
            
            scene=(scene+1)%SCENE_MAX;
            printf("[i] NEW SCENE: %d\n", scene);
            switch(scene){
                case SCENE_LANDSCAPE:{
                    load_scene_landscape(&displaybuf, &tmap, &tset);
                    break; }
                case SCENE_FIRE:{
                    load_scene_fire(&displaybuf);
                    break; }
                case SCENE_TILES:{
                    load_scene_tiles(&displaybuf, &tmap, &tset);
                    break; }
                case SCENE_IMAGE:{
                    load_scene_image(&displaybuf, &ball);
                    break; }
                default:{ break; }
            }
        }
    }

}
Macchiato32 - A Framework for ESP32-based game consoles
=======================================================

Install
=======

Simply include this into your ESP-IDF components.

Usage
=====

Before you start your project, use `idf.py menuconfig` to specify the hardware you're building your game for.

After setup is complete, you can start coding with very little setup required:

```C
#include "macchiato32/m32.h"

void app_main(void){

    // Get info about the device's display and create a backbuffer
    m32_display_t display = m32_display_init();
    m32_graphic_t screen = m32_new_graphic(display.width, display.height);

    // If needed, set the backlight. Valid brightness should usually be between 0 and 100
    m32_display_backlight_set(50);

    while(1){
        // Game logic

        // Draw the backbuffer to the screen
        m32_display_draw(&screen);
    }
}
```


API Reference
=============

*(This API reference is aimed at game developers using Macchiato32 to program games on ESP32 hardware. If you want to contribute to Macchiato32 and are looking for reference on the internal drivers, check out `doc/`.)*

Visuals
-------

Macchiato32's core API is based on three components for displaying images:

1. A global Color palette
2. A single global Display, defined through your project's `sdkconfig`
3. Multiple, user-definable Graphics

Optionally, Macchiato32's graphics also support the following extensions:

1. Tilesets to store same-size images in a texture atlas
2. Tilemaps to assemble images using a Tileset

The following subsections follow alphabetical order for convenience.

### Custom Typedefs

- `typedef int16_t m32_coord_t`: Screen coordinates
- `typedef uint32_t m32_color_t`: Colors of pixel format `M32PIXEL_FORMAT_ARGB8888`.
- `typedef enum m32_response_t`: Error codes
- `typedef enum m32_pixel_format_t`: Pixel format identifiers. Only used for display information.

### Displays

```C
typedef struct m32_display{
    m32_pixel_format_t pixel_format;
    m32_coord_t width, height;
    uint8_t backlight_max_brightness;
} m32_display_t;
```

- `pixel_format`: Pixel format identifier
- `width` and `height`: Screen dimensions
- `max_brightness`: Maximum brightness of the display.

#### Functions

```C
m32_display_t m32_display_init();
```

Initializes the display driver and returns information about the currently-used display.

```C
m32_response_t m32_display_draw(m32_graphic_t* image);
```

Displays a graphic on-screen. The image is required to have at least the same size as the display itself. Any larger image will display aligned at the top left corner.

```C
m32_response_t m32_display_backlight_set(uint8_t brightness);
```

If the display has a programmable backlight, this funtion will set it's brightness.

### Graphics

Graphics are kept in-memory as linearized two-dimensional arrays of palette indices.

```C
typedef struct m32_graphic{
	bool use_colorkey;
	uint8_t width;
	uint8_t height;
	uint8_t left_offset;
	uint8_t top_offset;
	uint8_t colorkey;
	uint8_t* buffer;
} m32_graphic_t;
```

- `width` and `height` define the image's size in pixels
- `buffer` contains the image's pixel data in form of palette indices.
- `colorkey` defines a palette index to mark transparent pixels of the image. Transparency can be enabled or disabled by setting `use_colorkey`.
- `left_offset` and `top_offset` are designed to make screen coordinates more closely resemble the game world. For example, a running character may have it's offset be placed at the bottom of their feet. These offsets are currently unused.

#### Functions

```C
m32_graphic_t m32_new_graphic(m32_coord_t width, m32_coord_t height);
m32_response_t m32_free_graphic(m32_graphic_t* m32_graphic);
```

Allocates/deletes a graphic from memory.

```C
m32_response_t m32_graphic_fill_rect(m32_graphic_t* dest,  m32_coord_t x, m32_coord_t y, m32_coord_t w, m32_coord_t h, uint8_t* pcolor);`
```

Fills a rectengular area within `dest` with a solid palettized color `pcolor`.

```C
m32_response_t m32_graphic_draw(m32_graphic_t* dest, m32_graphic_t* src, m32_coord_t x, m32_coord_t y);
```

Draws `src` onto `dest`. If `src->use_colorkey` is set, colors from `src` matching `src->colorkey` are ignored. 


```C
m32_response_t m32_graphic_copy(m32_graphic_t* dest, m32_graphic_t* src);
```

Copies an entire graphic `src` into `dest`. All data within `dest` is overwritten. Transparency is ignored.

### Palettes

Macchiato32 features a single, global array of 256 individual values as it's color palette. As per it being composed of `m32_color_t` types, it's pixel format is assumed to be `M32_PIXEL_FORMAT_ARGB8888`. By default, the palette is filled with black (`0xFF000000`) and colors are generally assumed to be directly written into the palette:


```C
m32_graphic_t = m32_new_graphic
m32_color_t* palette = m32_get_palette();

// Write a simple red gradient into palette
for(int i=0;i<256, i++){
    palette[i] = 0xFF000000 | (i << 16);
}

// Random seed init
srand(time(NULL));

while(1){
    // Draw a fire
    for(int y = display.height-1; y>=0; y--){
        for(int x=0; x<display.width; x++){
            uint8_t above = (y==0)?0:screen[(y-1)*display.width + x];
            uint8_t below = (y==display.height-1)? (rand()%256) : screen[(y+1)*display.width + x];
            
            uint8_t screen[y*display.width + x] = (above+below)/2;
        }
    }
    m32_display_draw(screen);
}
```

#### Functions

- `m32_color_t* m32_get_palette();`: Returns a pointer to the global palette.
- `uint8_t m32_palette_quantize_color(m32_color_t needle);`: Finds the palette color closest to `needle` and returns it's index.
- `m32_response_t m32_palette_write_linear_gradient(uint8_t index_start, uint8_t length, m32_color_t rcolor_start, m32_color_t rcolor_end);`: Writes a linear color gradient from `rcolor_start` to `rcolor_end` into the palette memory starting at `index_start`. If `index_start+length` reaches above the palette's 256 color limit, the gradient is shortened.

### Pixel Formats

#### 8 Bit

- `M32_PIXEL_FORMAT_MONO8`: 8-Bit grayscale
- `M32_PIXEL_FORMAT_RGB332`: 256 color display mode

#### 16 Bit

- `M32_PIXEL_FORMAT_ARGB1555`: Symmetrical 16-Bit High Color w/ alpha bit
- `M32_PIXEL_FORMAT_ARGB4444`: Symmetrical 16-Bit Highcolor w/ alpha
- `M32_PIXEL_FORMAT_RGB565`: Asymmetrical 16-Bit High Color

#### 32 Bit

`M32_PIXEL_FORMAT_ARGB8888`: 8 bpc True Color w/ 8-bit alpha

### Tilemaps (EXT)

*(To use Tilemaps and Tilesets, enable `CONFIG_M32_USE_EMULATED_TILE_MAPPING` within your project's `sdkconfig`. Tilemaps and Tilesets can only be included alongside one another.)*

*Tilemaps* are to Tilesets, what graphics are to the color palette - within a Tileset you can define large images by assembling them from a pre-defined set of 16x16 graphics called a *Tileset*. Tilemaps are commonly used to create game worlds in both retro and modern 2D games:

```C
// Basically a 2D array referencing a tileset
typedef struct{
    uint8_t width, height; // Measured in *tiles*, not pixels 
    m32_tileset_t* tileset;
    uint8_t* map;  // Simple malloc() array. 256 tiles per-tileset should be enough.

} m32_tilemap_t;
```

To initialize a Tilemap, use `m32_new_tilemap(m32_tileset_t* tileset, uint8_t width, uint8_t height)`. This function will allocate an array of `width*height` tileset indices (`uint8_t`) before returning a usable Tilemap.

Each Tilemap can reference up to 256 tiles from a single Tileset pointed to by `m32_tilemap_t->tileset`. `m32_tilemap_t->map` references an array of `width*height` containing all Tilemap indices.

Tilemaps can be rendered onto graphics using the following function. `dest_x` and `dest_y` define a pixel offset from the top left corner of `dest` onto which the Tilemap should be drawn `loop_x` and `loop_y` optionally enable Tilemaps to stretch across the entire X/Y-axis of `dest` by looping the Tilemap's indices across their respective axes:

```C
m32_response_t m32_tilemap_draw_to_graphic(
    m32_graphic_t* dest,
    m32_tilemap_t* tilemap,
    m32_coord_t dest_x,
    m32_coord_t dest_y,
    bool loop_x,
    bool loop_y
);
```

To deallocate a tilemap, use `m32_free_tilemap(gmaedrivertilemap_t* tilemap)`.

### Tilesets (EXT)

*(To use Tilemaps and Tilesets, enable `CONFIG_M32_USE_EMULATED_TILE_MAPPING` within your project's `sdkconfig`. Tilemaps and Tilesets can only be included alongside one another.)*

Tilesets contain the image data used to render a Tilemap. To borrow character graphics as a metaphor, Tilemaps define a text while Tilesets define it's font. To allocate a Tileset in memory and receive it, call `m32_new_tileset()`:

```C
typedef struct{
    bool use_colorkey;
    uint8_t colorkey;
    uint8_t* image;     // Serialized tile pixels
} m32_tileset_t;
```

Each Tileset reserves space for 64 tiles of size 16x16px. When called to draw, Tilesets interpret the most significant bits 6 and 7 as flags to mirror tiles along the X and/or Y axis according to the bit scheme `YXii iiii`. This allows use of up to 256 distinct tiles per Tileset at only 25% of the usually required memory space.

All image data within `m32_tileset_t->image` is serialized tile-by-tile first, then row-by-row into a linear array. To address a single pixel `(x,y)` from tile `i` within your tileset's image data, use the following linearization formula. Remember that there cannot be more than 64 tiles of 16x16px size within one tileset:

    0 <= x < 16
    0 <= y < 16
    0 <= i < 64

    pixel = tileset.image[ (i*16*16) + (y*16) + x ];

Just like Graphics, Tilesets can define one palette index across all contained tiles for transparency masking using the attributes `colorkey` and `use_colorkey`.

To draw a single tile from a Tileset onto a Graphic, call the following function. `dest_x` and `dest_y` specify an offset from the top left corner of `dest` onto which the tile should be drawn from it's own top left corner. `src` accepts an index between 0 and 255 referencing the tile to draw. As mentioned above values `src >= 64` are simply references to the first 64 tiles of the Tileset utilizing X/Y mirroring:

```C
m32_response_t m32_tileset_draw_tile_to_graphic(
    m32_graphic_t* dest,
    m32_tileset_t* tileset,
    uint8_t src,
    m32_coord_t dest_x,
    m32_coord_t dest_y);
```

To deallocate a Tileset from memory, call `m32_free_tileset(m32_tileset_t* set)`.

Other Hardware
--------------

### LEDs

Macchiato32 supports two types of LEDs:

1. `m32_simple_led_t`
2. `m32_rgb_led_t`, which consists of three `m32_simple_led_t` values for red

```C
typedef struct m32_simple_led {
    uint8_t gpio;
    uint16_t duty;
    const uint8_t max_brightness;
    uint8_t brightness;
} m32_simple_led_t;

typedef struct m32_rgb_led {
    m32_simple_led_t r;
    m32_simple_led_t g;
    m32_simple_led_t b;
} m32_rgb_led_t;
```

- `gpio`: The GPIO pin the LED is connected to.
- `duty`: Read-only. PWM duty used by the LED driver.
- `max_brightness`/`brightness`: (Maximum) brightness value of the LED
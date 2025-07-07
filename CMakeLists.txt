set(general_c)

list(APPEND general_c graphic.c)
list(APPEND general_c palette.c)
list(APPEND general_c led.c)
list(APPEND general_c m32.c)

if(CONFIG_M32_USE_EMULATED_TILE_MAPPING)
    list(APPEND general_c tiling.c)
endif()

# Display driver includes
set(display_c)
# SPI-controlled displays
if(CONFIG_M32_USE_SPI)
    # Internal SPI driver
    list(APPEND general_c io/spi.c)
    
    # Sitronix ST7789 LCD controller
    if(CONFIG_M32_DISPLAY_TYPE_ST7789)
        list(APPEND display_c display/st7789.c)
    endif(CONFIG_M32_DISPLAY_TYPE_ST7789)
    # Worldsemi WS2812 LED strip
    if(CONFIG_M32_DISPLAY_TYPE_WS2812)
        list(APPEND display_c display/ws2812.c)
    endif(CONFIG_M32_DISPLAY_TYPE_WS2812)

endif(CONFIG_M32_USE_SPI)

idf_component_register(SRCS ${general_c} ${display_c}
                    PRIV_REQUIRES esp_lcd driver
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "priv_include")

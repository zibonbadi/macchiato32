#include "esp_err.h"
#include "driver/spi_master.h"

#include "macchiato32/m32_common.h"
#include "macchiato32/io/spi.h"

static bool spi_initialized = false;

m32_response_t m32_spi_init(){

    if(spi_initialized){ return M32_OK; }

    esp_err_t ret;
    spi_bus_config_t init_config = {
        .mosi_io_num = CONFIG_M32_SPI_PIN_MOSI,
        .miso_io_num = CONFIG_M32_SPI_PIN_MISO,
        .sclk_io_num = CONFIG_M32_SPI_PIN_SCLK
    };
    
    ret = spi_bus_initialize(SPI2_HOST, &init_config, SPI_DMA_CH_AUTO);

    if(ret != ESP_OK){ return M32_ERROR_GENERIC; }   // TODO: Better error codes
    spi_initialized = true;

    printf("SPI bus Initialized.\n");
    return M32_OK;
};

m32_response_t m32_spi_attach(m32_spi_device_t* device){
    esp_err_t ret;
    spi_device_interface_config_t spi_conf = {
        .mode = device->spi_mode,
        .spics_io_num = device->pin_cs,
        .clock_speed_hz = device->clock_hz
    };
    ret = spi_bus_add_device(SPI2_HOST, &spi_conf, &(device->_handle));
    if(ret != ESP_OK){ return M32_ERROR_GENERIC; }   // TODO: Better error codes
    printf("SPI device %d ATTACHED\n", device->pin_cs);
    return M32_OK;
};

m32_response_t m32_spi_detach(m32_spi_device_t* device){
    esp_err_t ret;
    ret = spi_bus_remove_device(device->_handle);
    if(ret != ESP_OK){ return M32_ERROR_GENERIC; }   // TODO: Better error codes
    printf("SPI device %d DETACHED\n", device->pin_cs);
    return M32_OK;
};


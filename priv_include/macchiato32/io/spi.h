#ifndef M32_IO_SPI_H
#define M32_IO_SPI_H

#include "driver/spi_master.h"


typedef struct m32_spi_device{
    spi_device_handle_t _handle;    // Private
    uint8_t spi_mode;
    uint8_t pin_cs;
    uint8_t clock_hz;
} m32_spi_device_t;

m32_response_t m32_spi_init();
m32_response_t m32_spi_attach(m32_spi_device_t* device);
m32_response_t m32_spi_detach(m32_spi_device_t* device);


#endif
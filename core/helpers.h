#ifndef HELPERS_H
#define HELPERS_H

#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "config.h"


void InitI2C()
{
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(config::kI2C_0_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(config::kI2C_0_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(config::kI2C_0_sda_pin);
    gpio_pull_up(config::kI2C_0_scl_pin);
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(config::kI2C_1_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(config::kI2C_1_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(config::kI2C_1_sda_pin);
    gpio_pull_up(config::kI2C_1_scl_pin);
    printf("I2C initialized \n");
}

void InitSPI()
{
    spi_init(spi1, 2000000);
    spi_set_format( spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(config::kSPI_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(config::kSPI_sck_pin, GPIO_FUNC_SPI);

    printf("SPI initialized \n");
}



#endif // HELPERS_H
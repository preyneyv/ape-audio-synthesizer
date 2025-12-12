#pragma once

#include <stdint.h>

#define SYS_CLOCK_HZ 132000000

/// DISPLAY CONFIGURATION ///
#define DISP_SPI_PORT spi1
#define DISP_SPI_SPEED (40 * 1000 * 1000) // 40 MHz

#define DISP_RST 9
#define DISP_DC 12
#define DISP_SCK 10
#define DISP_MOSI 11
#define DISP_CS 13

/// ENCODERS CONFIGURATION ///
#define ENC_PIO pio0
#define ENC_COUNT 2

#define ENC0_PIN_AB 0
#define ENC0_PIN_PRESS 2

#define ENC1_PIN_AB 18
#define ENC1_PIN_PRESS 20

/// TOUCH CONFIGURATION ///
#define TOUCH_I2C_PORT i2c1
#define TOUCH_I2C_SPEED 100000 // 100 kHz
#define TOUCH_I2C_PIN_SDA 14
#define TOUCH_I2C_PIN_SCL 15
#define TOUCH_I2C_ADDR 0x0D
#define TOUCH_I2C_TIMEOUT_MS 1000 // 1s

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

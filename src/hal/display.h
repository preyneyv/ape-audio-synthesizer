// u8g2 driver for SSD1306 over hardware SPI

#pragma once

#include <u8g2.h>

#include <utils/elm.h>

extern u8g2_t u8g2;
extern elm_t g_elm;

void display_init();

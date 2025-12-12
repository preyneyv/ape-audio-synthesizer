#include <stdio.h>
#include <pico/stdlib.h>

#include <u8g2.h>

#include "display.h"

u8g2_t u8g2;

int main()
{
    stdio_init_all();

    u8g2_Setup_ssd1306_128x64_hwspi_f(&u8g2, U8G2_R0);
    u8g2_InitDisplay(&u8g2);

    u8g2_SetPowerSave(&u8g2, 0);

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawBox(&u8g2, 0, 0, 10, 10);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 15, 10, "Hello, World!");
    u8g2_SendBuffer(&u8g2);

    while (true)
    {
        sleep_ms(1000);
    }
}

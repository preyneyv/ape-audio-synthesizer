#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/clocks.h>

#include <u8g2.h>

#include "hal/display.h"
#include "hal/encoders.h"
#include "hal/keys.h"
#include "hal/audio.h"

void error_trap(const char *msg)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "init failed:");
    u8g2_DrawStr(&u8g2, 0, 22, msg);
    u8g2_SendBuffer(&u8g2);
    while (true)
        sleep_ms(1000);
}

int main()
{
    set_sys_clock_hz(SYS_CLOCK_HZ, true);

    stdio_init_all();
    sleep_ms(1000);
    printf("hi\n");

    display_init();

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "Initializing...");
    u8g2_SendBuffer(&u8g2);

    encoders_init();
    if (keys_init() != 0)
        error_trap("keys_init");

    audio_init();

    uint8_t v[2] = {0, 0};
    while (true)
    {
        encoders_tick();
        keys_tick();
        v[0] += g_encoders[0].delta;
        v[1] += g_encoders[1].delta;

        char buf[8];

        u8g2_ClearBuffer(&u8g2);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);

        snprintf(buf, sizeof(buf), "%03d", v[0]);
        u8g2_DrawStr(&u8g2, 0, 10, buf);

        snprintf(buf, sizeof(buf), "%03d", g_encoders[0].delta);
        u8g2_DrawStr(&u8g2, 0, 22, buf);

        if (g_encoders[0].pressed)
            u8g2_DrawBox(&u8g2, 0, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 0, 26, 8, 8);

        if (g_encoders[0].edge)
            u8g2_DrawBox(&u8g2, 10, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 10, 26, 8, 8);

        snprintf(buf, sizeof(buf), "%03d", v[1]);
        u8g2_DrawStr(&u8g2, 64, 10, buf);

        snprintf(buf, sizeof(buf), "%03d", g_encoders[1].delta);
        u8g2_DrawStr(&u8g2, 64, 22, buf);

        if (g_encoders[1].pressed)
            u8g2_DrawBox(&u8g2, 64, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 64, 26, 8, 8);

        if (g_encoders[1].edge)
            u8g2_DrawBox(&u8g2, 74, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 74, 26, 8, 8);

        u8g2_SendBuffer(&u8g2);
    }
}

#include <stdio.h>

#include "scenes.h"

#include "hal/display.h"
#include "hal/encoders.h"
#include "tracker/tracker.h"

static void enter()
{
    g_tracker.tempo = 120;
}

static void update()
{
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "TEMPO");

    char tempo_str[16];
    snprintf(tempo_str, sizeof(tempo_str), "%d BPM", g_tracker.tempo);
    u8g2_SetFont(&u8g2, u8g2_font_10x20_tr);
    u8g2_DrawStr(&u8g2, 0, 30, tempo_str);

    g_tracker.tempo += g_encoders[0].delta;
    if (g_tracker.tempo < 80)
        g_tracker.tempo = 80;
    if (g_tracker.tempo > 180)
        g_tracker.tempo = 180;

    if (g_encoders[0].pressed)
    {
        scene_switch(&scene_play);
    }
}

static void leave()
{
}

scene_t scene_setup = {
    .enter = enter,
    .update = update,
    .leave = leave,
};

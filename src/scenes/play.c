#include <stdio.h>
#include "scenes.h"
#include "hal/display.h"
#include "hal/encoders.h"
#include "tracker/tracker.h"

static void enter()
{
    // lock setup values
    tracker_enter_play();
}

static void update()
{
    instrument_t *inst = &g_instruments[g_tracker.instrument_idx];

    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "INSTRUMENT");

    char octave_str[32];
    snprintf(octave_str, sizeof(octave_str), "C%d", g_tracker.octave);
    u8g2_DrawStr(&u8g2, 114, 10, octave_str);

    if (g_tracker.record)
    {
        u8g2_DrawStr(&u8g2, 108, 50, "REC");
    }

    u8g2_SetFont(&u8g2, u8g2_font_10x20_tr);
    u8g2_DrawStr(&u8g2, 0, 30, inst->name);

    float ratio = g_tracker.buffer_pos / (float)g_tracker.buffer_end;

    u8g2_DrawFrame(&u8g2, 0, 54, 128, 10);
    u8g2_DrawBox(&u8g2, 0, 54, (uint32_t)128 * (ratio), 10);

    uint8_t beat = (uint8_t)(ratio * 4);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (i == beat)
        {
            u8g2_DrawBox(&u8g2, i * 10, 44, 8, 8);
        }
        else
        {
            u8g2_DrawFrame(&u8g2, i * 10, 44, 8, 8);
        }
    }

    tracker_change_instrument(g_encoders[0].delta);
    if (g_encoders[0].pressed && g_encoders[0].edge)
    {
        tracker_toggle_record();
    }
}

static void leave()
{
}

scene_t scene_play = {
    .enter = enter,
    .update = update,
    .leave = leave,
};

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

    char octave_str[8];
    snprintf(octave_str, sizeof(octave_str), "OCT %d", g_tracker.octave);
    u8g2_DrawStr(&u8g2, 90, 10, octave_str);

    u8g2_SetFont(&u8g2, u8g2_font_10x20_tr);
    u8g2_DrawStr(&u8g2, 0, 30, inst->name);

    tracker_change_instrument(g_encoders[0].delta);
}

static void leave()
{
}

scene_t scene_play = {
    .enter = enter,
    .update = update,
    .leave = leave,
};

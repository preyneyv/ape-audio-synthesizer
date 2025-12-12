#include "scenes.h"
#include "hal/display.h"

static void enter()
{
}

static void update()
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "Play Scene");
    u8g2_SendBuffer(&u8g2);
}

static void exit()
{
}

scene_t scene_play = {
    .enter = enter,
    .update = update,
    .exit = exit,
};

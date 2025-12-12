#pragma once

#include <stdint.h>
#include "../config.h"

typedef struct encoder_state_t
{
    uint _sm;
    uint _pin_press;

    int32_t _last_value;
    int32_t delta;

    bool pressed;
    bool edge;
} encoder_state_t;

extern encoder_state_t g_encoders[ENC_COUNT];

void encoders_init();
void encoders_tick();

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../config.h"

typedef struct encoder_t
{
    uint32_t _sm;
    uint32_t _pin_press;

    int32_t _last_value;
    int32_t delta;

    bool pressed;
    bool edge;
} encoder_t;

extern encoder_t g_encoders[ENC_COUNT];

void encoders_init();
void encoders_tick();

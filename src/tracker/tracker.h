#pragma once

#include <stdint.h>

typedef struct tracker_t
{
    uint8_t tempo;
} tracker_t;

extern tracker_t g_tracker;

void tracker_init();

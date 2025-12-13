#pragma once

#include <stdint.h>

#include <synth/synth.h>
#include "instruments.h"

typedef struct tracker_t
{
    uint8_t tempo;
    uint8_t instrument_idx;
    uint8_t octave;
} tracker_t;

extern tracker_t g_tracker;

void tracker_init();
void tracker_tick();
void tracker_change_instrument(int8_t delta);
void tracker_set_instrument(uint8_t instrument_idx);
void tracker_enter_play();

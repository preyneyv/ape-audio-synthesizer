#pragma once

#include <stdint.h>

#include <synth/synth.h>
#include "instruments.h"

#define BUFFER_MAX_DURATION_SECONDS 4 // 4 seconds
#define BUFFER_SAMPLE_RATE 16000      // 16kHz

typedef struct tracker_t
{
    uint8_t tempo;
    uint8_t instrument_idx;
    uint8_t octave;

    int16_t buffer[(BUFFER_SAMPLE_RATE * BUFFER_MAX_DURATION_SECONDS)]; // 4 seconds of audio at 16kHz
    uint32_t buffer_end;
    uint32_t buffer_pos;
} tracker_t;

extern tracker_t g_tracker;

void tracker_init();
void tracker_tick();
void tracker_change_instrument(int8_t delta);
void tracker_set_instrument(uint8_t instrument_idx);
void tracker_enter_play();
void tracker_process_audio(const int32_t *input, int32_t *output);
void tracker_change_octave(int8_t delta);

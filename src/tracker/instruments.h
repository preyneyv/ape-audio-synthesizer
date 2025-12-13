#pragma once

#include <synth/synth.h>

typedef enum
{
    INSTR_SRC_SYNTH,
    INSTR_SRC_MIC,
    INSTR_SRC_LINE,
} instrument_source_t;

typedef struct instrument_t
{
    char name[16];
    instrument_source_t source;
    audio_synth_operator_config_t config[AUDIO_SYNTH_OPERATOR_COUNT];
} instrument_t;

#define INSTRUMENT_COUNT 5

extern instrument_t g_instruments[INSTRUMENT_COUNT];
void instruments_init();

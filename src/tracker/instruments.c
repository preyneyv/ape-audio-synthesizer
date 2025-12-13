#include "instruments.h"

instrument_t g_instruments[INSTRUMENT_COUNT];

void instruments_init()
{
    g_instruments[0] = (instrument_t){
        .name = "Bongos",
        .source = INSTR_SRC_SYNTH,
        .config = {
            {
                .freq_mult = 6,
                .level = q1x15_f(0.4f),
                .mode = AUDIO_SYNTH_OP_MODE_ADDITIVE,
                .env = {
                    .a = 2,
                    .d = 50,
                    .s = q1x31_f(0.0f),
                    .r = 100,
                },
            },
            {
                .freq_mult = 1,
                .level = q1x15_f(0.5f),
                .mode = AUDIO_SYNTH_OP_MODE_FREQ_MOD,
                .env = {
                    .a = 2,
                    .d = 150,
                    .s = q1x31_f(0.f),
                    .r = 100,
                },
            },
            audio_synth_operator_config_default,
            audio_synth_operator_config_default,
        },
    };

    g_instruments[1] = (instrument_t){
        .name = "Celesta",
        .source = INSTR_SRC_SYNTH,
        .config = {
            {
                .freq_mult = 8,
                .level = q1x15_f(0.8f),
                .mode = AUDIO_SYNTH_OP_MODE_ADDITIVE,
                .env = {
                    .a = 2,
                    .d = 500,
                    .s = q1x31_f(0.2f),
                    .r = 100,
                },
            },
            {
                .freq_mult = 1,
                .level = q1x15_f(0.6f),
                .mode = AUDIO_SYNTH_OP_MODE_FREQ_MOD,
                .env = {
                    .a = 2,
                    .d = 500,
                    .s = q1x31_f(0.2f),
                    .r = 100,
                },
            },
            audio_synth_operator_config_default,
            audio_synth_operator_config_default,
        },
    };

    g_instruments[2] = (instrument_t){
        .name = "Guitar",
        .source = INSTR_SRC_SYNTH,
        .config = {
            {.freq_mult = 1,
             .level = q1x15_f(0.8f),
             .mode = AUDIO_SYNTH_OP_MODE_ADDITIVE,
             .env = {
                 .a = 2,
                 .d = 1500,
                 .s = Q1X31_ZERO,
                 .r = 50}},
            {.freq_mult = 1, .level = q1x15_f(0.6f), .mode = AUDIO_SYNTH_OP_MODE_FREQ_MOD, .env = {.a = 2, .d = 1500, .s = Q1X31_ZERO, .r = 100}},
            audio_synth_operator_config_default,
            audio_synth_operator_config_default,
        }};

    g_instruments[3] = (instrument_t){
        .name = "Microphone",
        .source = INSTR_SRC_MIC,
    };

    g_instruments[4] = (instrument_t){
        .name = "Line In",
        .source = INSTR_SRC_LINE,
    };
}

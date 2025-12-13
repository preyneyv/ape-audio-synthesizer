#include "tracker.h"
#include "hal/keys.h"

tracker_t g_tracker;

void tracker_init()
{
    instruments_init();
    g_synth.master_level = q1x15_f(0.5f); // todo: move elsewhere

    g_tracker.octave = 4;
    g_tracker.tempo = 120;
    tracker_set_instrument(1);
}

void tracker_tick()
{
    uint8_t base_note = 24 + (g_tracker.octave * 12);
    for (int i = 0; i < KEY_NOTE_COUNT; i++)
    {
        key_t *key = &g_keys[i];
        if (key->edge)
        {
            if (key->pressed)
            {
                uint8_t voice = audio_synth_next_voice(&g_synth);
                key->voice = voice;
                audio_synth_enqueue(&g_synth,
                                    &(audio_synth_message_t){
                                        .type = AUDIO_SYNTH_MESSAGE_NOTE_ON,
                                        .data.note_on =
                                            {
                                                .voice = voice,
                                                .note_number = base_note + i,
                                                .velocity = 100,
                                            },
                                    });
            }
            else
            {
                audio_synth_enqueue(&g_synth,
                                    &(audio_synth_message_t){
                                        .type = AUDIO_SYNTH_MESSAGE_NOTE_OFF,
                                        .data.note_off =
                                            {
                                                .voice = key->voice,
                                            },
                                    });
            }
        }
    }
}

void tracker_enter_play()
{
}

void tracker_set_instrument(uint8_t instrument_idx)
{
    g_tracker.instrument_idx = instrument_idx;
    instrument_t *instr = &g_instruments[g_tracker.instrument_idx];
    if (instr->source == INSTR_SRC_SYNTH)
    {
        for (int i = 0; i < AUDIO_SYNTH_OPERATOR_COUNT; i++)
        {
            audio_synth_operator_set_all_config(&g_synth, i, instr->config[i]);
        }
    }
    else
    {
        for (int i = 0; i < AUDIO_SYNTH_OPERATOR_COUNT; i++)
        {
            audio_synth_operator_set_all_config(&g_synth, i, audio_synth_operator_config_default);
        }
    }
}

void tracker_change_instrument(int8_t delta)
{
    if (delta == 0)
        return;
    int8_t new_idx = (int8_t)g_tracker.instrument_idx + delta;
    if (new_idx < 0)
        new_idx = INSTRUMENT_COUNT - 1;
    else if (new_idx >= INSTRUMENT_COUNT)
        new_idx = 0;

    tracker_set_instrument((uint8_t)new_idx);
}

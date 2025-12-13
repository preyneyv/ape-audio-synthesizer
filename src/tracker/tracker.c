#include <stdio.h>
#include "tracker.h"
#include "hal/keys.h"
#include "../config.h"

tracker_t g_tracker;

void tracker_init()
{
    instruments_init();
    g_synth.master_level = q1x15_f(0.5f); // todo: move elsewhere

    g_tracker.octave = 4;
    g_tracker.tempo = 120;
    g_tracker.initialized = false;
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

    if (g_keys[KEY_OCT_DN].edge && g_keys[KEY_OCT_DN].pressed)
    {
        tracker_change_octave(-1);
    }
    if (g_keys[KEY_OCT_UP].edge && g_keys[KEY_OCT_UP].pressed)
    {
        tracker_change_octave(1);
    }
}

void tracker_enter_play()
{
    // calculate end pointer using bpm at 16khz
    g_tracker.buffer_end = (BUFFER_SAMPLE_RATE * 60 * BUFFER_MAX_DURATION_SECONDS) / g_tracker.tempo / 2;
    g_tracker.initialized = true;
}

void tracker_set_instrument(uint8_t instrument_idx)
{
    g_tracker.record = false;
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

void tracker_toggle_record()
{
    g_tracker.record = !g_tracker.record;
}

void tracker_change_octave(int8_t delta)
{
    if (delta == 0)
        return;
    int8_t new_octave = (int8_t)g_tracker.octave + delta;
    if (new_octave < 2)
        new_octave = 2;
    else if (new_octave > 7)
        new_octave = 7;

    g_tracker.octave = (uint8_t)new_octave;
}

static void decimate_48_to_16khz(const int32_t *src, int32_t *dst, size_t num_dst_frames)
{
    // simple decimation by averaging every 3 samples
    for (size_t i = 0; i < num_dst_frames; i++)
    {
        dst[i] = (src[i * 3] + src[i * 3 + 1] + src[i * 3 + 2]) / 3;
    }
}

static void inflate_16_to_48khz(const int32_t *src, int32_t *dst, size_t num_src_frames)
{
    // simple inflation by repeating each sample 3 times
    for (size_t i = 0; i < num_src_frames; i++)
    {
        dst[i * 3] = src[i];
        dst[i * 3 + 1] = src[i];
        dst[i * 3 + 2] = src[i];
    }
}

static inline void extract_input_channel(const int32_t *input, int32_t *output, uint8_t offset)
{
    for (size_t i = 0; i < AUDIO_BUFFER_FRAMES; i++)
    {
        output[i] = input[i * 2 + offset] >> 16;
    }
}

void tracker_process_audio(const int32_t *input, int32_t *output)
{
    static int32_t buffer_48khz[AUDIO_BUFFER_FRAMES];
    static int32_t mix_in_16khz[TRACKER_BUFFER_FRAMES];
    static int32_t out_buffer_16khz[TRACKER_BUFFER_FRAMES];

    bool mix_to_out = false;
    switch (g_instruments[g_tracker.instrument_idx].source)
    {
    case INSTR_SRC_SYNTH:
        mix_to_out = true;
        audio_synth_fill_buffer(&g_synth, buffer_48khz, AUDIO_BUFFER_FRAMES);
        break;
    case INSTR_SRC_LINE:
        extract_input_channel(input, buffer_48khz, 0); // left is line input
        break;
    case INSTR_SRC_MIC:
        extract_input_channel(input, buffer_48khz, 1); // right is mic input
        break;
    };
    decimate_48_to_16khz(buffer_48khz, mix_in_16khz, TRACKER_BUFFER_FRAMES);

    if (g_tracker.initialized)
    {
        // Pull samples from the tracker buffer
        for (size_t i = 0; i < TRACKER_BUFFER_FRAMES; i++)
        {
            uint32_t b_idx = g_tracker.buffer_pos++;
            if (g_tracker.buffer_pos >= g_tracker.buffer_end)
                g_tracker.buffer_pos = 0;

            out_buffer_16khz[i] = g_tracker.buffer[b_idx];
            if (mix_to_out)
                out_buffer_16khz[i] += mix_in_16khz[i];
            if (g_tracker.record)
            {
                g_tracker.buffer[b_idx] = out_buffer_16khz[i];
                if (!mix_to_out)
                    g_tracker.buffer[b_idx] += mix_in_16khz[i];
            }
        }
    }

    inflate_16_to_48khz(out_buffer_16khz, buffer_48khz, TRACKER_BUFFER_FRAMES);
    for (size_t i = 0; i < AUDIO_BUFFER_FRAMES; i++)
    {
        // stereo output from mono buffer
        output[i * 2] = buffer_48khz[i] << 16;
        output[i * 2 + 1] = output[i * 2];
    }
}

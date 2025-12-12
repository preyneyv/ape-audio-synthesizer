#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/clocks.h>

#include <u8g2.h>

#include "hal/display.h"
#include "hal/encoders.h"
#include "hal/keys.h"
#include "hal/audio.h"

#include "synth/synth.h"

void error_trap(const char *msg)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "init failed:");
    u8g2_DrawStr(&u8g2, 0, 22, msg);
    u8g2_SendBuffer(&u8g2);
    while (true)
        sleep_ms(1000);
}

void core1_main()
{
    audio_init();
    while (true)
    {
        __wfi();
    }
}

int main()
{
    set_sys_clock_hz(SYS_CLOCK_HZ, true);

    multicore_reset_core1();
    multicore_launch_core1(core1_main);

    stdio_init_all();
    sleep_ms(1000);
    printf("hi\n");

    display_init();

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "Initializing...");
    u8g2_SendBuffer(&u8g2);

    encoders_init();
    if (keys_init() != 0)
        error_trap("keys_init");

    audio_synth_init(&g_synth, 48000.0f, 1000);
    g_synth.master_level = q1x15_f(0.5f);
    audio_synth_operator_config_t config = audio_synth_operator_config_default;
    config.env = (audio_synth_env_config_t){
        .a = 2,
        .d = 50,
        .s = q1x31_f(0.f), // sustain level
        .r = 50,
    };
    config.freq_mult = 6;
    config.level = q1x15_f(.4f);
    audio_synth_operator_set_all_config(&g_synth, 0, config);

    config = audio_synth_operator_config_default;
    config.env = (audio_synth_env_config_t){
        .a = 2,
        .d = 150,
        .s = q1x31_f(0.f), // sustain level
        .r = 100,
    };
    config.level = q1x15_f(.5f);
    config.mode = AUDIO_SYNTH_OP_MODE_FREQ_MOD;
    audio_synth_operator_set_all_config(&g_synth, 1, config);

    uint8_t v[2] = {0, 0};
    while (true)
    {
        encoders_tick();
        keys_tick();

        v[0] += g_encoders[0].delta;
        v[1] += g_encoders[1].delta;

        char buf[8];

        u8g2_ClearBuffer(&u8g2);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);

        snprintf(buf, sizeof(buf), "%03d", v[0]);
        u8g2_DrawStr(&u8g2, 0, 10, buf);

        snprintf(buf, sizeof(buf), "%03d", g_encoders[0].delta);
        u8g2_DrawStr(&u8g2, 0, 22, buf);

        if (g_encoders[0].pressed)
            u8g2_DrawBox(&u8g2, 0, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 0, 26, 8, 8);

        if (g_encoders[0].edge)
            u8g2_DrawBox(&u8g2, 10, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 10, 26, 8, 8);

        snprintf(buf, sizeof(buf), "%03d", v[1]);
        u8g2_DrawStr(&u8g2, 64, 10, buf);

        snprintf(buf, sizeof(buf), "%03d", g_encoders[1].delta);
        u8g2_DrawStr(&u8g2, 64, 22, buf);

        if (g_encoders[1].pressed)
            u8g2_DrawBox(&u8g2, 64, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 64, 26, 8, 8);

        if (g_encoders[1].edge)
            u8g2_DrawBox(&u8g2, 74, 26, 8, 8);
        else
            u8g2_DrawFrame(&u8g2, 74, 26, 8, 8);

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
                                                    .note_number = 60 + i,
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

        u8g2_SendBuffer(&u8g2);
    }
}

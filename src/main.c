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
#include "scenes/scenes.h"

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

scene_t *g_current_scene;

void scene_switch(scene_t *new_scene)
{
    if (g_current_scene && g_current_scene->exit)
        g_current_scene->exit();
    g_current_scene = new_scene;
    if (g_current_scene->enter)
        g_current_scene->enter();
}

int main()
{
    set_sys_clock_hz(SYS_CLOCK_HZ, true);

    multicore_reset_core1();
    multicore_launch_core1(core1_main);

    stdio_init_all();
    sleep_ms(1000);
    printf("hi\n");

    // initialize all peripherals
    display_init();

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "Initializing...");
    u8g2_SendBuffer(&u8g2);

    encoders_init();
    if (keys_init() != 0)
        error_trap("keys_init");

    // initialize synthesizer with default sine wave
    audio_synth_init(&g_synth, 48000.0f, 1000);
    g_synth.master_level = q1x15_f(0.5f); // todo: move elsewhere
    audio_synth_operator_config_t config = audio_synth_operator_config_default;
    audio_synth_operator_set_all_config(&g_synth, 0, config);

    config = audio_synth_operator_config_default;
    config.level = q1x15_f(0.3f);
    audio_synth_operator_set_all_config(&g_synth, 1, config);

    scene_switch(&scene_setup);
    while (true)
    {
        encoders_tick();
        keys_tick();

        u8g2_ClearBuffer(&u8g2);

        if (g_current_scene->update)
            g_current_scene->update();

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
                                                    .note_number = 72 + i,
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

#pragma once

#define KEY_COUNT 15
#define KEY_NOTE_COUNT 13
#define KEY_OCT_UP 13
#define KEY_OCT_DN 14

typedef struct key_t
{
    uint8_t idx;
    bool pressed;
    bool edge;

    uint8_t voice;
} key_t;

extern key_t g_keys[KEY_COUNT];

int keys_init();
void keys_tick();

#pragma once

typedef struct scene_t
{
    void (*enter)(void);
    void (*exit)(void);
    void (*update)(void);
} scene_t;

void scene_switch(scene_t *new_scene);

extern scene_t scene_setup;
extern scene_t scene_play;

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "ili9341.h"
#include "sounds.h"

enum anims {
    anim_idle_loop,
    anim_sad_loop,
    anim_lowbatt_loop,
    anim_sleep_loop,
    anim_left_loop,
    anim_dead_loop,
    anim_eat,
    anim_backflip,
    anim_dance,
    anim_fall_asleep,
    anim_leave,
    anim_die
};

// MUTATORS
void incrementFrameCount();
void triggerAnim(enum anims);
void setLoop(enum anims);
void endAnim();

// MENU BACKGROUND
void drawMenuBackground();

// SPRITE GRAPHICS
void drawSprite();
void drawCatchStarsCatcher(uint16_t x, uint16_t y);
void drawHudBackground();
void updateHudDisplay(uint8_t, uint8_t);


uint8_t clamp(uint8_t, uint8_t, uint8_t);

#endif
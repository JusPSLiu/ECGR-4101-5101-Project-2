#ifndef CHARACTER_H
#define CHARACTER_H

#include "ili9341.h"
#include "sprite.h"

uint8_t battery_percent = 100;
uint8_t happy_percent = 100;
uint8_t is_dead = 0;

// ACTIONS
void feed();
void play();

// MUTATORS
void updateHappy();
void updateBattery();

// INTERNAL LOGIC
void characterLogic();
void updateAnimState();

#endif
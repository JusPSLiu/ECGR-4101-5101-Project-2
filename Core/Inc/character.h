#ifndef CHARACTER_H
#define CHARACTER_H

#include "ili9341.h"
#include "graphics_controller.h"

// ACTIONS
void feed();
void play();

// MUTATORS
   void updateStats(uint32_t now);   /* call every loop pass with HAL_GetTick() */

// INTERNAL LOGIC
void characterLogic();
void updateAnimState();
uint8_t clamp(uint8_t, uint8_t, uint8_t);

void updateHud();


#endif
#ifndef MENU_H
#define MENU_H

#include "ili9341.h"
// #include "character.h"

#define SCREEN_HEIGHT 320U
#define SCREEN_WIDTH  240U
#define VERTICAL_SPACE 33U
#define TEXT_SIZE 4
#define MENU_OPTIONS_COUNT 7

uint8_t drawMenu(int menuState);

#endif

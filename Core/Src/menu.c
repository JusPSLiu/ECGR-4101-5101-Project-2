#include "menu.h"

uint8_t drawMenu(int menuState) {
  char *menuoptions[] = {
    "  FEED  ",
    "  PLAY  ",
    "  DANCE  ",
    "  THING  ",
    "  STARS  "};
  char *selectedoptions[] = {
    "> FEED <",
    "> PLAY <",
    "> DANCE <",
    "> THING <",
    "> STARS <"};
  uint8_t num_options = 5;
  
  // wraparound
  if (menuState < 0) menuState = num_options - 1;
  else menuState %= num_options;

  // draw
  uint8_t y_pos = 0;
  for (int i=0;i<num_options;i++) {
    char *curr_str;
    short color;

    if (menuState == i) {curr_str = selectedoptions[i]; color=ILI9341_COLOR_BLACK;}
    else {curr_str = menuoptions[i]; color=ILI9341_COLOR_PINK;}

    ILI9341_DrawString(0 , y_pos, curr_str, color, ILI9341_COLOR_WHITE, 4);
    y_pos += 33;
  }

  return menuState;
}

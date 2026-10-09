#include "menu.h"

void drawMenu(uint8_t menuState) {
  char *menuoptions[] = {
    "  FEED  ",
    "  PLAY  ",
    "  SOME  ",
    " THING  "};
  char *selectedoptions[] = {
    "> FEED <",
    "> PLAY <",
    "> SOME <",
    ">THING <"};
  uint8_t num_options = 4;
  
  // wraparound
  menuState %= num_options;
  if (menuState < 0) menuState = num_options-1;

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
}
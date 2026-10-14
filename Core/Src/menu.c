#include "menu.h"
#include "sounds.h"

uint8_t drawMenu(int menuState) {
  PlayMenuCycle();

  char *menuoptions[] = {
    "   FEED   ",
    "   JIVE   ",
    "   STAR   ",
    "  REBOOT  ",
    "   EXIT   "};
  char *selectedoptions[] = {
    ">  FEED  <",
    ">  JIVE  <",
    ">  STAR  <",
    "> REBOOT <",
    ">  EXIT  <"};
  uint8_t num_options = 5;
  
  // wraparound
  if (menuState < 0) menuState = num_options - 1;
  else menuState %= num_options;

  // draw
  uint8_t y_pos = (SCREEN_HEIGHT - (num_options * VERTICAL_SPACE)) / 2;
  for (int i=0;i<num_options;i++) {
    char *curr_str;
    short color;

    if (menuState == i) {curr_str = selectedoptions[i]; color=ILI9341_COLOR_GREEN;}
    else {curr_str = menuoptions[i]; color=ILI9341_COLOR_BLACK;}
    
    // int x = (SCREEN_WIDTH  - sizeof(menuoptions) / sizeof(menuoptions[i])) / 2;
    ILI9341_DrawString(0 , y_pos, curr_str, color, ILI9341_COLOR_WHITE, TEXT_SIZE);
    y_pos += VERTICAL_SPACE;
  }

  return menuState;
}

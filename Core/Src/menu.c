#include "ili9341.h"
#include "menu.h"

void drawMenu(int menu_state) {
  switch (menu_state) {
    case 0:
      ILI9341_DrawString(0 , 7 * (0 * 4) + (5 * 0), " > FEED < ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (1 * 4) + (5 * 1), "   PLAY   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (2 * 4) + (5 * 2), "   WALK   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (3 * 4) + (5 * 3), "   MYST   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      break;

    case 1:
      ILI9341_DrawString(0 , 7 * (0 * 4) + (5 * 0), "   FEED   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (1 * 4) + (5 * 1), " > PLAY < ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (2 * 4) + (5 * 2), "   WALK   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (3 * 4) + (5 * 3), "   MYST   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      
      break;

    case 2:
      ILI9341_DrawString(0 , 7 * (0 * 4) + (5 * 0), "   FEED   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (1 * 4) + (5 * 1), "   PLAY   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (2 * 4) + (5 * 2), " > WALK < ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (3 * 4) + (5 * 3), "   MYST   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      break;

    case 3:
      ILI9341_DrawString(0 , 7 * (0 * 4) + (5 * 0), "   FEED   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (1 * 4) + (5 * 1), "   PLAY   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (2 * 4) + (5 * 2), "   WALK   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (3 * 4) + (5 * 3), " > MYST < ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      break;

    default:
      ILI9341_DrawString(0 , 7 * (0 * 4) + (5 * 0), " > FEED < ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (1 * 4) + (5 * 1), "   PLAY   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (2 * 4) + (5 * 2), "   WALK   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      ILI9341_DrawString(0 , 7 * (3 * 4) + (5 * 3), "   MYST   ", ILI9341_COLOR_PINK, ILI9341_COLOR_WHITE, 4);
      break;
  }
}
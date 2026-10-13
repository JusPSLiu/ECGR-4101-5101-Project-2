#ifndef CATCH_STARS_H
#define CATCH_STARS_H

#include <stdint.h>

void catchStarsStart(void);
uint8_t catchStarsIsActive(void);
void catchStarsUpdate(uint8_t move_left, uint8_t move_right,
                      uint8_t center_pressed);

#endif

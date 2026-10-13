#include "graphics_controller.h"
#include "sprite.h"

#define SPRITEX 40
#define SPRITEY 56

uint8_t curr_anim;
uint8_t curr_loop;

uint8_t flairing = 0;
uint8_t frame_count;


// SPRITE ANIMATION
void incrementFrameCount() {
    frame_count++;
}

void drawSprite() {
restart_draw:
    uint8_t animlen = 0;
    const unsigned short **anim;
    switch(curr_anim) {
        case anim_eat:
            anim = eat_anim; animlen = eat_anim_len;
            break;
        case anim_die:
            anim = die_anim; animlen = die_anim_len;
            break;
        case anim_backflip:
            anim = backflip_anim; animlen = backflip_anim_len;
            break;
        case anim_lowbatt_loop:
            anim = lowbatt_anim; animlen = lowbatt_anim_len;
            break;
        case anim_idle_loop:
            anim = idle_anim; animlen = idle_anim_len;
            break;
        case anim_sad_loop:
            anim = sad_anim; animlen = sad_anim_len;
            break;
        case anim_dance:
            anim = floss_anim; animlen = floss_anim_len;
            break;
        case anim_fall_asleep:
            anim = fall_asleep_anim; animlen = fall_asleep_anim_len;
            break;
        case anim_sleep_loop:
            anim = sleep_anim; animlen = sleep_anim_len;
            break;
        case anim_leave:
            anim = leave_anim; animlen = leave_anim_len;
            break;
        case anim_left_loop:
            anim = left_anim; animlen = left_anim_len;
            break;
        case anim_dead_loop:
        default:
            anim = dead_anim; animlen = dead_anim_len;
    }

    if (frame_count >= animlen) {
        frame_count = 0;

        if (flairing) {
            // flair is over, go to loop. restart this function
            curr_anim = curr_loop;
            flairing = 0;
            goto restart_draw;
        }
    }

    //ILI9341_DrawImageScaled(SPRITEX, SPRITEY, 20, 26, (const unsigned short *)anim[frame_count],
    //    20, 160, 208, 0, 0xFFFF);
    ILI9341_DrawImageScaledFast(SPRITEX, SPRITEY, 20, 26, (const unsigned short *)anim[frame_count],
        8);
}

void triggerAnim(enum anims newanim) {
    curr_anim = newanim;
    frame_count = 0;
    flairing = 1;
}
void setLoop(enum anims newanim) {
    if (curr_anim == newanim) return;
    // flair takes priority, but if currently looping then immediately change
    if (!flairing) {
        curr_anim = newanim;
        frame_count = 0;
    }
    curr_loop = newanim;
}




/** GUI AND HUD **/
void drawMenuBackground() {
  ILI9341_FillScreen(ILI9341_COLOR_WHITE);
}
void drawHudBackground() {
  ILI9341_FillScreen(ILI9341_COLOR_WHITE);
  ILI9341_DrawImageScaledFast(4, 6, 41, 40, (const unsigned short *)battery,
          1);
  ILI9341_DrawImageScaledFast(4, 274, 41, 40, (const unsigned short *)heart,
          1);
}
void updateHudDisplay(uint8_t battery_percent, uint8_t happy_percent) {
  uint8_t batt = clamp((battery_percent * 182 / 100), 0, 182);
  uint8_t happ = clamp((happy_percent * 182 / 100), 0, 182);

  // display status bars
  ILI9341_DrawRect(49, 20, 184, 12, ILI9341_COLOR_BLACK);
  short batcolor[] = {0b0111111111101111, ILI9341_COLOR_GREEN, 0b0000001111100000};
  for (int y=21;y<31;y++) {
    uint8_t color_idx = ((y-4)/5) - 3;
    ILI9341_DrawHLine(50, y, batt, batcolor[color_idx]);
    ILI9341_DrawHLine(50+batt, y, 182-batt, ILI9341_COLOR_BLACK);
  }
  ILI9341_DrawRect(49, 288, 184, 12, ILI9341_COLOR_BLACK);
  short barcolor[] = {0b1111101111101111, ILI9341_COLOR_RED, 0b0111100000000000};
  for (int y=289;y<299;y++) {
    uint8_t color_idx = ((y-1)/5) - 57;
    ILI9341_DrawHLine(50, y, happ, barcolor[color_idx]);
    ILI9341_DrawHLine(50+happ, y, 182-happ, ILI9341_COLOR_BLACK);
  }
}

/* UTIL */
inline uint8_t clamp(uint8_t val, uint8_t min, uint8_t max) {
  if (val < min) return min;
  if (val > max) return max;
  return val;
}
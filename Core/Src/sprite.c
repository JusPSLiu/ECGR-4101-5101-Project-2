
#include "sprite.h"

#define SPRITEX 40
#define SPRITEY 56

uint8_t flairing = 0;
uint8_t frame_count;

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

    ILI9341_DrawImageScaled(SPRITEX, SPRITEY, 20, 26, (const unsigned short *)anim[frame_count],
        20, 160, 208, 0, 0xFFFF);
}

void triggerAnim(enum anims newanim) {
    curr_anim = newanim;
    frame_count = 0;
}
void setLoop(enum anims newanim) {
    // flair takes priority, but if currently looping then immediately change
    if (curr_anim == curr_loop) {
        curr_anim = newanim;
        frame_count = 0;
    }
    curr_loop = newanim;
}
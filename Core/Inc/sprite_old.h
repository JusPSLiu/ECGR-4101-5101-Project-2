#ifndef SPRITE_H

#define SPRITE_H

#define ROTATION 10.0f

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define SPRITE_SIZE 50
#define DST_SIZE 225

#define ANIMATION_DELAY ((1 / 16) * 1000)

extern const unsigned short sprite_animation0[][50];
extern const unsigned short sprite_animation1[][50];
extern const unsigned short sprite_animation2[][50];
extern const unsigned short sprite_animation3[][50];
extern const unsigned short sprite_animation4[][50];
extern const unsigned short sprite_animation5[][50];
extern const unsigned short sprite_animation6[][50];
extern const unsigned short sprite_animation7[][50];
extern const unsigned short sprite_animation8[][50];
extern const unsigned short sprite_animation9[][50];
extern const unsigned short sprite_animation10[][50];
extern const unsigned short sprite_animation11[][50];
extern const unsigned short sprite_animation12[][50];
extern const unsigned short sprite_animation13[][50];
extern const unsigned short sprite_animation14[][50];
extern const unsigned short sprite_animation15[][50];
extern const unsigned short sprite_animation16[][50];
extern const unsigned short sprite_animation17[][50];
extern const unsigned short sprite_animation18[][50];
extern const unsigned short sprite_animation19[][50];
extern const unsigned short sprite_animation20[][50];

void drawCelebi(int frame_count, int x, int y);

#endif

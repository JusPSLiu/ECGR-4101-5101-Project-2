#include "catch_stars.h"
#include "graphics_controller.h"

#define CATCH_STARS_WIDTH ILI9341_WIDTH
#define CATCH_STARS_HEIGHT ILI9341_HEIGHT
#define STAR_SIZE 16U
#define PLAYER_SCALE 2U
#define PLAYER_WIDTH (20U * PLAYER_SCALE)
#define PLAYER_HEIGHT (26U * PLAYER_SCALE)
#define PLAYER_Y (CATCH_STARS_HEIGHT - PLAYER_HEIGHT - 8U)
#define PLAYFIELD_TOP 58U
#define PLAYER_MOVE_INTERVAL 35U
#define PLAYER_MOVE_STEP 5
#define STAR_FALL_INTERVAL 30U
#define STAR_FALL_STEP 2U
#define TARGET_SCORE 10U

typedef struct {
  uint8_t first_x;
  uint8_t first_width;
  uint8_t second_x;
  uint8_t second_width;
} StarRow;

static const StarRow star_rows[STAR_SIZE] = {
    {7, 2, 0, 0},  {6, 4, 0, 0},  {5, 6, 0, 0},   {4, 8, 0, 0},
    {0, 16, 0, 0}, {1, 14, 0, 0}, {2, 12, 0, 0},  {3, 10, 0, 0},
    {4, 8, 0, 0},  {5, 6, 0, 0},  {2, 4, 10, 4},  {1, 5, 10, 5},
    {2, 4, 10, 4}, {3, 3, 10, 3}, {4, 2, 10, 2}, {0, 0, 0, 0},
};

static void drawSpaceBackground(void)
{
    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    /* Small distant stars */
    ILI9341_DrawHLine(18, 75, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(65, 92, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(205, 82, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(145, 115, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(35, 145, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(190, 165, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(85, 190, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(220, 220, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(45, 235, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(160, 245, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(105, 275, 2, ILI9341_COLOR_WHITE);
    ILI9341_DrawHLine(200, 285, 2, ILI9341_COLOR_WHITE);

    /* Brighter stars */
    ILI9341_DrawHLine(110, 85, 3, ILI9341_COLOR_CYAN);
    ILI9341_DrawHLine(175, 135, 3, ILI9341_COLOR_PINK);
    ILI9341_DrawHLine(20, 200, 3, ILI9341_COLOR_CYAN);
}

static uint8_t game_active;
static uint8_t center_armed;
static uint8_t victory;
static uint8_t score;
static uint16_t player_x;
static uint16_t star_x;
static uint16_t star_y;
static uint32_t last_player_move;
static uint32_t last_star_fall;
static uint32_t random_state;

static uint16_t nextStarX(void);
static void drawStar(uint16_t x, uint16_t y, uint16_t color);
static void drawGameHud(void);
static void drawVictoryScreen(void);
static void spawnStar(void);

void catchStarsStart(void) {
  uint32_t now = HAL_GetTick();

  game_active = 1;
  center_armed = 0;
  victory = 0;
  score = 0;
  player_x = (CATCH_STARS_WIDTH - PLAYER_WIDTH) / 2U;
  star_y = PLAYFIELD_TOP;
  random_state = now ^ 0xA5A5U;
  last_player_move = now;
  last_star_fall = now;
  spawnStar();

  drawSpaceBackground();
  drawGameHud();
  drawStar(star_x, star_y, ILI9341_COLOR_YELLOW);
  drawCatchStarsCatcher(player_x, PLAYER_Y);
}

uint8_t catchStarsIsActive(void) { return game_active; }

void catchStarsUpdate(uint8_t move_left, uint8_t move_right,
                      uint8_t center_pressed) {
  if (!game_active) return;

  if (!center_pressed) {
    center_armed = 1;
  } else if (center_armed) {
    game_active = 0;
    return;
  }

  if (victory) return;

  uint32_t now = HAL_GetTick();
  uint16_t old_player_x = player_x;
  uint16_t old_star_x = star_x;
  uint16_t old_star_y = star_y;
  uint8_t player_changed = 0;
  uint8_t star_changed = 0;
  uint8_t score_changed = 0;

  if (now - last_player_move >= PLAYER_MOVE_INTERVAL) {
    uint32_t steps = (now - last_player_move) / PLAYER_MOVE_INTERVAL;
    int32_t new_x = player_x;
    last_player_move += steps * PLAYER_MOVE_INTERVAL;

    if (move_left && !move_right) {
      new_x -= (int32_t)(steps * PLAYER_MOVE_STEP);
    } else if (move_right && !move_left) {
      new_x += (int32_t)(steps * PLAYER_MOVE_STEP);
    }

    if (new_x < 0) new_x = 0;
    if (new_x > (int32_t)(CATCH_STARS_WIDTH - PLAYER_WIDTH)) {
      new_x = CATCH_STARS_WIDTH - PLAYER_WIDTH;
    }

    player_x = (uint16_t)new_x;
    player_changed = player_x != old_player_x;
  }

  if (now - last_star_fall >= STAR_FALL_INTERVAL) {
    uint32_t steps = (now - last_star_fall) / STAR_FALL_INTERVAL;
    last_star_fall += steps * STAR_FALL_INTERVAL;
    uint32_t next_star_y = (uint32_t)star_y + steps * STAR_FALL_STEP;
    star_y = next_star_y > PLAYER_Y ? PLAYER_Y : (uint16_t)next_star_y;

    if ((uint32_t)star_y + STAR_SIZE >= PLAYER_Y) {
      if ((uint32_t)star_x + STAR_SIZE > player_x &&
          star_x < (uint32_t)player_x + PLAYER_WIDTH) {
        score++;
        score_changed = 1;
        if (score >= TARGET_SCORE) {
          victory = 1;
        }
      }

      if (!victory) spawnStar();
    }
    star_changed = 1;
  }

  if (player_changed) {
    ILI9341_FillRect(old_player_x, PLAYER_Y, PLAYER_WIDTH, PLAYER_HEIGHT,
                     ILI9341_COLOR_BLACK);
  }
  if (star_changed) {
    ILI9341_FillRect(old_star_x, old_star_y, STAR_SIZE, STAR_SIZE,
                     ILI9341_COLOR_BLACK);
  }
  if (score_changed) drawGameHud();

  if (victory) {
    drawVictoryScreen();
    return;
  }

  if (star_changed) {
    drawStar(star_x, star_y, ILI9341_COLOR_YELLOW);
  }
  if (player_changed || star_changed) {
    drawCatchStarsCatcher(player_x, PLAYER_Y);
  }
}

static uint16_t nextStarX(void) {
  random_state = random_state * 1664525U + 1013904223U;
  return (uint16_t)(random_state % (CATCH_STARS_WIDTH - STAR_SIZE + 1U));
}

static void drawStar(uint16_t x, uint16_t y, uint16_t color) {
  for (uint16_t row = 0; row < STAR_SIZE; row++) {
    const StarRow *span = &star_rows[row];
    if (span->first_width > 0) {
      ILI9341_DrawHLine(x + span->first_x, y + row, span->first_width, color);
    }
    if (span->second_width > 0) {
      ILI9341_DrawHLine(x + span->second_x, y + row, span->second_width,
                        color);
    }
  }
}

static void drawGameHud(void) {
  char score_text[] = "CAUGHT: 00/10";
  score_text[8] = (char)('0' + score / 10U);
  score_text[9] = (char)('0' + score % 10U);

  ILI9341_DrawString(10, 8, "CATCH THE STARS",
                   ILI9341_COLOR_WHITE,
                   ILI9341_COLOR_BLACK, 2);
  ILI9341_FillRect(8, 25, 224, 17, ILI9341_COLOR_WHITE);
  ILI9341_DrawString(10, 27, score_text, ILI9341_COLOR_BLACK,
                     ILI9341_COLOR_WHITE, 2);
  ILI9341_DrawRect(10, 47, 220, 8, ILI9341_COLOR_BLACK);
  ILI9341_FillRect(11, 48, 218, 6, ILI9341_COLOR_WHITE);
  if (score > 0) {
    ILI9341_FillRect(11, 48, (uint16_t)(score * 218U / TARGET_SCORE), 6,
                     ILI9341_COLOR_GREEN);
  }
}

static void drawVictoryScreen(void) {
  ILI9341_FillScreen(ILI9341_COLOR_WHITE);
  ILI9341_DrawString(60, 112, "YOU WIN!", ILI9341_COLOR_GREEN,
                     ILI9341_COLOR_WHITE, 3);
  ILI9341_DrawString(66, 158, "STARS: 10/10", ILI9341_COLOR_BLACK,
                     ILI9341_COLOR_WHITE, 2);
  ILI9341_DrawString(30, 206, "CENTER TO EXIT", ILI9341_COLOR_BLACK,
                     ILI9341_COLOR_WHITE, 2);
}

static void spawnStar(void) {
  star_x = nextStarX();
  star_y = PLAYFIELD_TOP;
}

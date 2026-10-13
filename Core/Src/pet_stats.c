/**
 * @file    pet_stats.c
 * @brief   Hunger and happiness logic + placeholder meters.
 *
 * Owner: Marly (pet stats)
 * See pet_stats.h for the limits and how other code should use this module.
 */

#include "pet_stats.h"
#include "ili9341.h"     /* ILI9341_FillRect, ILI9341_DrawRect, ILI9341_DrawString */
#include <stdio.h>       /* snprintf for the number labels                        */
#include "pet_flash.h"   /* PetFlash_Load / PetFlash_Save (non-volatile storage)  */

/* The pet's stats: one global struct (declared extern in pet_stats.h) so the
   whole thing can be saved to / loaded from flash in one piece. */
PetStats_t pet = { STAT_START, STAT_START, PET_ALIVE };

/* Private state (only this file can touch these) */

static uint32_t last_hunger_tick = 0;     /* time (ms) of the last hunger drop   */
static uint32_t last_happy_tick  = 0;     /* time (ms) of the last happy drop    */
static uint8_t  timers_started   = 0;     /* 0 until the first Update() call     */

/* -Placeholder meter layout */
#define METER_W        100U   /* full bar width in pixels (1 pixel per stat point) */
#define METER_H        10U    /* bar height in pixels                              */
#define METER_GAP      22U    /* vertical distance between the two bars            */
#define LABEL_SIZE     1U     /* text scale for labels                             */
#define LABEL_W        48U    /* space reserved for "HUNGER"/"HAPPY" text          */

/**
 * @brief  Clamp a signed value into [STAT_MIN, STAT_MAX]
 *         Using int16_t lets us add a negative delta without wrapping around
 *         (uint8_t 5 - 10 would wrap to 251 - which is the bug clamping prevents).
 */
static uint8_t clamp_stat(int16_t value) {
  if (value < (int16_t)STAT_MIN) return STAT_MIN;
  if (value > (int16_t)STAT_MAX) return STAT_MAX;
  return (uint8_t)value;
}

/* Setup / reset  */

void PetStats_Init(void) {
  pet.hunger     = STAT_START;   /* start both stats at the same healthy value */
  pet.happiness  = STAT_START;
  pet.state      = PET_ALIVE;
  timers_started = 0;            /* timers re-sync on the next Update()        */
}

void PetStats_Reset(void) {
  PetStats_Init();               /* restart = same as a fresh power-up         */
  PetStats_SaveNow();            /* so a power-off right after restart doesn't bring the old pet back */
}

/* Non-volatile save / load */

static uint32_t   last_save_tick = 0;                          /* time (ms) of the last autosave check */
static PetStats_t saved = { STAT_MAX + 1, STAT_MAX + 1, 0 };   /* what is in flash (impossible = nothing yet) */

/**
 * @brief  Restore the pet struct from flash if a save exists.
 *         Call once at startup, right after PetStats_Init().
 * @retval 1 if saved values were restored, 0 if this is a fresh pet
 */
uint8_t PetStats_LoadSaved(void) {
  PetStats_t loaded;
  if (!PetFlash_Load(&loaded)) return 0;        /* first power-up: keep STAT_START */

  pet.hunger    = clamp_stat((int16_t)loaded.hunger);     /* clamp in case limits changed since the save */
  pet.happiness = clamp_stat((int16_t)loaded.happiness);
  pet.state     = (loaded.state <= PET_RAN_AWAY) ? loaded.state : PET_ALIVE;
  saved = pet;
  return 1;
}

/**
 * @brief  Write the pet struct to flash right away if it changed
 *         Skipping unchanged values saves flash wear.
 */
void PetStats_SaveNow(void) {
  if (pet.hunger == saved.hunger && pet.happiness == saved.happiness &&
      pet.state  == saved.state) return;

  if (PetFlash_Save(&pet)) {
    saved = pet;
  }
}

/* time-based decay */

/**
 * @brief  Lower the stats as time passes - Non-blocking:  only compares the
 *         current time to the last drop time - returns immediately and
 *         never delays the menu or animations
 * @param  now_ms  current time from HAL_GetTick()
 */
void PetStats_Update(uint32_t now_ms) {
  /* first call: start the timers from "now" so the stats don't drop instantly because last_*_tick was still 0 */
  if (!timers_started) {
    last_hunger_tick = now_ms;
    last_happy_tick  = now_ms;
    last_save_tick   = now_ms;
    timers_started   = 1;
    return;
  }

    /* stats only drain while the pet is alive; dead / ran away freezes them
     (the autosave below still runs, so the new state gets saved to flash) */
  if (pet.state == PET_ALIVE) {
    /* unsigned subtraction handles HAL_GetTick() wrapping after ~49 days */
    if (now_ms - last_hunger_tick >= HUNGER_DECAY_MS) {
      last_hunger_tick += HUNGER_DECAY_MS;   /* step forward by one period, keeps timing exact */
      pet.hunger = clamp_stat((int16_t)pet.hunger - (int16_t)HUNGER_DECAY_AMOUNT);
    }

    if (now_ms - last_happy_tick >= HAPPY_DECAY_MS) {
      last_happy_tick += HAPPY_DECAY_MS;
      pet.happiness = clamp_stat((int16_t)pet.happiness - (int16_t)HAPPY_DECAY_AMOUNT);
    }
  } else {
    /* frozen (dead, ran away, or in the mini game): keep the timers at "now"
       so the stats don't suddenly catch up all at once when play resumes */
    last_hunger_tick = now_ms;
    last_happy_tick  = now_ms;
  }

  /* autosave to flash every SAVE_INTERVAL_MS (only writes if something changed) */
  if (now_ms - last_save_tick >= SAVE_INTERVAL_MS) {
    last_save_tick = now_ms;
    PetStats_SaveNow();
  }
}

/*  Actions  */

void PetStats_AddHunger(int16_t delta) {
  pet.hunger = clamp_stat((int16_t)pet.hunger + delta);  /* any change, always clamped */
}

void PetStats_AddHappiness(int16_t delta) {
  pet.happiness = clamp_stat((int16_t)pet.happiness + delta);
}

void PetStats_Feed(void) {
  PetStats_AddHunger((int16_t)FEED_AMOUNT);             /* feeding fills the pet up   */
}

void PetStats_Play(void) {
  PetStats_AddHappiness((int16_t)PLAY_AMOUNT);          /* playing cheers it up       */
  PetStats_AddHunger(-(int16_t)PLAY_HUNGER_COST);       /* a backflip burns a little energy */
}

void PetStats_Dance(void) {
  PetStats_AddHappiness((int16_t)DANCE_HAPPY_AMOUNT);   /* dancing is fun...          */
  PetStats_AddHunger(-(int16_t)DANCE_HUNGER_COST);      /* ...but makes the pet hungry */
}

/**
 * @brief  Mini game starts: switch to PET_IN_GAME. PetStats_Update() only drains
 *         the stats while the pet is PET_ALIVE, so they stay frozen during the game.
 */
void PetStats_StartGame(void) {
  if (pet.state != PET_ALIVE) return;                   /* only a living pet can play */
  pet.state = PET_IN_GAME;
}

/**
 * @brief  Mini game ends: refill both stats to the max and go back to normal.
 */
void PetStats_EndGame(void) {
  if (pet.state != PET_IN_GAME) return;                 /* no game running            */
  pet.hunger    = STAT_MAX;                             /* reward: full hunger...     */
  pet.happiness = STAT_MAX;                             /* ...and full happiness      */
  pet.state     = PET_ALIVE;
  PetStats_SaveNow();                                   /* rare event: save right away */
}

/* Read-only  */

uint8_t PetStats_GetHunger(void)       { return pet.hunger; }
uint8_t PetStats_GetHappiness(void)    { return pet.happiness; }
uint8_t PetStats_IsHungerLow(void)     { return pet.hunger    <= LOW_STAT_THRESHOLD; }
uint8_t PetStats_IsHappinessLow(void)  { return pet.happiness <= LOW_STAT_THRESHOLD; }
uint8_t PetStats_IsStarving(void)      { return pet.hunger    == STAT_MIN; }
uint8_t PetStats_IsMiserable(void)     { return pet.happiness == STAT_MIN; }

/*  Placeholder meters */

/**
 * @brief  Draw one labeled bar - Filled part = current value - rest = background.
 *         Red when the stat is low - green otherwise
 */
static void draw_one_meter(uint16_t x, uint16_t y, const char *label, uint8_t value) {
  char num[6];                                           /* "100" + null, with room */
  uint16_t bar_x  = x + LABEL_W;                         /* bar starts after label  */
  uint16_t filled = (uint16_t)value * METER_W / STAT_MAX;/* width of the filled part*/
  uint16_t color  = (value <= LOW_STAT_THRESHOLD) ? ILI9341_COLOR_RED
                                                  : ILI9341_COLOR_GREEN;

  /* Label on the left */
  ILI9341_DrawString(x, y + 1, label, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, LABEL_SIZE);

  /* Filled part - then the empty part */
  if (filled > 0)
    ILI9341_FillRect(bar_x, y, filled, METER_H, color);
  if (filled < METER_W)
    ILI9341_FillRect(bar_x + filled, y, METER_W - filled, METER_H, ILI9341_COLOR_WHITE);

  /* Outline around the whole bar */
  ILI9341_DrawRect(bar_x, y, METER_W, METER_H, ILI9341_COLOR_BLACK);

  /* Number to the right; padded to 3 chars so old digits get overwritten */
  snprintf(num, sizeof(num), "%3u", value);
  ILI9341_DrawString(bar_x + METER_W + 4, y + 1, num, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, LABEL_SIZE);
}

void PetStats_DrawMeters(uint16_t x, uint16_t y) {
  draw_one_meter(x, y,             "HUNGER", pet.hunger);
  draw_one_meter(x, y + METER_GAP, "HAPPY",  pet.happiness);
}

/* Low-stat alerts */

/* Blink + drawing state: only redraw when something changes - so the alerts
   cost almost nothing over SPI on loop passes where nothing changes */
static uint32_t last_blink_tick = 0;      /* time (ms) of the last on/off flip  */
static uint8_t  blink_on        = 1;      /* 1 = text visible this half second  */
static uint8_t  hunger_alert_shown = 0;   /* what is currently on the screen    */
static uint8_t  happy_alert_shown  = 0;

/**
 * @brief  Draw or erase one alert - but only if it changed since last time
 * @param  shown  pointer to the "currently on screen" flag for this alert
 * @param  want   1 = text should be visible now - 0 = should be blank
 */
static void update_one_alert(uint16_t x, uint16_t y, const char *text,
                             uint8_t *shown, uint8_t want) {
  if (want == *shown) return;                          /* nothing changed: skip SPI */

  /* Width of the text in pixels: each letter is 6 columns (5 + 1 gap) * size */
  uint16_t len = 0;
  while (text[len] != '\0') len++;
  uint16_t w = len * 6U * ALERT_TEXT_SIZE;
  uint16_t h = 7U * ALERT_TEXT_SIZE;

  if (want) {
    ILI9341_DrawString(x, y, text, ILI9341_COLOR_RED, ILI9341_COLOR_WHITE, ALERT_TEXT_SIZE);
  } else {
    ILI9341_FillRect(x, y, w, h, ILI9341_COLOR_WHITE);  /* erase with background */
  }
  *shown = want;
}

/**
 * @brief  Blink HUNGRY! / SAD! while a stat is at or below LOW_STAT_THRESHOLD
 *         Non-blocking: uses the same HAL_GetTick timing idea as PetStats_Update
 * @param  now_ms   current time from HAL_GetTick()
 * @param  enabled  0 = force both alerts off (dead screen, menu, etc.)
 */
void PetStats_DrawAlerts(uint32_t now_ms, uint8_t enabled) {
  /* Flip the blink every ALERT_BLINK_MS */
  if (now_ms - last_blink_tick >= ALERT_BLINK_MS) {
    last_blink_tick = now_ms;
    blink_on ^= 1U;
  }

  uint8_t want_hunger = enabled && blink_on && PetStats_IsHungerLow();
  uint8_t want_happy  = enabled && blink_on && PetStats_IsHappinessLow();

  update_one_alert(HUNGER_ALERT_X, HUNGER_ALERT_Y, "HUNGRY!", &hunger_alert_shown, want_hunger);
  update_one_alert(HAPPY_ALERT_X,  HAPPY_ALERT_Y,  "SAD!",    &happy_alert_shown,  want_happy);
}
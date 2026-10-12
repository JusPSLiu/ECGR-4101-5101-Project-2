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

/* ---------------- Private state (only this file can touch these) ---------------- */
static uint8_t  hunger    = STAT_START;   /* 0 = starving, 100 = full            */
static uint8_t  happiness = STAT_START;   /* 0 = miserable, 100 = very happy     */

static uint32_t last_hunger_tick = 0;     /* time (ms) of the last hunger drop   */
static uint32_t last_happy_tick  = 0;     /* time (ms) of the last happy drop    */
static uint8_t  timers_started   = 0;     /* 0 until the first Update() call     */

/* ---------------- Placeholder meter layout ---------------- */
#define METER_W        100U   /* full bar width in pixels (1 pixel per stat point) */
#define METER_H        10U    /* bar height in pixels                              */
#define METER_GAP      22U    /* vertical distance between the two bars            */
#define LABEL_SIZE     1U     /* text scale for labels                             */
#define LABEL_W        48U    /* space reserved for "HUNGER"/"HAPPY" text          */

/**
 * @brief  Clamp a signed value into [STAT_MIN, STAT_MAX].
 *         Using int16_t lets us add a negative delta without wrapping around
 *         (uint8_t 5 - 10 would wrap to 251, which is the bug clamping prevents).
 */
static uint8_t clamp_stat(int16_t value) {
  if (value < (int16_t)STAT_MIN) return STAT_MIN;
  if (value > (int16_t)STAT_MAX) return STAT_MAX;
  return (uint8_t)value;
}

/* ---------------- Setup / reset ---------------- */

void PetStats_Init(void) {
  hunger         = STAT_START;   /* start both stats at the same healthy value */
  happiness      = STAT_START;
  timers_started = 0;            /* timers re-sync on the next Update()        */
}

void PetStats_Reset(void) {
  PetStats_Init();               /* restart = same as a fresh power-up         */
}

/* ---------------- Time-based decay ---------------- */

/**
 * @brief  Lower the stats as time passes. Non-blocking: it only compares the
 *         current time to the last drop time, so it returns immediately and
 *         never delays the menu or animations.
 * @param  now_ms  current time from HAL_GetTick()
 */
void PetStats_Update(uint32_t now_ms) {
  /* First call: start the timers from "now" so the stats don't
     drop instantly because last_*_tick was still 0. */
  if (!timers_started) {
    last_hunger_tick = now_ms;
    last_happy_tick  = now_ms;
    timers_started   = 1;
    return;
  }

  /* Unsigned subtraction handles HAL_GetTick() wrapping after ~49 days. */
  if (now_ms - last_hunger_tick >= HUNGER_DECAY_MS) {
    last_hunger_tick += HUNGER_DECAY_MS;   /* step forward by one period, keeps timing exact */
    hunger = clamp_stat((int16_t)hunger - (int16_t)HUNGER_DECAY_AMOUNT);
  }

  if (now_ms - last_happy_tick >= HAPPY_DECAY_MS) {
    last_happy_tick += HAPPY_DECAY_MS;
    happiness = clamp_stat((int16_t)happiness - (int16_t)HAPPY_DECAY_AMOUNT);
  }
}

/* ---------------- Actions ---------------- */

void PetStats_AddHunger(int16_t delta) {
  hunger = clamp_stat((int16_t)hunger + delta);        /* any change, always clamped */
}

void PetStats_AddHappiness(int16_t delta) {
  happiness = clamp_stat((int16_t)happiness + delta);
}

void PetStats_Feed(void) {
  PetStats_AddHunger((int16_t)FEED_AMOUNT);             /* feeding fills the pet up   */
}

void PetStats_Play(void) {
  PetStats_AddHappiness((int16_t)PLAY_AMOUNT);          /* playing cheers it up       */
}

/* ---------------- Read-only access ---------------- */

uint8_t PetStats_GetHunger(void)       { return hunger; }
uint8_t PetStats_GetHappiness(void)    { return happiness; }
uint8_t PetStats_IsHungerLow(void)     { return hunger    <= LOW_STAT_THRESHOLD; }
uint8_t PetStats_IsHappinessLow(void)  { return happiness <= LOW_STAT_THRESHOLD; }
uint8_t PetStats_IsStarving(void)      { return hunger    == STAT_MIN; }
uint8_t PetStats_IsMiserable(void)     { return happiness == STAT_MIN; }

/* ---------------- Placeholder meters ---------------- */

/**
 * @brief  Draw one labeled bar. Filled part = current value, rest = background.
 *         Red when the stat is low, green otherwise.
 */
static void draw_one_meter(uint16_t x, uint16_t y, const char *label, uint8_t value) {
  char num[6];                                           /* "100" + null, with room */
  uint16_t bar_x  = x + LABEL_W;                         /* bar starts after label  */
  uint16_t filled = (uint16_t)value * METER_W / STAT_MAX;/* width of the filled part*/
  uint16_t color  = (value <= LOW_STAT_THRESHOLD) ? ILI9341_COLOR_RED
                                                  : ILI9341_COLOR_GREEN;

  /* Label on the left */
  ILI9341_DrawString(x, y + 1, label, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE, LABEL_SIZE);

  /* Filled part, then the empty part (redrawing only this small region,
     not the whole screen, keeps it fast over SPI) */
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
  draw_one_meter(x, y,             "HUNGER", hunger);
  draw_one_meter(x, y + METER_GAP, "HAPPY",  happiness);
}
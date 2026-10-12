/**
 * @file    pet_stats.h
 * @brief   Hunger and happiness for the virtual pet.
 *
 * Owner: Marly (pet stats)
 *
 * How the stats work:
 *   - Both values range from STAT_MIN (0) to STAT_MAX (100)
 *   - 100 = full / very happy - 0 = starving / miserable
 *   - Both values drop by a small amount on a fixed timer (non-blocking,
 *     uses HAL_GetTick -so the menu and animations never freeze)
 *   - Actions (feed, play, ...) raise them by calling the functions below
 *   - Every change is clamped - so values never go below 0 or above 100
 *
 * How others use this file:
 *   - Call PetStats_Init() once before the main loop
 *   - Call PetStats_Update(HAL_GetTick()) once every pass of the main loop
 *   - Call PetStats_Feed() / PetStats_Play() from your action code
 *   - Read the values with PetStats_GetHunger() / PetStats_GetHappiness()
 *   - Death/runaway code can check PetStats_IsStarving() / PetStats_IsMiserable().
 */

#ifndef PET_STATS_H
#define PET_STATS_H

#include <stdint.h>

/* Documented limits  */
#define STAT_MIN              0U     /* lowest allowed value                         */
#define STAT_MAX              100U   /* highest allowed value                        */
#define STAT_START            80U    /* value both stats start at after Init/Reset   */

/* Decay over time*/
#define HUNGER_DECAY_MS       3000U  /* hunger drops every 3 seconds                 */
#define HUNGER_DECAY_AMOUNT   1U     /* ...by this many points                       */
#define HAPPY_DECAY_MS        4000U  /* happiness drops every 4 seconds              */
#define HAPPY_DECAY_AMOUNT    1U     /* ...by this many points                       */

/* Action effects  */
#define FEED_AMOUNT           20U    /* feeding raises hunger by this much           */
#define PLAY_AMOUNT           15U    /* playing raises happiness by this much        */

/* Thresholds */
#define LOW_STAT_THRESHOLD    25U    /* at or below this, the stat counts as "low"   */

/* Setup / reset */
void    PetStats_Init(void);                   /* set both stats to STAT_START        */
void    PetStats_Reset(void);                  /* same as Init, for the restart code  */

/* Call once per main loop pass - handles the time-based decay */
void    PetStats_Update(uint32_t now_ms);

/* Actions that change the stats */
void    PetStats_Feed(void);                   /* hunger    += FEED_AMOUNT            */
void    PetStats_Play(void);                   /* happiness += PLAY_AMOUNT            */
void    PetStats_AddHunger(int16_t delta);     /* general: any +/- change to hunger   */
void    PetStats_AddHappiness(int16_t delta);  /* general: any +/- change to happiness*/

/* Read-only access */
uint8_t PetStats_GetHunger(void);
uint8_t PetStats_GetHappiness(void);
uint8_t PetStats_IsHungerLow(void);            /* 1 if hunger    <= LOW_STAT_THRESHOLD */
uint8_t PetStats_IsHappinessLow(void);         /* 1 if happiness <= LOW_STAT_THRESHOLD */
uint8_t PetStats_IsStarving(void);             /* 1 if hunger    == STAT_MIN (death)   */
uint8_t PetStats_IsMiserable(void);            /* 1 if happiness == STAT_MIN (runaway) */

/* Placeholder meters - draws two bars + numbers at the given top-left corner */
void    PetStats_DrawMeters(uint16_t x, uint16_t y);

/* Low-stat alerts: blinking "HUNGRY!" / "SAD!" text next to the status bars */
#define ALERT_BLINK_MS        500U   /* alert text turns on/off every half second    */
#define HUNGER_ALERT_X        50U    /* lines up with the left edge of the hunger bar */
#define HUNGER_ALERT_Y        36U    /* just under the hunger bar - above the pet      */
#define HAPPY_ALERT_X         50U    /* lines up with the left edge of the happy bar  */
#define HAPPY_ALERT_Y         270U   /* just above the happiness bar -  below the pet   */
#define ALERT_TEXT_SIZE       2U     /* 2x the 5x7 font = 10x14 pixels per letter     */

/* Call every pass while the main pet screen is showing
   enabled = 0 hides both alerts (e.g. on the dead screen) */
void    PetStats_DrawAlerts(uint32_t now_ms, uint8_t enabled);

#endif /* PET_STATS_H */
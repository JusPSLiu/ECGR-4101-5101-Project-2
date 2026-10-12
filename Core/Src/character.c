#include "character.h"
#include "pet_stats.h"   /* hunger/happiness values, decay timer, clamping */

uint8_t is_dead;

/* Last values seen by updateStats() -so the animation state is only
   re-checked when a stat actually changes (not on every loop pass). */
static uint8_t last_hunger    = STAT_MAX + 1;   /* impossible value forces the first check */
static uint8_t last_happiness = STAT_MAX + 1;

enum states {
    state_idle
};

void characterLogic() {
    // TODO: INSERT STATE MACHINE
    drawSprite();
}

/**
 * @brief  Advance hunger/happiness with real elapsed time
 *         Call once every main loop pass with HAL_GetTick().
 *          timing itself lives in pet_stats.c - this just reacts
 *         when a value changes so the right loop animation plays
 */
void updateStats(uint32_t now) {
    if (is_dead) return;                       /* stats freeze once the pet is dead */

    PetStats_Update(now);                      /* time-based decay + clamping */

    uint8_t hunger    = PetStats_GetHunger();
    uint8_t happiness = PetStats_GetHappiness();

    if (hunger != last_hunger || happiness != last_happiness) {
        last_hunger    = hunger;
        last_happiness = happiness;
        updateAnimState();                     /* pick idle / low battery / sad / dead */
    }
}

void feed() {
    PetStats_Feed();                           /* hunger += FEED_AMOUNT, clamped */
    updateAnimState();
    triggerAnim(anim_eat);
}
void play() {
    PetStats_Play();                           /* happiness += PLAY_AMOUNT, clamped */
    updateAnimState();
    triggerAnim(anim_backflip);
}

// im hardcoding this hierarchy of priority for the underlying loop animation.
// this is a small enough project.
void updateAnimState() {
    if (PetStats_IsStarving() || PetStats_IsMiserable()) {
        is_dead = 1;
        PetStats_AddHunger(-(int16_t)STAT_MAX);     /* drop both meters to 0 on death */
        PetStats_AddHappiness(-(int16_t)STAT_MAX);
        triggerAnim(anim_die);
    }

    if (is_dead) {
        setLoop(anim_dead_loop);
    } else if (PetStats_GetHunger() < 25) {
        setLoop(anim_lowbatt_loop);
    } else if (PetStats_GetHappiness() < 50) {
        setLoop(anim_sad_loop);
    } else {
        setLoop(anim_idle_loop);
    }
}

void updateHud() {
    updateHudDisplay(PetStats_GetHunger(), PetStats_GetHappiness());
        PetStats_DrawAlerts(HAL_GetTick(), !is_dead);   /* blink low-stat alerts - hidden once dead */
}

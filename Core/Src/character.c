#include "character.h"

#define UPDATE_COOLDOWN 10

uint8_t happ_update_counter = UPDATE_COOLDOWN;
uint8_t batt_update_counter = UPDATE_COOLDOWN;

enum states {
    state_idle
};

void characterLogic() {
    // TODO: INSERT STATE MACHINE
}

void updateHappy() {
    happ_update_counter--;
    if (happ_update_counter < 0) {
        happy_percent--;
        happ_update_counter = UPDATE_COOLDOWN;

        updateAnimState();
    }
}
void updateBattery() {
    batt_update_counter--;
    if (batt_update_counter < 0) {
        battery_percent--;
        batt_update_counter = UPDATE_COOLDOWN;

        updateAnimState();
    }
}

void feed() {
    battery_percent = 100;
    updateAnimState();
    triggerAnim(anim_eat);
}
void play() {
    happy_percent = 100;
    updateAnimState();
    triggerAnim(anim_backflip);
}

// im hardcoding this hierarchy of priority for the underlying loop animation.
// this is a small enough project.
void updateAnimState() {
    if (is_dead) {
        setLoop(anim_dead_loop);
    } else if (battery_percent < 25) {
        setLoop(anim_lowbatt_loop);
    } else if (happy_percent < 25) {
        setLoop(anim_sad_loop);
    } else {
        setLoop(anim_idle_loop);
    }
}




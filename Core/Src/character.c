#include "character.h"

#define UPDATE_COOLDOWN 10

uint8_t battery_percent = 100;
uint8_t happy_percent = 100;
uint8_t is_dead;

uint8_t happ_update_counter = UPDATE_COOLDOWN;
uint8_t batt_update_counter = UPDATE_COOLDOWN;
static uint8_t is_dancing;

enum states {
    state_idle
};

void characterLogic() {
    // TODO: INSERT STATE MACHINE
    drawSprite();
}

void updateHappy() {
    if (is_dead) return;
    
    happ_update_counter--;
    if (happ_update_counter == 0) {
        happy_percent--;
        happ_update_counter = UPDATE_COOLDOWN;

        updateAnimState();
    }
}
void updateBattery() {
    if (is_dead) return;
    
    batt_update_counter--;
    if (batt_update_counter == 0) {
        battery_percent--;
        batt_update_counter = UPDATE_COOLDOWN;

        updateAnimState();
    }
}

void feed() {
    is_dancing = 0;
    battery_percent = 100;
    updateAnimState();
    triggerAnim(anim_eat);
}
void play() {
    is_dancing = 0;
    happy_percent = 100;
    updateAnimState();
    triggerAnim(anim_backflip);
}
void dance() {
    if (is_dead) return;

    is_dancing = 1;
    setLoop(anim_dance);
}

// im hardcoding this hierarchy of priority for the underlying loop animation.
// this is a small enough project.
void updateAnimState() {
    if (battery_percent == 0 || happy_percent == 0) {
        is_dead = 1;
        battery_percent = 0; happy_percent = 0;
        triggerAnim(anim_die);
    }

    if (is_dead) {
        is_dancing = 0;
        setLoop(anim_dead_loop);
    } else if (is_dancing) {
        setLoop(anim_dance);
    } else if (battery_percent < 25) {
        setLoop(anim_lowbatt_loop);
    } else if (happy_percent < 50) {
        setLoop(anim_sad_loop);
    } else {
        setLoop(anim_idle_loop);
    }
}

void updateHud() {
    updateHudDisplay(battery_percent, happy_percent);
}
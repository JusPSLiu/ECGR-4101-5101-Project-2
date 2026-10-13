/**
 * sounds.c
 *
 * Non-blocking melody player using a hardware timer PWM output.
 * See sound.h for details.
 */

#include "sounds.h"
#include <stdint.h>

/* ---------- Nokia tune data ---------- */
#define NOKIA_LENGTH 13

static const uint16_t nokia_freq[NOKIA_LENGTH] = {
    1319, 1175, 740, 831,  // E6, D6, F#5, G#5
    1109, 988,  587, 659,  // C#6, B5, D5, E5
    988,  880,  554, 659,  // B5, A5, C#5, E5
    880                    // A5
};

static const uint16_t nokia_dur_ms[NOKIA_LENGTH] = {
    100, 100, 200, 200,    // Eighth, Eighth, Quarter, Quarter
    100, 100, 200, 200,    // Eighth, Eighth, Quarter, Quarter
    100, 100, 200, 200,    // Eighth, Eighth, Quarter, Quarter
    400                    // Half Note
};

#define WILHELM_LENGTH 29

static const uint16_t wilhelm_freq[] = {
    // Phase 1: The Inception (Rapid rise)
    400, 550, 750, 950, 1100, 
    // Phase 2: The Peak (High pitch stabilization)
    1150, 1200, 1250, 1300, 1280, 
    // Phase 3: The Plunge (The famous descending arc)
    1250, 1150, 1050, 950, 850, 750, 650, 550, 480, 420, 380, 350, 
    // Phase 4: The Guttural Fade (Low, raspy drop-off)
    320, 290, 260, 230, 200, 170, 150
};

static const uint16_t wilhelm_dur_ms[] = {
    // Phase 1 timings
    24, 24, 24, 24, 24, 
    // Phase 2 timings
    36, 36, 36, 36, 36, 
    // Phase 3 timings
    45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 45, 45, 
    // Phase 4 timings
    57, 57, 57, 57, 57, 57, 58
};

#define ALERT_LENGTH 6

static const uint16_t alert_freq[ALERT_LENGTH] = {
    880, 440, 880, 440, 880, 440
};

static const uint16_t alert_dur_ms[ALERT_LENGTH] = {
    250, 250, 250, 250, 250, 250 // Piercing 1.5-second cycle
};

// #define SUCCESS_LENGTH 4

// static const uint16_t success_freq[SUCCESS_LENGTH] = {
//     523, 659, 784, 1047 // C5, E5, G5, C6
// };

// static const uint16_t success_dur_ms[SUCCESS_LENGTH] = {
//     75, 75, 75, 250 // Quick flourish leading to a sustained final note
// };

#define LASER_LENGTH 10

static const uint16_t laser_freq[LASER_LENGTH] = {
    2500, 2100, 1700, 1400, 1100, 900, 750, 600, 450, 300
};

static const uint16_t laser_dur_ms[LASER_LENGTH] = {
    15, 15, 15, 15, 20, 20, 20, 25, 25, 30 // Total 200ms quick burst
};

#define SUCCESS_LENGTH 12

static const uint16_t success_freq[SUCCESS_LENGTH] = {
    523, 659, 784, 1047,   // C5, E5, G5, C6
    784, 880, 1047, 1175,  // G5, A5, C6, D6
    1047, 1175, 1319, 1568 // C6, D6, E6, G6
};

static const uint16_t success_dur_ms[SUCCESS_LENGTH] = {
    90, 90, 90, 180,
    80, 80, 140, 140,
    100, 100, 120, 300
};

#define POWERUP_LENGTH 9

static const uint16_t powerup_freq[POWERUP_LENGTH] = {
    392, 494, 587, 784, 988,
    1175, 1397, 1568, 1397
};

static const uint16_t powerup_dur_ms[POWERUP_LENGTH] = {
    45, 45, 45, 60, 60,
    60, 80, 100, 70
};

#define NOTE_LENGTH 32
 
const uint16_t note_freq[NOTE_LENGTH] = {
   880,  831,  784,  740,  880,  698,  740,  698,
   880,  659,  698,  659,  880,  622,  659,  622,
   880,  587,  622,  587,  880,  554,  587,  554,
   880,  523,  554,  523,  880,  494,  523,  494
};
 
const uint16_t note_dur_ms[NOTE_LENGTH] = {
    88,   88,   88,   88,   88,   88,   88,   88,
    88,   88,   88,   88,   88,   88,   88,   88,
    88,   88,   88,   88,   88,   88,   88,   88,
    88,   88,   88,   88,   88,   88,   88,   88
};

#define TEST_LENGTH 1

const uint16_t test_freq[] = {440};
const uint16_t test_dur_ms[]   = {1000};

/* ------------------------------------------------------------------------ */
/* State                                                                    */
/* ------------------------------------------------------------------------ */
static TIM_HandleTypeDef s_htim;
static uint32_t          s_timCounterClk;   /* timer counter clock, Hz       */
 
static const uint16_t   *s_frequencies;     /* current tune: Hz  (0 = rest)  */
static const uint16_t   *s_durations_ms;    /* current tune: ms              */
static uint16_t          s_numNotes;        /* entries in each array         */
 
static uint16_t          s_noteIndex;       /* index into BOTH arrays        */
static bool              s_active;          /* true while tune in progress   */
static uint32_t          s_noteStartMs;     /* HAL tick when note began      */
static uint32_t          s_noteDurMs;       /* current note length, tempo-scaled */
static uint16_t          s_tempoPercent = 100U; /* 100 = normal speed        */
 
/* ------------------------------------------------------------------------ */
/* Helpers                                                                  */
/* ------------------------------------------------------------------------ */
 
/* Timer kernel clock: PCLKx, doubled when the APB prescaler is not 1. */
static uint32_t get_timer_clock(void)
{
#if SOUND_TIM_IS_APB2
    uint32_t pclk = HAL_RCC_GetPCLK2Freq();
    return ((RCC->CFGR & RCC_CFGR_PPRE2) == RCC_CFGR_PPRE2_DIV1) ? pclk : (pclk * 2U);
#else
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();
    return ((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_CFGR_PPRE1_DIV1) ? pclk : (pclk * 2U);
#endif
}
 
/* Silence the output: PWM mode 1 with CCR = 0 holds the pin low. */
static void tone_off(void)
{
    __HAL_TIM_SET_COMPARE(&s_htim, SOUND_TIM_CHANNEL, 0U);
}
 
/* Set the PWM frequency (50 % duty). freq == 0 -> silence. */
static void tone_set(uint16_t freq)
{
    if (freq == 0U) {
        tone_off();
        return;
    }
 
    uint32_t arr = (s_timCounterClk / (uint32_t)freq);
    arr = (arr > 0U) ? (arr - 1U) : 0U;
    if (arr > SOUND_TIM_MAX_ARR) {
        arr = SOUND_TIM_MAX_ARR;
    }
    if (arr < 1U) {
        arr = 1U;
    }
 
    __HAL_TIM_SET_AUTORELOAD(&s_htim, arr);
    __HAL_TIM_SET_COMPARE(&s_htim, SOUND_TIM_CHANNEL, (arr + 1U) / 2U);
    __HAL_TIM_SET_COUNTER(&s_htim, 0U);
    /* Force an update event so the new ARR/CCR (preloaded) take effect now
     * instead of after the previous note's (possibly very long) period. */
    s_htim.Instance->EGR = TIM_EGR_UG;
}
 
/* Start note `index`: set its pitch and capture its tempo-scaled length.
 * Capturing the length here means a tempo change applies from the next note
 * on and never disturbs the note currently playing. */
static void start_note(uint16_t index)
{
    uint32_t dur = ((uint32_t)s_durations_ms[index] * 100U) / s_tempoPercent;
    if (dur == 0U) {
        dur = 1U;
    }
    s_noteDurMs = dur;
    tone_set(s_frequencies[index]);
}
 
/* Index back to 0, output silent, ready for the next Play_Tune(). */
static void reset_playback(void)
{
    s_active    = false;
    s_noteIndex = 0U;
    tone_off();
}
 
/* ------------------------------------------------------------------------ */
/* Public API                                                               */
/* ------------------------------------------------------------------------ */
HAL_StatusTypeDef Sound_Init(void)
{
    GPIO_InitTypeDef       gpio = {0};
    TIM_OC_InitTypeDef     oc   = {0};
 
    /* Pin -> timer alternate function */
    SOUND_GPIO_CLK_ENABLE();
    gpio.Pin       = SOUND_GPIO_PIN;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = SOUND_TIM_AF;
    HAL_GPIO_Init(SOUND_GPIO_PORT, &gpio);
 
    /* Timer base */
    SOUND_TIM_CLK_ENABLE();
    s_timCounterClk = get_timer_clock() / (SOUND_TIM_PRESCALER + 1U);
 
    s_htim.Instance               = SOUND_TIM_INSTANCE;
    s_htim.Init.Prescaler         = SOUND_TIM_PRESCALER;
    s_htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    s_htim.Init.Period            = 1000U;                        /* placeholder */
    s_htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    s_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&s_htim) != HAL_OK) {
        return HAL_ERROR;
    }
 
    /* PWM channel, starts silent (CCR = 0) */
    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = 0U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&s_htim, &oc, SOUND_TIM_CHANNEL) != HAL_OK) {
        return HAL_ERROR;
    }
 
    if (HAL_TIM_PWM_Start(&s_htim, SOUND_TIM_CHANNEL) != HAL_OK) {
        return HAL_ERROR;
    }
 
    s_frequencies  = NULL;
    s_durations_ms = NULL;
    s_numNotes     = 0U;
    reset_playback();
    return HAL_OK;
}
 
bool Play_Tune(const uint16_t *frequencies,
               const uint16_t *durations_ms,
               uint16_t        length)
{
    if ((frequencies == NULL) || (durations_ms == NULL) || (length == 0U)) {
        return false;
    }
 
    /* Load the new tune (replaces any tune already playing). */
    s_frequencies  = frequencies;
    s_durations_ms = durations_ms;
    s_numNotes     = length;
 
    /* Start at the first note. */
    s_noteIndex   = 0U;
    s_active      = true;
    s_noteStartMs = HAL_GetTick();
    start_note(0U);
 
    return true;
}
 
bool Sound_Play(void)
{
    if (!s_active) {
        return false;
    }
 
    uint32_t nowMs = HAL_GetTick();
 
    if ((uint32_t)(nowMs - s_noteStartMs) >= s_noteDurMs) {
        /* Current note's duration has elapsed -> advance. */
        s_noteStartMs += s_noteDurMs;                   /* no cumulative drift */
        s_noteIndex++;
 
        if (s_noteIndex >= s_numNotes) {
            reset_playback();       /* index back to 0, silent */
            return false;
        }
        start_note(s_noteIndex);
    }
 
    return true;
}
 
void Sound_SetTempo(uint16_t percent)
{
    if (percent < SOUND_TEMPO_MIN) {
        percent = SOUND_TEMPO_MIN;
    } else if (percent > SOUND_TEMPO_MAX) {
        percent = SOUND_TEMPO_MAX;
    }
    s_tempoPercent = percent;
}
 
void Sound_Stop(void)
{
    reset_playback();
}
 
bool Sound_IsPlaying(void)
{
    return s_active;
}

void PlayNokiaRingtone() {
    Sound_SetTempo(75);
    Play_Tune(nokia_freq, nokia_dur_ms, NOKIA_LENGTH); 
}

#define MELODY_LENGTH 16

static const uint16_t melody_freq[MELODY_LENGTH] = {
    440, 523, 659, 523,
    392, 494, 587, 494,
    349, 440, 523, 440,
    392, 349, 330, 294
};

static const uint16_t melody_dur_ms[MELODY_LENGTH] = {
    220, 180, 260, 180,
    220, 180, 300, 180,
    220, 180, 260, 180,
    220, 220, 300, 450
}; // Total 3750ms

#define TRAINER_LENGTH 16

static const uint16_t trainer_freq[TRAINER_LENGTH] = {
    440, 587, 784, 659,
    523, 698, 880, 784,
    622, 831, 1047, 932,
    784, 1047, 1319, 1568
};

static const uint16_t trainer_dur_ms[TRAINER_LENGTH] = {
    70, 70, 100, 70,
    70, 70, 100, 70,
    70, 70, 100, 70,
    100, 100, 140, 250
}; // Total 1420ms

#define CATCH_LENGTH 12

static const uint16_t catch_freq[CATCH_LENGTH] = {
    523, 659, 784, 1047, 988, 1047,
    1319, 1175, 1319, 1568, 1760, 2093
};

static const uint16_t catch_dur_ms[CATCH_LENGTH] = {
    80, 80, 100, 140, 80, 100,
    100, 80, 100, 140, 160, 300
}; // Total 1460ms

#define HYPE_LENGTH 32

static const uint16_t hype_freq[HYPE_LENGTH] = {
    880, 1047, 1319, 1568, 1319, 1047, 1568, 1760,
    1568, 1319, 1047, 1319, 1760, 2093, 1760, 1568,
    1319, 1568, 2093, 2349, 2093, 1760, 1568, 1760,
    2093, 2637, 2349, 2093, 1760, 2093, 2637, 3136
};

static const uint16_t hype_dur_ms[HYPE_LENGTH] = {
    60, 60, 60, 100, 60, 60, 60, 100,
    60, 60, 60, 100, 60, 60, 60, 100,
    60, 60, 60, 100, 60, 60, 60, 100,
    60, 60, 60, 100, 60, 60, 100, 180
}; // Total 2200ms


#define POKE_BATTLE_THEME_LENGTH 84

static const uint16_t poke_battle_theme_freqs[POKE_BATTLE_THEME_LENGTH] = {
    523, 493, 466, 440, 466, 440, 415, 391, 415, 391, 369, 349, 369, 349, 329, 311, 329, 311, 293, 277, 293, 277, 261, 246, 261, 246, 233, 220, 233, 246, 261, 277, 391, 329, 391, 311, 391, 277, 391, 329, 391, 311, 391, 277, 369, 391, 329, 391, 311, 391, 277, 391, 329, 391, 311, 391, 277, 369, 391, 369, 329, 391, 440, 391, 830, 783, 830, 783, 1108, 523, 466, 415, 554, 523, 466, 698, 659, 587, 466, 523, 587, 698, 830, 783
};

static const uint16_t poke_battle_theme_durations[POKE_BATTLE_THEME_LENGTH] = {
    82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 165, 165, 331, 165, 331, 165, 331, 165, 331, 165, 331, 662, 662, 331, 165, 331, 165, 331, 165, 331, 165, 331, 165, 331, 662, 662, 497, 497, 331, 497, 497, 331, 994, 165, 165, 165, 828, 497, 497, 331, 497, 497, 331, 497, 497, 331, 331, 331, 331, 331, 2651, 2651
};

#define POKE_HEAL_LENGTH 10

static const uint16_t poke_heal_freqs[POKE_HEAL_LENGTH] = {
    493.88, 164.81, 493.88, 164.81, 493.88, 164.81, 415.3, 207.65, 659.26, 164.81
};

static const uint16_t poke_heal_durations[POKE_HEAL_LENGTH] = {
    483, 483, 483, 483, 241, 241, 241, 241, 483, 483
};

// #define MELODY_LENGTH 40

// static const uint16_t melody_freqs[40] = {
//     246.94, 392.0, 369.99, 82.41, 246.94, 261.63, 369.99, 369.99, 329.63, 65.41, 246.94, 392.0, 369.99, 82.41, 246.94, 261.63, 369.99, 329.63, 261.63, 65.41, 246.94, 392.0, 369.99, 82.41, 246.94, 261.63, 261.63, 329.63, 261.63, 65.41, 246.94, 392.0, 369.99, 82.41, 246.94, 261.63, 369.99, 329.63, 261.63, 65.41
// };

// static const uint16_t melody_durations[40] = {
//     375, 375, 375, 1125, 375, 375, 375, 375, 375, 1875, 375, 375, 375, 1125, 375, 375, 375, 375, 375, 1875, 375, 375, 375, 1125, 375, 375, 375, 375, 375, 1875, 375, 375, 375, 1125, 375, 375, 375, 375, 375, 1875
// };

// Total number of notes and rests in this sequence
#define MARIO_THEME_LENGTH 41

// Frequencies in Hz (0 represents a musical rest)
static const uint16_t mario_frequencies[MARIO_THEME_LENGTH] = {
    // Intro
    659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392, 0,
    // Main Melody Phrase
    523, 0, 0, 392, 0, 0, 330, 0, 0, 440, 0, 494, 0, 
    466, 440, 0, 392, 659, 784, 880, 0, 698, 784, 0, 
    659, 0, 523, 587, 494
};

// Durations in milliseconds (corresponds directly to the frequencies above)
static const uint16_t mario_durations[MARIO_THEME_LENGTH] = {
    // Intro durations
    150, 150, 150, 150, 150, 150, 150, 150, 150, 450, 150, 300,
    // Main Melody Phrase durations
    150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 
    150, 150, 150, 113, 113, 113, 150, 150, 150, 150, 150, 150, 150, 
    150, 150, 300
};


#define WAKA_LENGTH 16
// Frequencies in Hz for the "Waka Waka" chattering sound
static const uint16_t WAKA_FREQUENCIES[] = {
    440, 880, 440, 880, 440, 880, 440, 880,
    440, 880, 440, 880, 440, 880, 440, 880
};

// Durations in milliseconds for each alternation
static const uint16_t WAKA_DURATIONS[] = {
    60,  60,  60,  60,  60,  60,  60,  60,
    60,  60,  60,  60,  60,  60,  60,  60
};


#define PACMAN_THEME_LENGTH 29
// Frequencies in Hz for the Pac-Man Intro Theme
const uint16_t PACMAN_THEME_FREQUENCIES[] = {
    494, 988, 740, 622, 988, 740, 622,  // B4, B5, F#5, D#5, B5, F#5, D#5
    523, 1047, 784, 659, 1047, 784, 659, // C5, C6, G5, E5, C6, G5, E5
    494, 988, 740, 622, 988, 740, 622,  // B4, B5, F#5, D#5, B5, F#5, D#5
    622, 659, 698, 740, 784, 831, 880, 988 // D#5, E5, F5, F#5, G5, G#5, A5, B5
};

// Durations in milliseconds for each note
const uint16_t PACMAN_THEME_DURATIONS[] = {
    115, 115, 115, 115, 115, 115, 230,
    115, 115, 115, 115, 115, 115, 230,
    115, 115, 115, 115, 115, 115, 230,
    57,  57,  57,  57,  57,  57,  57,  115
};

// The total number of elements in the sound array
#define EAT_SOUND_LENGTH 21

// Frequencies in Hz (0 represents a pause/silence)
const uint16_t eat_frequencies[EAT_SOUND_LENGTH] = {
    201, 178, 156, 134, 112,  90,   0,  // Chomp 1
    164, 144, 125, 105,  86,  66,   0,  // Chomp 2
    210, 185, 160, 135, 110,  85,   0   // Chomp 3
};

// Note durations in milliseconds
const uint16_t eat_durations[EAT_SOUND_LENGTH] = {
     30,  30,  30,  30,  30,  30,  40,  // Chomp 1
     30,  30,  30,  30,  30,  30,  40,  // Chomp 2
     30,  30,  30,  30,  30,  30,  40   // Chomp 3
};

#define REJUVENATED_LENGTH 20

const uint16_t REJUVENATED_MELODY_FREQ[] = {
    262, 330, 392, 523, 0,   // C4, E4, G4, C5 arpeggio + short rest
    392, 523, 659, 784, 0,   // G4, C5, E5, G5 arpeggio + short rest
    523, 587, 659, 698,      // C5, D5, E5, F5 fast climb
    784, 0,   880, 0,        // G5 + rest, A5 + rest
    988, 1047                // B5, C6 triumphant peak
};

// Duration of notes in milliseconds
const uint16_t REJUVENATED_MELODY_DUR[] = {
    75,  75,  75,  150, 50,  // Fast introductory arpeggio
    75,  75,  75,  150, 50,  // Higher octave burst
    100, 100, 100, 100,      // Steady driving climb
    150, 50,  150, 50,       // Punchy syncopated rhythm
    200, 500                 // Final sustained resolution
};

#define VON_LENGTH 16
static const uint16_t VON_FREQ[VON_LENGTH] = {
    // Phrase 1
    494,  // B4
    784,  // G5
    740,  // F#5
    494,  // B4
    523,  // C5
    659,  // E5
    659,  // E5
    523,  // C5

    // Phrase 2
    494,  // B4
    784,  // G5
    740,  // F#5
    494,  // B4
    523,  // C5
    740,  // F#5
    659,  // E5
    523   // C5
};

// Durations in milliseconds at ~150 BPM (Quarter note = 400ms, Eighth note = 200ms)
static const uint16_t VON_DUR[VON_LENGTH] = {
    // Phrase 1
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note

    // Phrase 2
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200,  // Eighth note
    200   // Eighth note
};

// Total notes in the sound effect sequence
#define MGS_ALERT_NOTE_COUNT 6

// Frequencies in Hz for each step of the arpeggio/punch
static const uint16_t MGS_ALERT_FREQ[MGS_ALERT_NOTE_COUNT] = {
    554,  // C#5 - Rapid ascending arpeggio start
    659,  // E5
    784,  // G5
    932,  // Bb5
    1109, // C#6 - Highest sharp exclamation note (Staccato accent)
    1109  // C#6 - Quick decay/release note
};

// Durations in milliseconds for each note
static const uint16_t MGS_ALERT_DURATION[MGS_ALERT_NOTE_COUNT] = {
    35,   // Extremely rapid slide up
    35,   // Rapid slide up
    35,   // Rapid slide up
    35,   // Rapid slide up
    200,  // Main piercing exclamation note accent
    100   // Quick fade/release
};

// Number of notes/rests in the sequence
#define ENTERTAINER_LENGTH 45

// Frequencies in Hz (0 indicates a rest/silence)
const uint16_t entertainerFrequencies[] = {
    988, 1047, 1109, 1760, 1109, 1760, 1109, 1760, 0, 1976, 
    2093, 2217, 1760, 1976, 2217, 1661, 1976, 1760, 0, 988, 
    1047, 1109, 1760, 1109, 1760, 1109, 1760, 0, 1480, 1661, 
    1760, 1976, 2217, 1760, 1976, 2217, 0, 988, 1047, 1109, 
    1760, 1109, 1760, 1109, 1760
};

// Durations in milliseconds
const uint16_t entertainerDurations[] = {
    130, 130, 130, 260, 130, 260, 130, 520, 130, 130, 
    130, 130, 130, 130, 260, 130, 260, 520, 260, 130, 
    130, 130, 260, 130, 260, 130, 520, 130, 130, 130, 
    130, 130, 260, 130, 260, 520, 130, 130, 130, 130, 
    260, 130, 260, 130, 520
};

void PlayPokemonSuccess() {
    Sound_SetTempo(50);
    Play_Tune(entertainerFrequencies, entertainerDurations, ENTERTAINER_LENGTH); 
}

// Notes: Sound_Play() returns true or false to play again
// to play a sound again ex: PlayNokiaRingtone(), just call it again and leave Sound_Play in while(1)
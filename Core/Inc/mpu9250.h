#ifndef MPU9250_H
#define MPU9250_H

#include <stdint.h>
#include <math.h>
#include "main.h"

/* Configuration Defines */
#define THRESH 0.1f    // g
#define WINDOW_MS 800
#define REVERSALS_NEEDED 4
#define GRAVITY_ALPHA 0.9f
#define ALPHA 0.9

/* Function Prototypes */
void MPU9250_Power_Init();
int detectSingleAxisShake(float delta);
float calculateShakeDelta(int16_t axis_data);
float getDelta(int16_t val);

#endif /* MPU_9250_H */

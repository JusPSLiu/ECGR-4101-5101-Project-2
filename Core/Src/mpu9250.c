
#include "mpu9250.h"
#include <iso646.h>


// float grav = 0;

static int lastSign = 0;
static int reversals = 0;
static unsigned long windowStart = 0;
static float gravity = 0.0f;  

BackFlipState_t backFlipState = NOT_FLIPPING;

int detectSingleAxisShake(float delta) {
  unsigned long now = HAL_GetTick();

  if (now - windowStart > WINDOW_MS) { reversals = 0; lastSign = 0; windowStart = now; }

  int sign = 0;
  
  if (delta >  THRESH) sign =  1;
  if (delta < -THRESH) sign = -1;

  if (sign != 0 && sign != lastSign) {
    if (lastSign != 0) reversals++;      // only count flips, not the first crossing
    lastSign = sign;
  }

  if (reversals >= REVERSALS_NEEDED) { reversals = 0; lastSign = 0; windowStart = now; return 1; }
  return 0;
}

float getDelta(float axisData) {
  float axisDataG = axisData / G;
// ay_g = raw Y accel in g (raw_ay / 16384.0)
  gravity = ALPHA * gravity + (1.0 - ALPHA) * axisDataG;
  float deltaData = axisDataG - gravity;             // motion on Y only, gravity removed

  return deltaData;
}

// uses the port as 3.3v vcc
void MPU9250_Power_Init() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  HAL_GPIO_WritePin(MPU9250_PWR_Port, MPU9250_PWR_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin   = MPU9250_PWR_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;        // or OUTPUT_OD with an external pull-up
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(MPU9250_PWR_Port, &GPIO_InitStruct);
}

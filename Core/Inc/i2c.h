#ifndef I2C_H
#define I2C_H

#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "ili9341.h"


// I2C1 SDA PG13
#define I2C1_SDA_Pin GPIO_PIN_13

// I2C1 SCL PG14
#define I2C1_SCL_Pin GPIO_PIN_14

// I2C1 Port -> PORTG
#define I2C_GPIO_Port GPIOG

#define MPU9250_DEVICE_ADDR     0x68
#define MPU9250_1ST_ADDR_BYTE   0x3B
#define MPU9250_PWR_MGMT_1_REG  0x6B
#define MPU9250_WAKE_UP_VAL     0x00

#define I2C_SCAN_FIRST_ADDR     0x08   /* 0x00-0x07 are reserved */
#define I2C_SCAN_LAST_ADDR      0x77   /* 0x78-0x7F are reserved */

typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;
} MPU9250AccelerometerData_t;

void MX_I2C1_Init(void);
uint8_t I2C_Scan(uint8_t *found, uint8_t max_found);
MPU9250AccelerometerData_t readMPU9250();
HAL_StatusTypeDef MPU9250_WakeUp(void);

void readMPU9250Test();

#endif
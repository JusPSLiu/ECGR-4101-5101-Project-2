#include "i2c.h"
#include "ili9341.h"
#include "stm32l4xx_hal_def.h"


I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00100D14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
    ILI9341_FillScreen(ILI9341_COLOR_GREEN);
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
 * Scan an I2C bus and store the 7-bit addresses that ACK.
 *
 * @param hi2c      Initialized I2C handle (e.g. &hi2c1)
 * @param found     Caller-provided buffer for 7-bit addresses
 * @param max_found Size of the buffer (112 is enough to hold every possible address)
 * @return          Number of devices found (entries written to `found`)
 */

// char str[20];
volatile uint8_t last_nack_addr;

// uint8_t I2C_Scan(I2C_HandleTypeDef *hi2c, uint8_t *found, uint8_t max_found)
// {
//     uint8_t count = 0;

//     for (uint8_t addr = 0x08; addr <= 0x77; addr++)
//     {
//         HAL_StatusTypeDef st =
//             HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(addr << 1), 1, 10);

//         if (st == HAL_OK)
//         {
//             if (count < max_found) { found[count] = addr; }
//             // snprintf(str, sizeof(str), "success: 0x%02X", addr);
        

//             // ILI9341_DrawString(0 , 7 * (count * 2) + (5 * count), str, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
//             count++;
//         }
//         else if ((hi2c->ErrorCode & ~(HAL_I2C_ERROR_AF | HAL_I2C_ERROR_TIMEOUT))
//                  || __HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY))
//         {
//             /* Real bus problem (BERR/ARLO/OVR, or BUSY stuck): reset peripheral */
//             __HAL_I2C_DISABLE(hi2c);
//             __HAL_I2C_ENABLE(hi2c);

//   //           snprintf(str, sizeof(str), "failed: 0x%02X", addr);
        

//   // ILI9341_DrawString(0 , 7 * (0 * 2) + (5 * 0), str, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
//         }
//         else {
//           last_nack_addr = addr;
          
//         }
//     }

//     return (count < max_found) ? count : max_found;
// }

// void scanDevices() {
//   uint8_t devices[16];
//   int device_size = sizeof(devices);
//   uint8_t n = I2C_Scan(&hi2c1, devices, device_size);

// }

MPU9250AccelerometerData_t readMPU9250() {
  uint8_t data_buffer[6];

  HAL_StatusTypeDef status;

  status = HAL_I2C_Mem_Read(&hi2c1, 
                            (MPU9250_DEVICE_ADDR << 1), 
                            MPU9250_1ST_ADDR_BYTE, 
                            I2C_MEMADD_SIZE_8BIT, 
                            data_buffer, 6, HAL_MAX_DELAY);

  if (status != HAL_OK) {
    MPU9250AccelerometerData_t accel_data = {0, 0, 0};
    return accel_data;
  }

  int16_t axis_x = (int16_t)((data_buffer[0] << 8) | data_buffer[1]);
  int16_t axis_y = (int16_t)((data_buffer[2] << 8) | data_buffer[3]);
  int16_t axis_z = (int16_t)((data_buffer[4] << 8) | data_buffer[5]);

  MPU9250AccelerometerData_t accel_data = {axis_x, axis_y, axis_z};

  return accel_data;
}

void readMPUTest9250() {
    MPU9250AccelerometerData_t data = readMPU9250();

    while(1) {
      char x[12]; // Large enough to hold any 32-bit integer (-2147483648 to 2147483647) plus '\0'
      char y[12];
      char z[12]; 

      snprintf(x, sizeof(x), "x: %d", data.ax);
      snprintf(y, sizeof(y), "y: %d", data.ay);
      snprintf(z, sizeof(z), "z: %d", data.az);

      ILI9341_DrawString(0 , 7 * (0 * 2) + (5 * 0), "              ", ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
      ILI9341_DrawString(0 , 7 * (1 * 2) + (5 * 1), "              ", ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
      ILI9341_DrawString(0 , 7 * (2 * 2) + (5 * 2), "              ", ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);

      ILI9341_DrawString(0 , 7 * (0 * 2) + (5 * 0), x, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
      ILI9341_DrawString(0 , 7 * (1 * 2) + (5 * 1), y, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
      ILI9341_DrawString(0 , 7 * (2 * 2) + (5 * 2), z, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
    }
}

// USE:
  // if (MPU9250_WakeUp() != HAL_OK) {
  //     // Error handling: Toggle an LED or print an error if communication fails
  //     printf("Failed to wake up MPU-9250!\r\n");
  // }

HAL_StatusTypeDef MPU9250_WakeUp(void) {
    uint8_t data = MPU9250_WAKE_UP_VAL;
    
    // Shift device address left by 1 because HAL expects a 7-bit address shifted for the R/W bit
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c1, 
                                                 (MPU9250_DEVICE_ADDR << 1), 
                                                 MPU9250_PWR_MGMT_1_REG, 
                                                 I2C_MEMADD_SIZE_8BIT, 
                                                 &data, 
                                                 1, 
                                                 HAL_MAX_DELAY);
    return status;
}

void MPU_Diag(void) {
  char line[24];
  uint8_t who = 0xFF;

  HAL_StatusTypeDef s1 = HAL_I2C_IsDeviceReady(&hi2c1, 0x68 << 1, 3, 20);
  uint32_t e1 = hi2c1.ErrorCode;
  uint32_t busy = __HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY) ? 1 : 0;

  HAL_StatusTypeDef s2 = HAL_I2C_Mem_Read(&hi2c1, 0x68 << 1, 0x75,
                                          I2C_MEMADD_SIZE_8BIT, &who, 1, 20);
  uint32_t e2 = hi2c1.ErrorCode;

  snprintf(line, sizeof(line), "rdy:%d err:%02lX bsy:%lu", s1, e1, busy);
  ILI9341_DrawString(0, 0, line, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
  snprintf(line, sizeof(line), "who:%02X st:%d e:%02lX", who, s2, e2);
  ILI9341_DrawString(0, 20, line, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE, 2);
  while (1) {}   // park here so you can read it
}

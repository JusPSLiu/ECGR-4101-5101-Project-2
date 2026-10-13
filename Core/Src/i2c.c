#include "i2c.h"


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

MPU9250AccelerometerData readMPU9250() {
  uint8_t data_buffer[6];

  HAL_StatusTypeDef status;

  status = HAL_I2C_Mem_Read(&hi2c1, (MPU9250_DEVICE_ADDR << 1), MPU9250_1ST_ADDR_BYTE, I2C_MEMADD_SIZE_8BIT, data_buffer, 6, HAL_MAX_DELAY);

  int16_t accel_x_h = data_buffer[0];
  int16_t accel_x_l = data_buffer[1];

  int16_t axis_x = (int16_t)(accel_x_h << 8) | accel_x_l;

  int16_t accel_y_h = data_buffer[2];
  int16_t accel_y_l = data_buffer[3];

  int16_t axis_y = (int16_t)(accel_y_h << 8) | accel_y_l;

  int16_t accel_z_h = data_buffer[4];
  int16_t accel_z_l = data_buffer[5];

  int16_t axis_z = (int16_t)(accel_z_h << 8) | accel_z_l;

  MPU9250AccelerometerData accel_data = {axis_x, axis_y, axis_z};

  return accel_data;
}

void readMPUTest9250() {
    MPU9250AccelerometerData data = readMPU9250();

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
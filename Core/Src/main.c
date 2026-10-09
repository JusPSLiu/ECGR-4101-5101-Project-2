/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : X-NUCLEO-GFX01M2 LCD demo for the NUCLEO-L496ZG-P
 *
 * This demo drives the 2.2" ILI9341 QVGA display on the X-NUCLEO-GFX01M2
 * expansion board over SPI1 and cycles through a few simple graphics demos:
 * color bars, filled/outlined shapes, text, and a live joystick (B1)
 * readout. See Drivers/BSP/GFX01M2 for the small display driver used here.
 *
 * WIRING - the GFX01M2 plugs directly onto the top of this board's CN11/CN12
 * ST Morpho headers. NOTE: ST's own datasheet (DB4236) only lists Nucleo-64
 * boards as compatible with the GFX01M2 - the NUCLEO-L496ZG-P (Nucleo-144)
 * is not officially validated, so UM2750 has no pin table for this exact
 * combination. Pins were instead cross-referenced against this board's own
 * schematic (mb1312-l4xxzx-a03): RESET=PA1, CS=PA9, DC=PB10, SPI1 SCK/MISO/
 * MOSI=PA5/PA6/PA7. See Drivers/BSP/GFX01M2/gfx01m2_conf.h for the full
 * derivation and the PA9/USB_VBUS_Pin sharing note.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gfx01m2_conf.h"
#include "ili9341.h"
#include <stdio.h>
#include "sprite.h"
#include "menu.h"


/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

static uint8_t Joystick_Read(void);

/* DRAWING STUFF TO THE SCREEN */
void drawSprite();
void readInputs(uint8_t);


/* Joystick (B1) bit flags returned by Joystick_Read() */
#define JOY_LEFT_MASK (1U << 0)
#define JOY_CENTER_MASK (1U << 1)
#define JOY_DOWN_MASK (1U << 2)
#define JOY_RIGHT_MASK (1U << 3)
#define JOY_UP_MASK (1U << 4)

// 100x100 pixels, all white (16-bit RGB565 format)
// 100x100 pixel 2-byte (16-bit RGB565) hex code array for an all-white image
// A 100x100 2D array of all-white pixels using 16-bit (2-byte) hex codes
// (0xFFFF for RGB565)
const unsigned short white_image[200][200] = {
    [0 ... 199] = {[0 ... 199] = 0xFFFF}};

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define SPRITE_SIZE 20
#define DST_SIZE 160
#define ANIMATION_DELAY 120
#define ROTATION 0.0f
#define SPRITEX 40
#define SPRITEY 56

#define DEBOUNCE_LEN 1


int menuState = 0;
int isMenuOpen = 0;

int frameCount = 0;
int totalFrames = 10;
uint32_t last_frame_time = 0;
uint32_t ud_frame_time = 0;

uint8_t currAnim = 0;

uint8_t screen_refresh = 1;
uint8_t input_debounce = 0;


/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  // init
  HAL_Init();
  // init clk
  SystemClock_Config();
  // init peripherals
  MX_GPIO_Init();
  MX_SPI1_Init();
  // init lcd
  ILI9341_Init(&hspi1);
  ILI9341_FillScreen(ILI9341_COLOR_WHITE);

  // main loop
  while (1) {
    uint32_t now = HAL_GetTick();

    readInputs(Joystick_Read());

    if (screen_refresh) {
      ILI9341_FillScreen(ILI9341_COLOR_WHITE);
      screen_refresh = 0;
    }

    if (isMenuOpen) {
      drawMenu(menuState);
    } else {
      if (now - last_frame_time >= ANIMATION_DELAY) {
        last_frame_time = now;
        frameCount += 1;
      }

      drawSprite();
    }  
  }
}

void readInputs(uint8_t joy) {
  if (input_debounce > 0) input_debounce--;
  else {
    // LR
    if (joy & JOY_LEFT_MASK) {
      currAnim -= 1; input_debounce = DEBOUNCE_LEN;
    } else if (joy & JOY_RIGHT_MASK) {
      currAnim += 1; input_debounce = DEBOUNCE_LEN;
    }

    // CLICK
    if (joy & JOY_CENTER_MASK) {
      if (isMenuOpen == 0) isMenuOpen = 1;
      else isMenuOpen = 0;
      input_debounce = DEBOUNCE_LEN;
      screen_refresh = 1;
    }

    // DOWN
    if (joy & JOY_DOWN_MASK) {
      menuState++; input_debounce = DEBOUNCE_LEN;
    }  
    
    if (joy & JOY_UP_MASK)  {
      menuState--; input_debounce = DEBOUNCE_LEN;
    }
  }
}

void drawSprite() {
    uint8_t animlen = 0;
    const unsigned short **anim;
    switch(currAnim) {
        case 0:
            anim = eat_anim; animlen = eat_anim_len;
            break;
        case 1:
            anim = die_anim; animlen = die_anim_len;
            break;
        case 2:
            anim = backflip_anim; animlen = backflip_anim_len;
            break;
        case 3:
            anim = lowbatt_anim; animlen = lowbatt_anim_len;
            break;
        case 4:
            anim = idle_anim; animlen = idle_anim_len;
            break;
        case 5:
        default:
            anim = dead_anim; animlen = dead_anim_len;
    }

    if (frameCount >= animlen) frameCount = 0;

    ILI9341_DrawImageScaled(SPRITEX, SPRITEY, 20, 26, (const unsigned short *)anim[frameCount],
           20, 160, 208, 0, 0xFFFF);
}

static uint8_t Joystick_Read(void) {
  uint8_t state = 0;

  /* All five directions are active low (see gfx01m2_conf.h) */
  if (HAL_GPIO_ReadPin(JOY_LEFT_GPIO_Port, JOY_LEFT_Pin) == GPIO_PIN_RESET) {
    state |= JOY_LEFT_MASK;
  }
  if (HAL_GPIO_ReadPin(JOY_CENTER_GPIO_Port, JOY_CENTER_Pin) ==
      GPIO_PIN_RESET) {
    state |= JOY_CENTER_MASK;
  }
  if (HAL_GPIO_ReadPin(JOY_DOWN_GPIO_Port, JOY_DOWN_Pin) == GPIO_PIN_RESET) {
    state |= JOY_DOWN_MASK;
  }
  if (HAL_GPIO_ReadPin(JOY_RIGHT_GPIO_Port, JOY_RIGHT_Pin) == GPIO_PIN_RESET) {
    state |= JOY_RIGHT_MASK;
  }
  if (HAL_GPIO_ReadPin(JOY_UP_GPIO_Port, JOY_UP_Pin) == GPIO_PIN_RESET) {
    state |= JOY_UP_MASK;
  }

  return state;
}

/**
 * @brief SPI1 Initialization Function
 *        SCK = PA5, MISO = PA6, MOSI = PA7 - connects to the GFX01M2 LCD.
 * @retval None
 */
static void MX_SPI1_Init(void) {
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  /* PCLK2 is now ~56.8 MHz (HCLK was previously quartered by a stray AHB
   * DIV4 - see SystemClock_Config). /32 keeps the same ~1.775 MHz target
   * that gave comfortable margin over the ILI9341's 10 MHz write ceiling
   * on this jumper-wired, unsupported-board setup. */
  hspi1.Init.BaudRatePrescaler =
      SPI_BAUDRATEPRESCALER_4; // update this line to increase the speed of the
                               // spi interface lower is higher
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
   */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 71;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  /* DIV4 here previously left HCLK at 56.8/4 = 14.2 MHz despite the PLL
   * being configured for 56.8 MHz - DIV1 makes HCLK match the PLL output. */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  /* HCLK is now ~56.8 MHz - VOS Range 1 requires 3 wait states for
   * 48-64 MHz (RM0351), not the 0 WS that was only valid up to 16 MHz. */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
   */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin | LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(
      GPIOG, USB_PowerSwitchOn_Pin | SMPS_V1_Pin | SMPS_EN_Pin | SMPS_SW_Pin,
      GPIO_PIN_RESET);

  /* LCD CS/RESET idle high, DC state does not matter until the first transfer
   */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin | LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_OverCurrent_Pin SMPS_PG_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin | SMPS_PG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_PowerSwitchOn_Pin SMPS_V1_Pin SMPS_EN_Pin
   * SMPS_SW_Pin */
  GPIO_InitStruct.Pin =
      USB_PowerSwitchOn_Pin | SMPS_V1_Pin | SMPS_EN_Pin | SMPS_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_CS_Pin (X-NUCLEO-GFX01M2) */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /* LCD_DC is on a different port than LCD_CS (GPIOB vs GPIOA) - must be a
   * separate HAL_GPIO_Init call */
  GPIO_InitStruct.Pin = LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RESET_Pin (X-NUCLEO-GFX01M2) */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : JOY_LEFT_Pin JOY_DOWN_Pin JOY_RIGHT_Pin
   * (X-NUCLEO-GFX01M2 joystick) */
  GPIO_InitStruct.Pin = JOY_LEFT_Pin | JOY_DOWN_Pin | JOY_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : JOY_CENTER_Pin JOY_UP_Pin (X-NUCLEO-GFX01M2 joystick)
   */
  GPIO_InitStruct.Pin = JOY_CENTER_Pin | JOY_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

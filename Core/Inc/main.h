/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define open_in_Pin GPIO_PIN_1
#define open_in_GPIO_Port GPIOC
#define close_in_Pin GPIO_PIN_2
#define close_in_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_6
#define LED_RED_GPIO_Port GPIOA
#define LED_GREEN_Pin GPIO_PIN_7
#define LED_GREEN_GPIO_Port GPIOA
#define LED_CAN_Pin GPIO_PIN_4
#define LED_CAN_GPIO_Port GPIOC
#define end_in_Pin GPIO_PIN_6
#define end_in_GPIO_Port GPIOC
#define error_holla_Pin GPIO_PIN_7
#define error_holla_GPIO_Port GPIOC
#define enable_holla_Pin GPIO_PIN_8
#define enable_holla_GPIO_Port GPIOC
#define open_out_Pin GPIO_PIN_4
#define open_out_GPIO_Port GPIOB
#define fb_open_Pin GPIO_PIN_5
#define fb_open_GPIO_Port GPIOB
#define fb_close_Pin GPIO_PIN_6
#define fb_close_GPIO_Port GPIOB
#define close_out_Pin GPIO_PIN_7
#define close_out_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

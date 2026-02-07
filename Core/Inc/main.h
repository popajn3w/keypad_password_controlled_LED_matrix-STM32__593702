/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#include "stdbool.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MAX_PASS_LEN 64    // max 255
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_11
#define BUZZER_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define LED4x4_Port GPIOA
#define LED4x4_C1 GPIO_PIN_1
#define LED4x4_C2 GPIO_PIN_2
#define LED4x4_C3 GPIO_PIN_3
#define LED4x4_C4 GPIO_PIN_4
#define LED4x4_R1 GPIO_PIN_5    // row cathode
#define LED4x4_R2 GPIO_PIN_6
#define LED4x4_R3 GPIO_PIN_7
#define LED4x4_R4 GPIO_PIN_8

#define KEY4x4_Port GPIOB
#define KEY4x4_C1 GPIO_PIN_0
#define KEY4x4_C2 GPIO_PIN_1
#define KEY4x4_C3 GPIO_PIN_3
#define KEY4x4_C4 GPIO_PIN_4
#define KEY4x4_R1 GPIO_PIN_5
#define KEY4x4_R2 GPIO_PIN_6
#define KEY4x4_R3 GPIO_PIN_7
#define KEY4x4_R4 GPIO_PIN_8

// software timer multiplexing 4 timers
typedef struct
{
    unsigned short int T1;
    unsigned short int cnt1;
    unsigned short int T2;
    unsigned short int cnt2;
    unsigned short int T3;
    unsigned short int cnt3_1;
    unsigned short int cnt3_2;
    unsigned short int T4;
    unsigned short int cnt4;
} SOFT_TIM_HandleTypeDef;

typedef struct
{
    int8_t key;
    bool was_processed;
    bool is_held;
} keypad_input_t;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

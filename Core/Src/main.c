/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
volatile SOFT_TIM_HandleTypeDef stim3 = {.T3 = 3, .T4=50};
volatile uint16_t LED4x4Pattern;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void playNoteBlocking(uint16_t FHz, uint16_t T10ms)
{
  bool silent;
  const uint32_t baseTimFreq = 250000;    // after CKDIV4: 1MHz/4=250kHz

  silent = !FHz;
  htim2.Init.Period = (FHz<=5) ? 24999 : baseTimFreq/2 / FHz - 1;    // Fmin = 5Hz

  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if(!silent)
    HAL_TIM_Base_Start_IT(&htim2);

  HAL_TIM_Base_Stop_IT(&htim3);    // temp stop for config
  stim3.cnt1 = 0;
  stim3.T1 = T10ms;
  if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  while(stim3.cnt1 < stim3.T1);    // put sleep and WDR here in the future!

  return;
}

void LED4x4DrawBlocking(uint16_t pattern, uint16_t T10ms)
{
  HAL_TIM_Base_Stop_IT(&htim3);
  HAL_TIM_Base_Stop_IT(&htim4);

  stim3.cnt2 = 0;
  stim3.T2 = T10ms;
  LED4x4Pattern = pattern;

  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim4);

  while(stim3.cnt2 < stim3.T2);    // put sleep and WDR here in the future!

  return;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    switch(htim->Instance) {
        case TIM2: TIM2_PeriodElapsedCallback_ISR(htim);
                   break;
        case TIM3: TIM3_PeriodElapsedCallback_ISR(htim);
                   break;
        case TIM4: TIM4_PeriodElapsedCallback_ISR(htim);
                   break;
    }
}

void TIM2_PeriodElapsedCallback_ISR(TIM_HandleTypeDef *htim)
{
    HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
}

void TIM3_PeriodElapsedCallback_ISR(TIM_HandleTypeDef *htim)
{
    if(stim3.cnt1 < stim3.T1)
        stim3.cnt1 ++;
    else if(stim3.cnt1 == stim3.T1)    // ISR
    {
        HAL_TIM_Base_Stop_IT(&htim2);    // stop playing note
        stim3.cnt1 = 0;
    }

    if(stim3.cnt2 < stim3.T2)
        stim3.cnt2 ++;
    else if(stim3.cnt2 == stim3.T2)    // ISR
    {
        //LED4x4Pattern = 0x0000;    // the screen should be cleared from main
        HAL_TIM_Base_Stop_IT(&htim4);    // freeze screen
        stim3.cnt2 = 0;
    }

    // keypad debounce or some other timing
    //if(stim3.cnt3_1 < stim3.T3)
    //    stim3.cnt3_1 ++;
    //else if(stim3.cnt3_1 == stim3.T3)    // ISR
    //{
    //    if(!HAL_GPIO_ReadPin(KEY4x4_Port,KEY4x4_C1) || !HAL_GPIO_ReadPin(KEY4x4_Port,KEY4x4_C1)
    //       || !HAL_GPIO_ReadPin(KEY4x4_Port,KEY4x4_C1) || !HAL_GPIO_ReadPin(KEY4x4_Port,KEY4x4_C1))
    //    {
    //        ;
    //    }
    //    stim3.cnt3_1++;
    //}

    if(stim3.cnt4 < stim3.T4)
        stim3.cnt4 ++;
    else if(stim3.cnt4 == stim3.T4)    // ISR
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        stim3.cnt4 = 0;
    }
}

void TIM4_PeriodElapsedCallback_ISR(TIM_HandleTypeDef *htim)
{
  static uint8_t iled=0;
  bool lit;

  lit = (LED4x4Pattern & (1<<iled)) ? 1 : 0;
  switch(iled)
  {
    case 0:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C1, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R1, !lit);
              break;
    case 1:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C2, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R1, !lit);
              break;
    case 2:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C3, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R1, !lit);
              break;
    case 3:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C4, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R1, !lit);
              break;
    case 4:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C1, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R2, !lit);
              break;
    case 5:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C2, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R2, !lit);
              break;
    case 6:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C3, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R2, !lit);
              break;
    case 7:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C4, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R2, !lit);
              break;
    case 8:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C1, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R3, !lit);
              break;
    case 9:   HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C2, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R3, !lit);
              break;
    case 10:  HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C3, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R3, !lit);
              break;
    case 11:  HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C4, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R3, !lit);
              break;
    case 12:  HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C1, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R4, !lit);
              break;
    case 13:  HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C2, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R4, !lit);
              break;
    case 14:  HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C3, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R4, !lit);
              break;
    case 15:  HAL_GPIO_WritePin(LED4x4_Port, LED4x4_C4, lit);
              HAL_GPIO_WritePin(LED4x4_Port, LED4x4_R4, !lit);
              break;
  }

  iled = (iled+1) % 16;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  int i;

  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim4);

  for(i=0; i<16; i++)
  {
    LED4x4DrawBlocking(1<<i, 50);
    LED4x4DrawBlocking(0, 30);
  }

  LED4x4DrawBlocking(0b1111000011010001, 100);
  LED4x4DrawBlocking(0b0000000000000000, 50);

  LED4x4DrawBlocking(0b0100111100101110, 100);
  LED4x4DrawBlocking(0b0000000000000000, 50);

  LED4x4DrawBlocking(0b0101010101010101, 100);
  LED4x4DrawBlocking(0b0000000000000000, 50);

  LED4x4DrawBlocking(0b1111000011010001, 100);
  LED4x4DrawBlocking(0b0000000000000000, 50);

  LED4x4DrawBlocking(0b0100111100101110, 100);
  LED4x4DrawBlocking(0b0000000000000000, 50);

  LED4x4DrawBlocking(0b0101010101010101, 100);
  LED4x4DrawBlocking(0b0000000000000000, 50);

  playNoteBlocking(1500,100);
  playNoteBlocking(0,50);
  playNoteBlocking(1500,100);
  playNoteBlocking(0,50);
  playNoteBlocking(2000,100);
  playNoteBlocking(0,50);
  playNoteBlocking(6000,50);

  playNoteBlocking(1500,100);
  playNoteBlocking(0,50);
  playNoteBlocking(1500,100);
  playNoteBlocking(0,50);
  playNoteBlocking(2000,100);
  playNoteBlocking(0,50);
  playNoteBlocking(2500,100);
  playNoteBlocking(0,50);
  playNoteBlocking(3500,100);
  playNoteBlocking(0,50);

  playNoteBlocking(1000,40);
  playNoteBlocking(500,40);

  playNoteBlocking(0,100);
  //playNoteBlocking(10000,300);
  playNoteBlocking(5500,300);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 62;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 99;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 4;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4
                           PA5 PA6 PA7 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB3 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB5 PB6 PB7 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

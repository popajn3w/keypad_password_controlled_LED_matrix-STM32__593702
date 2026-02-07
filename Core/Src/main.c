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
#include "string.h"

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
volatile SOFT_TIM_HandleTypeDef stim3 = {.T3 = 3, .T4 = 50};
volatile uint16_t LED4x4Pattern;
volatile uint16_queue notes_F_q = {.front = -1, .rear = -1};
volatile uint16_queue notes_T_q = {.front = -1, .rear = -1};
volatile uint16_queue patterns_img_q = {.front = -1, .rear = -1};
volatile uint16_queue patterns_T_q = {.front = -1, .rear = -1};
const int8_t keypad[4][4] = {'1','2','3','A',
                             '4','5','6','B',
                             '7','8','9','C',
                             '*','0','#','D'};
volatile keypad_input_t keypad_input = {.key = 0, .was_processed = true, .is_held = false};
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
}

void playNote(uint16_t FHz, uint16_t T10ms)
{
    while (uint16_queue_isFull(notes_F_q))
        HAL_Delay(5);
    uint16_queue_enqueue(notes_F_q, FHz);
    uint16_queue_enqueue(notes_T_q, T10ms);
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
}

void LED4x4Draw(uint16_t pattern, uint16_t T10ms)
{
    while (uint16_queue_isFull(patterns_img_q))
        HAL_Delay(5);
    uint16_queue_enqueue(patterns_img_q, pattern);
    uint16_queue_enqueue(patterns_T_q, T10ms);
}

void keyDrawOnLED4x4(char key, bool inverted)
{
    uint16_t pattern;
    switch(key)
    {
        case '1': pattern = 0b1000000000000000;  break;
        case '2': pattern = 0b0100000000000000;  break;
        case '3': pattern = 0b0010000000000000;  break;
        case 'A': pattern = 0b0001000000000000;  break;
        case '4': pattern = 0b0000100000000000;  break;
        case '5': pattern = 0b0000010000000000;  break;
        case '6': pattern = 0b0000001000000000;  break;
        case 'B': pattern = 0b0000000100000000;  break;
        case '7': pattern = 0b0000000010000000;  break;
        case '8': pattern = 0b0000000001000000;  break;
        case '9': pattern = 0b0000000000100000;  break;
        case 'C': pattern = 0b0000000000010000;  break;
        case '*': pattern = 0b0000000000001000;  break;
        case '0': pattern = 0b0000000000000100;  break;
        case '#': pattern = 0b0000000000000010;  break;
        case 'D': pattern = 0b0000000000000001;  break;
        default:  pattern = 0;                   break;
    }
    pattern = inverted ? ~pattern : pattern;
    LED4x4Draw(pattern, 30);
    inverted ? LED4x4Draw(0xFFFF,10) : LED4x4Draw(0x0000,10);
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
    bool silent;
    const uint32_t noteBaseTimFreq = 250000;    // after CKDIV4: 1MHz/4=250kHz
    uint16_t note_FHz;
    static uint8_t irow, pressed_col;

    if(stim3.cnt1 < stim3.T1)
        stim3.cnt1 ++;
    else if(stim3.cnt1 == stim3.T1)    // ISR
    {
        HAL_TIM_Base_Stop_IT(&htim2);    // stop playing note
        if (uint16_queue_isEmpty(notes_F_q))
            stim3.T1 = 0;    // check at htim3 period (==10ms) for notes
        else
        {
            // start playing next buffered note
            note_FHz = uint16_queue_dequeue(notes_F_q);
            stim3.T1 = uint16_queue_dequeue(notes_T_q);

            silent = !note_FHz;
            // Fmin = 5Hz
            htim2.Init.Period = (note_FHz<=5) ? 24999 : noteBaseTimFreq/2 / note_FHz - 1;
            if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
            {
                Error_Handler();
            }
            if(!silent)
                HAL_TIM_Base_Start_IT(&htim2);
        }
        stim3.cnt1 = 0;
    }

    if(stim3.cnt2 < stim3.T2)
        stim3.cnt2 ++;
    else if(stim3.cnt2 == stim3.T2)    // ISR
    {
        //LED4x4Pattern = 0x0000;    // the screen should be cleared from main
        // to hold the last pattern, htim4 must keep running
        //HAL_TIM_Base_Stop_IT(&htim4);
        if (uint16_queue_isEmpty(patterns_img_q))
            stim3.T2 = 0;    // check at htim3 period (==10ms) for patterns
        else
        {
            LED4x4Pattern = uint16_queue_dequeue(patterns_img_q);
            stim3.T2 = uint16_queue_dequeue(patterns_T_q);
        }
        stim3.cnt2 = 0;
    }

    // keypad scanning, stim3.T3_1 = 0 (10ms period)
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
    if (!keypad_input.is_held)
    {
        pressed_col = -1;    // read col
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C1) ? 0 : pressed_col;
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C2) ? 1 : pressed_col;
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C3) ? 2 : pressed_col;
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C4) ? 3 : pressed_col;
        if (pressed_col != -1)
        {
            keypad_input.key = keypad[irow][pressed_col];
            keypad_input.was_processed = false;
            keypad_input.is_held = true;
        }
        else    // no keypress, iterate row
        {
            irow++;
            switch(irow)
            {
                case 0: HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R4, 1);
                        HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R1, 0);
                        break;
                case 1: HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R1, 1);
                        HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R2, 0);
                        break;
                case 2: HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R2, 1);
                        HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R3, 0);
                        break;
                case 3: HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R3, 1);
                        HAL_GPIO_WritePin(KEY4x4_Port, KEY4x4_R4, 0);
                        break;
            }
        }
    }
    else    // wait for release
    {
        pressed_col = -1;
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C1) ? 0 : pressed_col;
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C2) ? 1 : pressed_col;
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C3) ? 2 : pressed_col;
        pressed_col = !HAL_GPIO_ReadPin(KEY4x4_Port, KEY4x4_C4) ? 3 : pressed_col;
        // still pressed, but something else -> register;  wait if same key
        if ((pressed_col != -1) && (keypad[irow][pressed_col] != keypad_input.key))
        {
            keypad_input.key = keypad[irow][pressed_col];
            keypad_input.was_processed = false;
        }
        else if (pressed_col == -1)
            keypad_input.is_held = false;
    }

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

  lit = LED4x4Pattern & (1<<(15-iled));
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
    char stored_pass[MAX_PASS_LEN+1];
    uint8_t stored_pass_len = 0;
    char typed_pass[MAX_PASS_LEN+1], last_typed_char = 0;
    enum lock_state_t {LOCKED, UNLOCKED, SET_NEW_CHECK_OLD, SET_NEW} lock_state = UNLOCKED;
    int i, j;

    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_Base_Start_IT(&htim4);

    for(i=0; i<16; i++)
    {
        LED4x4DrawBlocking(1<<i, 50);
        LED4x4DrawBlocking(0, 30);
    }

    LED4x4DrawBlocking(0b1000101100001111, 100);
    LED4x4DrawBlocking(0b0000000000000000, 50);

    LED4x4DrawBlocking(0b0111010011110010, 100);
    LED4x4DrawBlocking(0b0000000000000000, 50);

    LED4x4DrawBlocking(0b1010101010101010, 100);
    LED4x4DrawBlocking(0b0000000000000000, 50);

    LED4x4DrawBlocking(0b1000101100001111, 100);
    LED4x4DrawBlocking(0b0000000000000000, 50);

    LED4x4DrawBlocking(0b0111010011110010, 100);
    LED4x4DrawBlocking(0b0000000000000000, 50);

    LED4x4DrawBlocking(0b1010101010101010, 100);
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
    i = 0;
    LED4x4Draw(0b0000000000000000, 10);
    while (1)
    {
        if (!keypad_input.was_processed)
        {
            switch (lock_state)
            {
                case UNLOCKED:
                    if (keypad_input.key == '#'  &&  last_typed_char == '#')
                    {
                        LED4x4Draw(0b1111000000000000, 100);
                        LED4x4Draw(0b1111111100000000, 100);
                        LED4x4Draw(0b1111111111110000, 100);
                        LED4x4Draw(0b1111111111111111, 100);
                        lock_state = LOCKED;
                    }
                    if (keypad_input.key == '*'  &&  last_typed_char == '*')
                        lock_state = SET_NEW;
                    break;

                case SET_NEW_CHECK_OLD:
                    if (keypad_input.key == '*')        // flush
                        i = 0;
                    else if (keypad_input.key == '#'  || i >= MAX_PASS_LEN)    // check
                    {
                        if (i==stored_pass_len && !memcmp(typed_pass, stored_pass, i))
                        {
                            lock_state = SET_NEW;
                            playNote(440, 50);    // play success tune
                            playNote(0, 25);
                            playNote(440, 50);
                            playNote(0, 25);
                            playNote(440, 25);
                            playNote(880, 25);
                            playNote(1760, 25);
                        }
                        else
                        {
                            playNote(440, 50);    // play fail tune
                            playNote(0, 25);
                            playNote(220, 50);
                            playNote(0, 25);
                            playNote(220, 50);
                            lock_state = UNLOCKED;
                        }
                        i = 0;
                    }
                    else            // store char
                        typed_pass[i++] = keypad_input.key;
                    break;

                case SET_NEW:
                    if (keypad_input.key == '*')
                        i = 0;
                    else if (keypad_input == '#')
                    {
                        LED4x4Draw(0b1111000000000000, 100);
                        LED4x4Draw(0b1111111100000000, 100);
                        LED4x4Draw(0b1111111111110000, 100);
                        LED4x4Draw(0b1111111111111111, 100);
                        stored_pass_len = i;
                        i = 0;
                        lock_state = LOCKED;
                    }
                    else
                        stored_pass[i++] = keypad_input.key;
                    break;

                case LOCKED:
                    if (keypad_input.key == '*')        // flush
                        i = 0;
                    else if (keypad_input.key == '#'  || i >= MAX_PASS_LEN)    // check
                    {
                        if (i==stored_pass_len && !memcmp(typed_pass, stored_pass, i))
                        {
                            lock_state = UNLOCKED;
                            playNote(1760, 50);    // play success tune
                            playNote(0, 25);
                            playNote(880, 50);
                            playNote(0, 25);
                            playNote(3520, 25);
                            LED4x4Draw(0b1111111111110000, 100);
                            LED4x4Draw(0b1111111100000000, 100);
                            LED4x4Draw(0b1111000000000000, 100);
                            LED4x4Draw(0b0000000000000000, 100);
                        }
                        else
                        playNote(440, 50);    // play fail tune
                        playNote(0, 25);
                        playNote(220, 50);
                        playNote(0, 25);
                        playNote(220, 50);
                        i = 0;
                    }
                    else            // store char
                        typed_pass[i++] = keypad_input.key;
                    break;
            }
            keyDrawOnLED4x4(keypad_input.key, lock_state==LOCKED);
            last_typed_char = keypad_input.key;
            keypad_input.was_processed = true;
        }
    }
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

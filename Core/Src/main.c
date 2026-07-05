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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Emm_V5.h"
#include "motion.h"
#include "view.h"
#include "distance.h"
#include <stdio.h>
#include "normaldj.h"
#include "protocol.h"
#include "parse.h"
#include "motioncmd.h"
#include "command_queue.h"
#include "serial.h"
#include "planner.h"
#include "motor_serial.h"
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

/* USER CODE BEGIN PV */
extern uint8_t RxByte;
extern volatile uint8_t overflag;
extern uint8_t uart_rx_buf[64];

uint8_t test=0;

/*==================== 调试计数器 ====================*/
volatile uint32_t dbg_serial_rx     = 0;
volatile uint32_t dbg_parse_ok      = 0;
volatile uint32_t dbg_parse_fail    = 0;
volatile uint32_t dbg_queue_push    = 0;
volatile uint32_t dbg_queue_full    = 0;
volatile uint32_t dbg_planner_pop   = 0;
volatile uint32_t dbg_planner_busy  = 0;
volatile uint32_t dbg_motor_poll    = 0;
volatile uint32_t dbg_go_called     = 0;
volatile uint32_t dbg_frame_parse   = 0;
volatile uint32_t dbg_frame_skip    = 0;
volatile uint32_t dbg_idle_found    = 0;
volatile uint32_t dbg_run_found     = 0;
volatile uint32_t dbg_usart1_idle   = 0;
volatile uint32_t dbg_usart1_len    = 0;
volatile uint32_t dbg_motor1_state  = 0;
volatile uint32_t dbg_motor2_state  = 0;

#define MOTOR_RX_BUF_SIZE 128
extern uint8_t usart1_rx_buf[MOTOR_RX_BUF_SIZE];
extern uint8_t usart1_frame_buf[MOTOR_RX_BUF_SIZE];
 uint32_t counter = 0;

int16_t x=0,y=0;
//lo,ld,ro,rd
uint16_t lo[]={0,0};
uint16_t ld[]={0,0};
uint16_t ro[]={0,0};
uint16_t rd[]={0,0};

/* 软件脉冲计数，替代 RCR（RCR 硬件只有 8 位，最大 255）
   normaldj.c 的 motor_pulse_start() 写入，回调里递减 */
volatile uint16_t pulse_remaining = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_UART4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  //__HAL_UART_CLEAR_IDLEFLAG(&huart3); 											// 清除IDLE标志	

  //test=HAL_UARTEx_ReceiveToIdle_DMA(&huart3,distance_buffer,buffer);
  HAL_UARTEx_ReceiveToIdle_DMA(&huart3,uart_rx_buf,sizeof(uart_rx_buf));
	HAL_UART_Receive_IT(&huart4, &RxByte, 1);
 // state.view=0;
 // state.go=0;
 // state.over=0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_Delay(500);
	//Emm_V5_Pos_Control(1, 0, 100, 0, 3200, 0, 0);
	
/**********************************************************
***	上电延时500毫秒等待闭环初始化完毕
**********************************************************/
	//while(rxFrameFlag == false); rxFrameFlag = false;
  planner_init();
  queue_init();
  motor_serial_init();
 // parser_init();

  /* 中断优先级只需设一次 */
  HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

  while (1)
  {
    serial_process();

    planner_process();

    motor_poll();

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief  TIM1 更新中断回调（RCR=0，每个 PWM 周期触发一次）
 *         由软件 pulse_remaining 计数，减到 0 时自动停止
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        if (--pulse_remaining == 0)
        {
            HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
            __HAL_TIM_MOE_DISABLE(&htim1);
            __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE);
        }
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{ 
  if(huart != &huart3)
  {
    return;
  }    
  serial_rx_callback(uart_rx_buf, Size);

  HAL_UARTEx_ReceiveToIdle_DMA(&huart3,uart_rx_buf,sizeof(uart_rx_buf)
);
}
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
#ifdef USE_FULL_ASSERT
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

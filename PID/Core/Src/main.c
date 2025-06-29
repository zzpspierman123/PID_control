/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint8_t KeyNum = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
float Target = 50, Actual, Out;			//目标值，实际值，输出值
float Error0, Error1, ErrorInt;		//本次误差，上次误差，误差积分
float Kp=10, Ki=5, Kd;					//比例项，积分项，微分项的权重

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == (&htim2))
	{
		//当前速度
		Actual = (__HAL_TIM_GET_COUNTER(&htim4)) / (52*0.01); //速度 = 脉冲总数 / （单圈脉冲总数 * 时间周期）
		__HAL_TIM_SET_COUNTER(&htim4,0);
			
		//本次误差Error0 上次误差Error1
		Error1 = Error0;			
		Error0 = Target - Actual;	
			
		//误差积分
		if (Ki != 0)				
		{
			ErrorInt += Error0;		
		}
		else						
		{
			ErrorInt = 0;			
		}
		
		//PID计算
		Out = Kp * Error0 + Ki * ErrorInt + Kd * (Error0 - Error1);
	
		//输出限幅
		if (Out > 7200) {Out = 7200;}	
		if (Out < -7200) {Out = -7200;}	
		
		/*执行控制*/
		/*输出值给到电机PWM*/	
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, Out);
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin== GPIO_PIN_0)
	{
		HAL_Delay(20);
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==1)
		{
			KeyNum++;
		}
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
	}
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
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_Base_Start_IT(&htim2);
	
	
	 OLED_Init();                          
	 OLED_Clear();                         

	OLED_ShowString(0,0,"Target:",16, 1);   
	OLED_ShowString(0,3,"Actual:",16,1);
	OLED_ShowString(0,6,"Out:",16,1); 

	OLED_ShowString(100,0,"r/s",16, 1);   
	OLED_ShowString(100,3,"r/s",16,1);
	OLED_ShowString(100,6,"%",16,1); 
  /* USER CODE END 2 */

			
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(KeyNum==0)
		{
			Target = 50;
		}
		else if(KeyNum==1)
		{
			Target = 30;
		}
		else if(KeyNum==3)
		{
			Target = 10;
		}
//		HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_10);
	 OLED_ShowNum(60,0,Target,3,16, 0);
	 OLED_ShowNum(60,3,Actual,3,16, 0);
	 OLED_ShowNum(60,6,Out/7200*100,3,16, 0);
		printf("%f,%f,%f\r\n", Target, Actual, Out/7200*100);	
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
	return ch;
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

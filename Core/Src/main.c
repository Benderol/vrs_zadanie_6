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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "lps22hb.h"
#include "hts221.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SAMPLE_PERIOD_MS  200
#define MESSAGE_SIZE      64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static int ScaleFloat(float value, int factor)
{
  return (int)((value * (float)factor) + ((value >= 0.0f) ? 0.5f : -0.5f));
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
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  LPS22HB_RegisterBus(I2C1_ReadByte, I2C1_ReadBytes, I2C1_WriteByte, I2C1_WriteBytes);
  HTS221_RegisterBus(I2C1_ReadByte, I2C1_ReadBytes, I2C1_WriteByte, I2C1_WriteBytes);

  uint8_t pressureReady = LPS22HB_Init();
  uint8_t humidityReady = HTS221_Init();

  if(pressureReady)
  {
    LL_mDelay(50);
    LPS22HB_SetGroundReference();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if(!pressureReady)
    {
      pressureReady = LPS22HB_Init();

      if(pressureReady)
      {
        LPS22HB_SetGroundReference();
      }
    }

    if(!humidityReady)
    {
      humidityReady = HTS221_Init();
    }

    if(!pressureReady || !humidityReady)
    {
      char scan[96];
      int length = sprintf(scan, "SCAN:");

      for(uint8_t address = 0x08; address < 0x78; address++)
      {
        if(I2C1_IsDeviceReady(address))
        {
          length += sprintf(scan + length, " %02X", address);
        }
      }

      sprintf(scan + length, "\r\n");
      USART2_SendString(scan);
    }

    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;
    float altitude = 0.0f;

    if(pressureReady)
    {
      pressureReady = LPS22HB_ReadPressure(&pressure);
      altitude = LPS22HB_CalculateAltitude(pressure);
    }

    if(humidityReady)
    {
      humidityReady = HTS221_ReadTemperature(&temperature);
      HTS221_ReadHumidity(&humidity);
    }

    int temperatureScaled = ScaleFloat(temperature, 10);
    int humidityScaled = ScaleFloat(humidity, 1);
    int pressureScaled = ScaleFloat(pressure, 100);
    int altitudeScaled = ScaleFloat(altitude, 100);

    char message[MESSAGE_SIZE];

    sprintf(message, "%s%d.%d,%d,%d.%02d,%s%d.%02d\r\n",
            (temperatureScaled < 0) ? "-" : "",
            abs(temperatureScaled) / 10, abs(temperatureScaled) % 10,
            humidityScaled,
            pressureScaled / 100, pressureScaled % 100,
            (altitudeScaled < 0) ? "-" : "",
            abs(altitudeScaled) / 100, abs(altitudeScaled) % 100);

    USART2_SendString(message);

    LL_mDelay(SAMPLE_PERIOD_MS);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SetSystemCoreClock(8000000);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_HSI);
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

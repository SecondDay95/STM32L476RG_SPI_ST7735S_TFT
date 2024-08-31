/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include "lcd.h"
#include "forbot_logo.c"
//Dodanie plikow naglowkowych do funkcji uzywanych przez biblioteke hagl:
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t test_image[64 * 64];

int __io_putchar(int ch)
{
  if (ch == '\n') {
    __io_putchar('\r');
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

  return 1;
}

//Funkcja wywolywna po zakonczeniu transmisji przez SPI:
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {

	if(hspi == &hspi2) {

		lcd_transfer_done();

	}
}
uint32_t last_fotoresistor_value = 0;
void draw_progress_bar(uint32_t fotoresistor_value) {

	int y;
	y = 0.09 * fotoresistor_value - 2.7;
	while (lcd_is_busy()) {}
	if(fotoresistor_value >= last_fotoresistor_value)
		hagl_fill_rectangle(31, 49, 29 + y, 41, YELLOW);
	else
		hagl_fill_rectangle(30 + y, 49, 128, 41, BLACK);
	lcd_copy();
	last_fotoresistor_value = fotoresistor_value;

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
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //Inicjalizacja wyswietlacza:
  lcd_init();

  //Rysowanie kolorowych prostokatow:
  /*
  lcd_fill_box(0, 0, 160, 16, RED);
  lcd_fill_box(0, 16, 160, 16, GREEN);
  lcd_fill_box(0, 32, 160, 16, BLUE);
  lcd_fill_box(0, 48, 160, 16, YELLOW);
  lcd_fill_box(0, 64, 160, 16, MAGENTA);
  lcd_fill_box(0, 80, 160, 16, CYAN);
  lcd_fill_box(0, 96, 160, 16, WHITE);
  lcd_fill_box(0, 112, 160, 16, BLACK);
  */

  //Rysowanie 2 skrzyżowanych ze soba linii:
  /*
  for (int i = 0; i < 128; i++) {
    lcd_put_pixel(i,  i, RED);
    lcd_put_pixel(127 - i,  i, RED);
  }
  */

  //Rysowanie obrazu:
  /*
  lcd_draw_image(35, 20, 100, 80, forbot_logo);
  */

  //Rysowanie obrazów - białych prostokatow:
  /*
  for (int i = 0; i < 64 * 64; i++)
    test_image[i] = __REV16(BLUE);

  lcd_draw_image(0, 0, 64, 64, test_image);
  lcd_draw_image(16, 16, 64, 64, test_image);
  lcd_draw_image(32, 32, 64, 64, test_image);
  lcd_draw_image(48, 48, 64, 64, test_image);
  lcd_draw_image(64, 64, 64, 64, test_image);
  */

  //Rysowanie kolorowego gradientu na wyswietlaczu:
  /*
  for (int y = 0; y < LCD_HEIGHT; y++) {
    for (int x = 0; x < LCD_WIDTH; x++) {
      lcd_put_pixel(x, y, __REV16(x / 10 + y * 16));
    }
  }
  */
  //Wykorzystanie biblioteki hagl do rysowania zaokraglonego prostokata oraz do pisania tekstu:
  for (int i = 0; i < 8; i++) {
    hagl_draw_rounded_rectangle(2+i, 2+i, 158-i, 126-i, 8-i, rgb565(0, 0, i*16));
  }

  hagl_put_text(L"Jasnosc pomieszczenia", 16, 25, YELLOW, font6x9);

  //Zolty autobus
  /*
  hagl_fill_circle(30, 100, 5, GREEN);
  hagl_fill_circle(90, 100, 5, GREEN);
  hagl_fill_rounded_rectangle(10, 95, 110, 55, 4, YELLOW);
  hagl_fill_rounded_rectangle(110, 95, 130, 75, 4, YELLOW);
  hagl_fill_rounded_rectangle(90, 75, 110, 55, 4, WHITE);
  hagl_draw_rectangle(60, 90, 80, 60, BLACK);
  hagl_fill_rectangle(65, 85, 75, 65, WHITE);
  hagl_fill_rectangle(40, 75, 50, 65, WHITE);
  hagl_fill_rectangle(20, 75, 30, 65, WHITE);
  */

  //Obramowanie progress_bar:
  hagl_draw_rectangle(30, 50, 130, 40, YELLOW);

  //Przesylanie danych z bufora na wyswietlacz:
  lcd_copy();

  volatile static uint16_t value;
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&value, 1);
  //HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&value, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  printf("ADC = %u\n", value);
	  draw_progress_bar(value);

	  HAL_Delay(250);

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
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

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
#include <stdbool.h>
#include "lcd.h"
#include "forbot_logo.c"
//Dodanie plikow naglowkowych do funkcji uzywanych przez biblioteke hagl:
#include "hagl.h"
#include "font6x9.h"
#include "font5x7.h"
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
const short pitch_left_edge = 30;
const short pitch_right_edge = 130;
const short pitch_up_edge = 50;
const short pitch_down_edge = 110;

const short left_goal_left_edge = 20;
const short left_goal_right_edge = 30;
const short left_goal_up_edge = 65;
const short left_goal_down_edge = 95;

const short right_goal_left_edge = 130;
const short right_goal_right_edge = 140;
const short right_goal_up_edge = 65;
const short right_goal_down_edge = 95;

int y1 = 10;
int y2 = 10;
int last_y1 = 0;
int last_y2 = 0;
int circle_position_x = 80;
int circle_position_y = 60;
const short circle_radius = 3;
int last_circle_position_x = 0;
int last_circle_position_y = 0;

int ball_position_x = 80;
int ball_position_y = 80;
const short shot_distance = 10;
const short bounce_distance = 10;
const short ball_radius = 2;

wchar_t result_left = 0;
wchar_t result_right = 0;
wchar_t result_left_start = 0 + '0';
wchar_t result_right_start = 0 + '0';
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t test_image[64 * 64];

static void draw_pitch(short radius) {

	hagl_set_clip_window(left_goal_left_edge - 10, pitch_up_edge,
			right_goal_right_edge + 10, pitch_down_edge);
	hagl_clear_clip_window();
	hagl_draw_rectangle(pitch_left_edge, pitch_up_edge, pitch_right_edge,
			pitch_down_edge, WHITE);
	hagl_draw_rectangle(left_goal_left_edge, left_goal_up_edge, left_goal_right_edge,
			left_goal_down_edge, WHITE);
	hagl_draw_rectangle(right_goal_left_edge, right_goal_up_edge, right_goal_right_edge,
			right_goal_down_edge, WHITE);
	hagl_fill_circle(circle_position_x, circle_position_y, radius, GREEN);
	hagl_fill_circle(ball_position_x, ball_position_y, ball_radius, BLUE);
	lcd_copy();

}

static void shot_goal() {

	hagl_put_text(L"GOAL", 68, 75, YELLOW, font6x9);
	lcd_copy();
	HAL_Delay(1000);
	circle_position_x = 80;
	circle_position_y = 60;
	ball_position_x = 80;
	ball_position_y = 80;

}

static void match_result() {

	wchar_t result_left_char = result_left + '0';
	wchar_t result_right_char = result_right + '0';

	hagl_set_clip_window(30, 115, 130, 125);
	hagl_clear_clip_window();
	hagl_put_text(&result_left_char, 64, 111, YELLOW, font6x9);
	hagl_put_text(L":", 80, 111, YELLOW, font6x9);
	hagl_put_text(&result_right_char, 90, 111, YELLOW, font6x9);
	lcd_copy();
}

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
	if(y >= 100)
		y = 99;
	if(y < 0)
		y = 0;
	while (lcd_is_busy()) {}
	hagl_set_clip_window(0, 0, 159, 127);
	//hagl_clear_clip_window();
	if(fotoresistor_value >= last_fotoresistor_value) {
		//hagl_set_clip_window(30, 50, 28, 40);
		hagl_fill_rectangle(31, 39, 29 + y, 31, YELLOW);
	}
	else {
		//hagl_set_clip_window(29 + y, 49, 128, 41);
		hagl_fill_rectangle(29 + y, 39, 128, 31, BLACK);
	}
	lcd_copy();
	last_fotoresistor_value = fotoresistor_value;

}


uint32_t last_potentiometer_x;
uint32_t last_potentiometer_y;
void control_circle(uint32_t potentiometer_x, uint32_t potentiometer_y) {

	//y1 = 0.037 * potentiometer_x + 4.188;
	y1 = 0.022 * potentiometer_x + 35;
	//y2 = 0.029 * potentiometer_y + 4.796;
	//y2 = 0.0098 * potentiometer_y + 65;
	y2 = 0.0123 * potentiometer_y + 55;

	while(lcd_is_busy()){}

	if(circle_position_y + circle_radius <= ball_position_y - ball_radius   ||
		circle_position_y - circle_radius >= ball_position_y + ball_radius  ||
		((circle_position_y + circle_radius > ball_position_y - ball_radius ||
		circle_position_y - circle_radius < ball_position_y + ball_radius   ||
		circle_position_y == ball_position_y)				                &&
		(circle_position_x + circle_radius < ball_position_x - ball_radius  ||
		circle_position_x - circle_radius >= ball_position_x + ball_radius))) {

		if(y1 > 78 && circle_position_x < 125) {

			circle_position_x ++;

		}
	}
	if(circle_position_y + circle_radius <= ball_position_y - ball_radius   ||
		circle_position_y - circle_radius >= ball_position_y + ball_radius  ||
		((circle_position_y + circle_radius > ball_position_y - ball_radius ||
		circle_position_y - circle_radius < ball_position_y + ball_radius   ||
		circle_position_y == ball_position_y)				                &&
		(circle_position_x - circle_radius > ball_position_x + ball_radius  ||
		circle_position_x + circle_radius <= ball_position_x - ball_radius))) {

		if(y1 < 78 && circle_position_x > 35) {

			circle_position_x --;

		}

	}
	if(circle_position_x + circle_radius <= ball_position_x - ball_radius   ||
		circle_position_x - circle_radius >= ball_position_x + ball_radius  ||
		((circle_position_x + circle_radius > ball_position_x - ball_radius ||
		circle_position_x - circle_radius < ball_position_x + ball_radius   ||
		circle_position_x == ball_position_x)				                &&
		(circle_position_y + circle_radius < ball_position_y - ball_radius  ||
		circle_position_y - circle_radius >= ball_position_y + ball_radius))) {

		if(y2 > 80 && circle_position_y < 105) {

			circle_position_y ++;

		}

	}
	if(circle_position_x + circle_radius <= ball_position_x - ball_radius   ||
		circle_position_x - circle_radius >= ball_position_x + ball_radius  ||
		((circle_position_x + circle_radius > ball_position_x - ball_radius ||
		circle_position_x - circle_radius < ball_position_x + ball_radius   ||
		circle_position_x == ball_position_x)				                &&
		(circle_position_y - circle_radius > ball_position_y + ball_radius  ||
		circle_position_y + circle_radius <= ball_position_y - ball_radius))) {

		if(y2 < 80 && circle_position_y > 55) {

			circle_position_y --;

		}

	}

	if(circle_position_x != last_circle_position_x ||
			circle_position_y != last_circle_position_y) {

		draw_pitch(circle_radius);
		last_circle_position_x = circle_position_x;
		last_circle_position_y = circle_position_y;

	}

}


void control_ball() {

		if(HAL_GPIO_ReadPin(BUTTON_SHOT_GPIO_Port, BUTTON_SHOT_Pin) == GPIO_PIN_SET) {

			bool ball_shot_right = false;
			bool ball_shot_left = false;
			bool ball_shot_up = false;
			bool ball_shot_down = false;

			if(circle_position_x + circle_radius == ball_position_x - ball_radius &&
			   circle_position_y + circle_radius >= ball_position_y - ball_radius &&
			   circle_position_y - circle_radius <= ball_position_y + ball_radius) {

				ball_shot_right = true;
				if(ball_position_x < pitch_right_edge) {

					ball_position_x += shot_distance;

				}
				else if(ball_position_x == pitch_right_edge && (ball_position_y < right_goal_up_edge ||
						ball_position_y > right_goal_down_edge)){

					ball_position_x -= bounce_distance;

				}
				else if(ball_position_x == pitch_right_edge && (ball_position_y >= right_goal_up_edge &&
						ball_position_y <= right_goal_down_edge)) {

					ball_position_x += shot_distance;
					//result_right += 1;

				}

			}
			if(circle_position_x - circle_radius == ball_position_x + ball_radius &&
			   circle_position_y + circle_radius >= ball_position_y - ball_radius &&
			   circle_position_y - circle_radius <= ball_position_y + ball_radius) {

				ball_shot_left = true;

				if(ball_position_x > pitch_left_edge) {

					ball_position_x -= shot_distance;

				}
				else if(ball_position_x == pitch_left_edge && (ball_position_y < right_goal_up_edge ||
						ball_position_y > right_goal_down_edge)){

					ball_position_x += bounce_distance;

				}
				else if(ball_position_x == pitch_left_edge && (ball_position_y >= right_goal_up_edge &&
						ball_position_y <= right_goal_down_edge)) {

					ball_position_x -= shot_distance;
					//result_left += 1;

				}

			}
			if(circle_position_y + circle_radius == ball_position_y - ball_radius &&
			   circle_position_x + circle_radius >= ball_position_x - ball_radius &&
			   circle_position_x - circle_radius <= ball_position_x + ball_radius) {

				ball_shot_up = true;

				if(ball_position_y < 110) {

					ball_position_y += shot_distance;

				}
				else {

					ball_position_y -= bounce_distance;

				}

			}
			if(circle_position_y - circle_radius == ball_position_y + ball_radius &&
			   circle_position_x + circle_radius >= ball_position_x - ball_radius &&
			   circle_position_x - circle_radius <= ball_position_x + ball_radius) {

				ball_shot_down = true;

				if(ball_position_y > 50) {

					ball_position_y -= shot_distance;

				}
				else {

					ball_position_y += bounce_distance;

				}

			}
			if(ball_shot_right || ball_shot_left || ball_shot_up || ball_shot_down) {

				draw_pitch(circle_radius + 1);

				if(ball_position_x == left_goal_left_edge ||
						ball_position_x == right_goal_right_edge) {

					shot_goal();
					//match_result();

				}

			}

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

  hagl_put_text(L"Jasnosc pomieszczenia", 16, 15, YELLOW, font6x9);

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
  hagl_draw_rectangle(30, 40, 130, 30, YELLOW);

  //Obramowanie boiska:
  hagl_draw_rectangle(pitch_left_edge, pitch_up_edge, pitch_right_edge, pitch_down_edge, WHITE);

  //Lewa bramka:
  hagl_draw_rectangle(left_goal_left_edge, left_goal_up_edge, left_goal_right_edge,
		  left_goal_down_edge, WHITE);

  //Prawa bramka:
  hagl_draw_rectangle(right_goal_left_edge, right_goal_up_edge, right_goal_right_edge,
		  right_goal_down_edge, WHITE);

  //hagl_put_text(&result_left_start, 64, 111, YELLOW, font6x9);
  //hagl_put_text(L":", 80, 111, YELLOW, font6x9);
  //hagl_put_text(&result_right_start, 90, 111, YELLOW, font6x9);

  //Przesylanie danych z bufora na wyswietlacz:
  lcd_copy();

  volatile static uint16_t value[3];
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)value, 3);
  //HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&value, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  printf("Photoresistor = %u\n", value[0]);
	  draw_progress_bar(value[0]);

	  printf("Potentiometer 1: %u\n", y1);
	  printf("Potentiometer 2: %u\n", y2);
	  control_circle(value[1], value[2]);
	  control_ball();

	  //HAL_Delay(250);

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

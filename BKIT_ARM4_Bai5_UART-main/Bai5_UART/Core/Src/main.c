/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "uart.h"
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
void system_init();
void test_LedDebug();
void test_Uart();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
struct dateTime {
	int x_axis;
	int y_axis;
	uint8_t *showName;
	uint8_t stopValue;
	uint16_t  fontColor;
	uint16_t  bgColor;
	uint8_t writeName;
	int countDown;
	uint8_t titleName[100];
	int limitStart;
	int limitEnd;
};

struct dateTime dateTimeInfo[7] = {
		{70, 100, &ds3231_hours, 0, GREEN, BLACK, ADDRESS_HOUR, 0, "HOUR", 0, 23},
		{110, 100, &ds3231_min, 0, GREEN, BLACK, ADDRESS_MIN, 0, "MIN", 0, 59},
		{150, 100, &ds3231_sec, 0, GREEN, BLACK, ADDRESS_SEC, 0, "SEC", 0, 59},
		{20, 130, &ds3231_day, 0, YELLOW, BLACK, ADDRESS_DAY, 0, "DAY", 1, 7},
		{70, 130, &ds3231_date, 0, YELLOW, BLACK, ADDRESS_DATE, 0, "DATE", 1, 31},
		{110, 130, &ds3231_month, 0, YELLOW, BLACK, ADDRESS_MONTH, 0, "MONTH", 1, 12},
		{150, 130, &ds3231_year, 0, YELLOW, BLACK, ADDRESS_YEAR, 0, "YEAR", 0, 99},
};

int mode = 0; // 0: view date and time; 1: edit date and time; 2: countdown

int editIndex = 0;
int editBlink = 0;
int tempValue = 0;
uint8_t readIndex = 0;
uint8_t tempStr[100];

int timeOut = 0;
int isSendUartSignal = 0;
int sendReqTimes = 0;
int isExeedReqTimes = 0;
int isWrongInput = 0;
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
  MX_SPI1_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  lcd_Clear(BLACK);
  updateTime();
  while (1)
  {
    /* USER CODE END WHILE */
	  while(!flag_timer[1]);
	  flag_timer[1] = 0;
	  button_Scan();
	  ds3231_ReadTime();
	  buttonHandle();
	  handleUartRXSignal();
	  displayWrongInput ();
	  if (displayTimeOut()) continue;
	  handleUartTXSignal();
//	  lcd_ShowIntNum(150,200, readIndex, 10, GREEN, BLACK, 24);
//	  lcd_ShowIntNum(150,260, writeIndex, 10, GREEN, BLACK, 24);
	  if (displayExceedError()) continue;
	  displayMode();
	  displayTime();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void system_init(){
	  HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	  HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	  HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);
	  timer_init();
	  led7_init();
	  button_init();
	  lcd_init();
	  ds3231_init();
	  uart_init_rs232();
	  setTimer(50, 1);
}

uint16_t count_led_debug = 0;

void test_LedDebug(){
	count_led_debug = (count_led_debug + 1)%20;
	if(count_led_debug == 0){
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

void test_button(){
	for(int i = 0; i < 16; i++){
		if(button_count[i] == 1){
			led7_SetDigit(i/10, 2, 0);
			led7_SetDigit(i%10, 3, 0);
		}
	}
}

void test_Uart(){
	if(button_count[12] == 1){
		uart_Rs232SendNum(ds3231_hours);
		uart_Rs232SendString(":");
		uart_Rs232SendNum(ds3231_min);
		uart_Rs232SendString(":");
		uart_Rs232SendNum(ds3231_sec);
		uart_Rs232SendString("\n");
	}
}

void updateTime(){
	ds3231_Write(ADDRESS_YEAR, 24);
	ds3231_Write(ADDRESS_MONTH, 11);
	ds3231_Write(ADDRESS_DATE, 13);
	ds3231_Write(ADDRESS_DAY, 4);
	ds3231_Write(ADDRESS_HOUR, 9);
	ds3231_Write(ADDRESS_MIN, 27);
	ds3231_Write(ADDRESS_SEC, 23);
}

void displayMode () {
	 lcd_ShowStr(20, 30, "Mode: ", WHITE, RED, 24, 0);
	 lcd_ShowIntNum(150, 30, mode, 1, GREEN, BLACK, 24);
}

int displayTimeOut() {
	if (timeOut) {
		 lcd_StrCenter(0, 2, "TIME OUT !!!!", RED, BLUE, 16, 1);
		 return 1;
	} else return 0;
}

void displayTime(){
	switch (mode) {
	case 0: //view date and time mode
		for (int i = 0; i < 7; i ++) {
			lcd_ShowIntNum (dateTimeInfo[i].x_axis, dateTimeInfo[i].y_axis,
					*(dateTimeInfo[i].showName), 2, dateTimeInfo[i].fontColor,
					dateTimeInfo[i].bgColor, 24);
			if (*(dateTimeInfo[i].showName) == dateTimeInfo[i].countDown) tempValue++;
		}
		if (tempValue == 7) { //7 values are same
			lcd_Clear(BLACK);
			timeOut = 1;
		}
		tempValue = 0;
		break;
	case 1: // edit date and time mode
	case 2: // countdown mode
	case 3:
		if (flag_timer[0] == 1) {
			flag_timer[0] = 0;
			setTimer(500, 0);
			editBlink = (editBlink + 1) % 2;
		}
		if (editBlink) {
			lcd_ShowIntNum(dateTimeInfo[editIndex].x_axis, dateTimeInfo[editIndex].y_axis,
					tempValue, 2, dateTimeInfo[editIndex].fontColor,
					dateTimeInfo[editIndex].bgColor, 24);
		}
		else {
			lcd_ShowIntNum(dateTimeInfo[editIndex].x_axis, dateTimeInfo[editIndex].y_axis,
					tempValue, 2, dateTimeInfo[editIndex].bgColor,
					dateTimeInfo[editIndex].bgColor, 24);
		}

		if (mode == 3) {
			sprintf((char*)tempStr, "Updating %s", dateTimeInfo[editIndex].titleName);
			lcd_StrCenter(0, 2, tempStr, RED, BLUE, 16, 1);
		}
		break;
	default:
		break;
	}
}

void buttonHandle() {
    if (button_count[0] == 1) {
    	mode++;
    	lcd_Clear(BLACK);
    	if (mode > 3) mode = 0;
    	if (mode == 0) {
    		tempValue = 0;
    	} else if (mode == 1) {
    		editIndex = 0;
    		setTimer(500, 0);
			for (int i = 0 ; i < 7 ; i++){
				dateTimeInfo[i].stopValue = *(dateTimeInfo[i].showName);
			}
    		tempValue = dateTimeInfo[0].stopValue;
    	} else if (mode == 2) {
    		editIndex = 0;
    		setTimer(500, 0);
    		for (int i = 0 ; i < 7 ; i++){
    			dateTimeInfo[i].countDown = *(dateTimeInfo[i].showName);
    		}
    		tempValue = dateTimeInfo[0].countDown;
    	} else if (mode == 3) {
    		resetValues();
    		setTimer(500, 0);
    		setTimer(5000, 3);
    		tempValue = *(dateTimeInfo[0].showName);
    		isSendUartSignal = 1;
    		memset(tempStr, 0, sizeof(tempStr));
    	}
    }

	if (button_count[1] == 1) { // reset button
		timeOut = 0;
		lcd_Clear(BLACK);
	}

    switch (mode){
    case 1:
    case 2:
    	if (button_count[3] == 1) // up arrow button
			tempValue ++;
		else if (button_count[3] == 40){
			setTimer(200, 2);
		}
		else if (button_count[3] > 40) {
			if (flag_timer[2] == 1) {
				flag_timer[2] = 0;
				setTimer(200, 2);
				tempValue ++;
				if (tempValue > 99) tempValue = 0;
			}
		}

		if (button_count[7] == 1) // down arrow button
			tempValue --;
		else if (button_count[7] == 40){
			setTimer(200, 2);
		}
		else if (button_count[7] > 40) {
			if (flag_timer[2] == 1) {
				flag_timer[2] = 0;
				setTimer(200, 2);
				tempValue --;
				if (tempValue < 0) tempValue = 0;
			}
		}

		if (button_count[12] == 1) { // save button
			if (mode == 1) ds3231_Write(dateTimeInfo[editIndex].writeName, tempValue);
			else if (mode == 2) dateTimeInfo[editIndex].countDown = tempValue;
			editIndex = (editIndex + 1) % 7;
			tempValue = dateTimeInfo[editIndex].stopValue;
			lcd_Clear(BLACK);
		}
    	break;
    default:
    	break;
    }
}

void handleUartRXSignal(){
	if (mode!=3) return;
	if (uartFlag == 1) {
		tempValue = 0;
		while (readIndex < writeIndex) {
			if (ringBuffer[readIndex] < 0 || ringBuffer[readIndex] > 9) {
				isWrongInput = 1;
			}
			tempValue = tempValue*10 + ringBuffer[readIndex];
			readIndex = (readIndex + 1) % BUFFER_SIZE;
		}
		if (tempValue < dateTimeInfo[editIndex].limitStart || tempValue > dateTimeInfo[editIndex].limitEnd) {
			isWrongInput = 1;
		}
		readIndex = writeIndex;
		uartFlag = 0;
		if(!isWrongInput){
			resetValues();
			setTimer(5000, 3);
			ds3231_Write(dateTimeInfo[editIndex].writeName, tempValue);
			editIndex = (editIndex + 1) % 7;
			tempValue = *(dateTimeInfo[editIndex].showName);
			isSendUartSignal = 1;
			uart_Rs232SendString("\n");
			lcd_Clear(BLACK);
		}
	}
	return;
}

void handleUartTXSignal() {
	if (mode != 3) return;
	if (flag_timer[3] == 1) {
		flag_timer[3] = 0;
		sendReqTimes ++;
		if (sendReqTimes >= 3) {
			isExeedReqTimes = 1;
			sendReqTimes = 0;
			lcd_Clear(BLACK);
			setTimer(5000, 4);
			return;
		}
		lcd_ShowIntNum(150, 60, sendReqTimes, 1, GREEN, BLACK, 24);
		isSendUartSignal = 1;
		setTimer(5000, 3);
		uart_Rs232SendString("\n");
	}

	if (isSendUartSignal) {
		isSendUartSignal = 0;
		uart_Rs232SendString(dateTimeInfo[editIndex].titleName);
		uart_Rs232SendString(": ");
	}
}

void displayExceedError () {
	if (mode != 3) return 0;
	if (isExeedReqTimes) {
		lcd_StrCenter(0, 2, "NO RESPONSE. RETURN IN 5S", RED, BLUE, 16, 1);
		if (flag_timer[4]) {
			flag_timer[4];
			mode = 0;
			tempValue = 0;
			isExeedReqTimes = 0;
			lcd_Clear(BLACK);
		}
		return 1;
	} else return 0;
}

void displayWrongInput () {
	if (isWrongInput) {
		isWrongInput = 0;
		sendReqTimes--;
		setTimer(5000, 3);
		uart_Rs232SendString("\n");
	}
}

void resetValues () {
	if (mode == 3) {
		isSendUartSignal = 0;
		sendReqTimes = 0;
		isExeedReqTimes = 0;
	}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "lcd.h"
#include<string.h>
#include<stdio.h>
/*------------------------------------------------------------------------*/
/*-----led-----*/
uint8_t led = 0x00;

/*-----减速变量-----*/
__IO uint32_t uwTick_Led_Point = 0;//滴答定时器
__IO uint32_t uwTick_Key_Point = 0;
__IO uint32_t uwTick_Lcd_Point = 0;

/*-----车辆信息变量-----*/
uint16_t Empty_Num = 8;
double cnbr = 3.50;
double vnbr = 2.00;
uint8_t cnbr_number = 0;
uint8_t vnbr_number = 0;
uint8_t idle_number = 8;


typedef struct
{
	uint8_t type[5];
	uint8_t id[5];
	uint8_t year_in;
	uint8_t month_in;
	uint8_t day_in;
	uint8_t hour_in;
	uint8_t min_in;
	uint8_t sec_in;
	_Bool notEmpty;
}Car_Data_Storage_Type;

Car_Data_Storage_Type Car_info[8];



/*-----TIM-----*/
_Bool Pwm_Mode = 0;//_Bool为布尔类型

/*-----lcd-----*/
uint8_t set = 0;
uint8_t lcd_chooose;
uint8_t lcdstring[21];


/*-----uart-----*/
uint8_t Uart_Rx;
uint8_t Uart_Rx_String[40];
uint16_t Uart_Point = 0;
uint8_t Uart_Tx_String[40];
/*------------------------------------------------------------------------*/
/*-----------------*/
void LED(uint8_t led);
void LED_Proc(void);
void KEY_Scan(void);
void LCD_Proc(void);
void Pwm_Proc(void);

void Uart_Proc(void);
_Bool Check(uint8_t *string);
void substr(uint8_t* d_str,uint8_t* s_str, uint8_t locate,uint8_t length);
void Uart_Error(void);
uint8_t isExist(uint8_t* str);
uint8_t findLocate(void);

void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
	LCD_Init();


  SystemClock_Config();


  MX_GPIO_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
/*------------------------------------------------------------------------*/	
	
	LED(0x00);
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
	TIM17->CCR1 = 0;
	
	HAL_UART_Receive_IT(&huart1,&Uart_Rx,1);

  while (1)
  {
		LED_Proc();
    KEY_Scan();
		LCD_Proc();
		Pwm_Proc();
		Uart_Proc();
  } 
}

/*------------------------------------------------------------------------*/
/*-----led-----*/
void LED(uint8_t led)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOC, led<<8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

void LED_Proc(void)
{
	if(uwTick - uwTick_Led_Point < 50) return;
	uwTick_Led_Point = uwTick;
	
	if(Empty_Num > 0)
		led |= 0x01;
	else
		led &= (~0x01);
	
	if(Pwm_Mode == 0)
		led &= (~0x02);
	else
		led |= 0x02;
	
	LED(led);
}


/*-----KEY-----*/
void KEY_Scan(void)
{
	if(uwTick - uwTick_Key_Point < 100) return;
	uwTick_Key_Point = uwTick;
	
	if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)) == 0)
	{
		HAL_Delay(30);
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)) == 0)
		{
			set = !set;
		}
		while(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)));
	}
	if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) == 0)
	{
		HAL_Delay(30);
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)) == 0)
		{
			if(set == 1)
			{
				cnbr += 0.50;
				vnbr += 0.50;
			}
		}
		while(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)));
	}
	if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)) == 0)
	{
		HAL_Delay(30);
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)) == 0)
		{
			if(set == 1)
			{
				cnbr -= 0.50;
				vnbr -= 0.50;
				if(cnbr < 1.50)
				{
					cnbr = 1.50;
					vnbr = 0.00;
				}
			}
		}
		while(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)));
	}
	if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) == 0)
	{
		HAL_Delay(30);
		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) == 0)
		{
			Pwm_Mode = !Pwm_Mode;
		}
		while(!(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)));
	}
}

/*------LCD-----*/
void LCD_Proc(void)
{
	if(uwTick - uwTick_Lcd_Point < 50) return;
	uwTick_Lcd_Point = uwTick;
	
	if(set == 0)
	{
		LCD_DisplayStringLine(Line1,(unsigned char *)"        Data         ");
		sprintf((char *)lcdstring, "   CNBR:%d        ",cnbr_number);
		LCD_DisplayStringLine(Line3,lcdstring);
		sprintf((char *)lcdstring, "   VNBR:%d        ",vnbr_number);
		LCD_DisplayStringLine(Line5,lcdstring);
		sprintf((char *)lcdstring, "   IDLE:%d        ",idle_number);
		LCD_DisplayStringLine(Line7,lcdstring);
	}
	else
	{
		LCD_DisplayStringLine(Line1,(unsigned char *)"        Para         ");
		sprintf((char *)lcdstring, "   CNBR:%1.2f        ",cnbr);
		LCD_DisplayStringLine(Line3,lcdstring);
		sprintf((char *)lcdstring, "   VNBR:%1.2f        ",vnbr);
		LCD_DisplayStringLine(Line5,lcdstring);
		LCD_ClearLine(Line7);
	}
}

/*-----UART-----*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Uart_Rx_String[Uart_Point++] = Uart_Rx;
	HAL_UART_Receive_IT(&huart1,&Uart_Rx,1);
}

/*-----pwm-----*/
void Pwm_Proc(void)
{
	if(Pwm_Mode == 0)
		TIM17->CCR1 = 0;
	else
		TIM17->CCR1 =200;
}

void Uart_Proc(void)
{
	if(Check(Uart_Rx_String))
	{
		uint8_t car_id[5];
		uint8_t car_type[5];
		uint8_t car_year,car_month, car_day, car_hour, car_min, car_sec;
		
		car_year = (((Uart_Rx_String[10] - '0')*10) + (Uart_Rx_String[11] - '0'));
		car_month = (((Uart_Rx_String[12] - '0')*10) + (Uart_Rx_String[13] - '0'));
		car_day = (((Uart_Rx_String[14] - '0')*10) + (Uart_Rx_String[15] - '0'));
		car_hour = (((Uart_Rx_String[16] - '0')*10) + (Uart_Rx_String[17] - '0'));
		car_min = (((Uart_Rx_String[18] - '0')*10) + (Uart_Rx_String[19] - '0'));
		car_sec = (((Uart_Rx_String[20] - '0')*10) + (Uart_Rx_String[21] - '0'));
		substr(car_id, Uart_Rx_String, 5, 4);
		substr(car_type, Uart_Rx_String, 0, 4);
		
		if((car_month > 12)||(car_day > 30)||(car_hour > 23)||(car_min > 59)||(car_sec > 59))
		{
			goto SEND_ERROR;
		}
		
		if(isExist(car_id) == 0xff)
		{
			uint8_t locate = findLocate();
			
			if(locate == 0xff)
			{
				goto SEND_ERROR;
			}
			substr(Car_info[locate].type,car_type, 0, 4);
			substr(Car_info[locate].id,car_id, 5, 4);
			Car_info[locate].year_in = car_year;
			Car_info[locate].month_in = car_month;
			Car_info[locate].day_in = car_day;
			Car_info[locate].hour_in = car_hour;
			Car_info[locate].min_in = car_min;
			Car_info[locate].sec_in = car_sec;
			Car_info[locate].notEmpty = 1;
			
			if(Car_info[locate].type[0] == 'C')
				cnbr_number++;
			else if(Car_info[locate].type[0] == 'V')
				vnbr_number++;
			else
				idle_number--;
		}
		else if(isExist(car_id) != 0xff)
		{
			uint64_t Second_darte;
			uint8_t in_locate = isExist(car_id);
			
			if(strcmp((const char *)car_type,(const char *)Car_info[in_locate].type) != 0)
			{
				goto SEND_ERROR;
			}
			Second_darte = (car_year - Car_info[in_locate].year_in)*365*24*60*60+(car_month-Car_info[in_locate].month_in)*30*24*60*60+\
			(car_day - Car_info[in_locate].day_in)*24*60*60+(car_hour - Car_info[in_locate].hour_in)*60*60+\
			(car_min - Car_info[in_locate].min_in)*60+(car_sec - Car_info[in_locate].sec_in);
			if(Second_darte < 0)
			{
				goto SEND_ERROR;
			}
			Second_darte = (Second_darte + 3599)/3600;
			
			sprintf((char*)Uart_Tx_String, "%s:%s:%d:%.2f\r\n",Car_info[in_locate].type,Car_info[in_locate].id,(unsigned int)Second_darte,((Car_info[in_locate].type[0] == 'C')?cnbr:vnbr)*Second_darte);
			HAL_UART_Transmit(&huart1,Uart_Tx_String, strlen((const char*)Uart_Tx_String),50);
			
			if(Car_info[in_locate].type[0] == 'C')
				cnbr_number--;
			else if(Car_info[in_locate].type[0] == 'V')
				vnbr_number--;
			else
				idle_number++;
			
			memset(&Car_info[in_locate], 0,sizeof(Car_info[in_locate]));
		}
		goto CMD_YES;
		
		SEND_ERROR:
		Uart_Error();
		
		CMD_YES:
		memset(&Uart_Rx_String[0],0,sizeof(Uart_Rx_String));
		Uart_Point = 0;
	}
}

_Bool Check(uint8_t *string)
{
	if(Uart_Point != 22)
		return 0;
	if(((string[0] == 'C')||(string[0] == 'V'))&&(string[1] == 'N')&&(string[2] == 'B')&&(string[3] == 'R')&&(string[4] == ':')&&(string[9] == ':'))
	{
		uint8_t i;
		for(i=10;i<22;i++)
		{
			if((string[i]>'9')||(string[i]<'0'))
			{
				Uart_Error();
				Uart_Point = 0;
				return 0;
			}
			return 1;
		}
	}
	else
	{
		Uart_Error();
		Uart_Point = 0;
		return 0;
	}
}

void substr(uint8_t* d_str,uint8_t* s_str, uint8_t locate,uint8_t length)
{
	uint8_t i;
	for(i=0;i<length;i++)
	{
		d_str[i] = s_str[locate+i];
	}
	d_str[length] = '\0';
}

/*---判断车id是否在库----*/
uint8_t isExist(uint8_t *str)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if(strcmp((const char*)str,(const char*)Car_info[i].id) == 0)
		{
			return i;
		}
	}
	return 0xff;
}

uint8_t findLocate(void)
{
	uint8_t i = 0;
	for(i=0;i<8;i++)
	{
		if(Car_info[i].notEmpty == 0)
			return i;
	}
	return 0xff;
}

void Uart_Error(void)
{
	sprintf((char*)Uart_Tx_String,"Error\r\n");
	HAL_UART_Transmit(&huart1,Uart_Tx_String,strlen((const char*)Uart_Rx_String),50);
}


















void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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


#include "reg52.h"
#include "iic.h"
#include "intrins.h"

unsigned char code smg_duan[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
unsigned char state = 0;
unsigned char smg_state = 0;
unsigned char DAC_state = 0;
unsigned char LED_state = 0;
unsigned int ADC;
unsigned int ADC_v;
long int count_f;
long int dat_f;
unsigned char count_t;

sbit S7 = P3^0;
sbit S6 = P3^1;
sbit S5 = P3^2;
sbit S4 = P3^3;

void LED3();
void LED4();

//****************显示相关函数************************
void Delay100us(unsigned char n)		//@12.000MHz
{
	unsigned char i, j;
    for( ;n>0;n--)
	{
	i = 2;
	j = 39;
	do
	{
		while (--j);
	} while (--i);
	}
}

void Init_138(unsigned char n)
{
	switch(n)
	{
		case 4 : P2 = (P2 & 0x1f) | 0x80; break ;
		case 5 : P2 = (P2 & 0x1f) | 0xa0; break ;
		case 6 : P2 = (P2 & 0x1f) | 0xc0; break ;
		case 7 : P2 = (P2 & 0x1f) | 0xe0; break ;
	}
}

void SMG_Display(unsigned char dat,unsigned char pos)
{
	Init_138(6);
	P0 = 0x01 << pos;
	Init_138(7);
	P0 = dat;
	Delay100us(20);
}

void OFF_SMG_Display()
{
	Init_138(6);
	P0 = 0xff;
	Init_138(7);
	P0 = 0xff;
	Delay100us(20);
}
//**************************************************

//*****************PCF8591相关函数******************
void Read_PCF8591()         //读取DAC数据
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x43);
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	ADC = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
}

void DAC_PCF8591(unsigned int dat)   //将数据写入，输出电压值
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x40);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

//**************************************************

//*********************数码管显示*******************
void ADC_Display()
{
	
	Read_PCF8591();
	
	ADC_v = ADC * 1.961;
	if(smg_state == 0)
	{
	SMG_Display(0xc1,0);             //1100 0001
	SMG_Display(smg_duan[ADC_v /100] & 0x7f,5);
	SMG_Display(smg_duan[ADC_v %100 /10],6);
	SMG_Display(smg_duan[ADC_v % 10],7);
	SMG_Display(0xff,7);
	}
	else
	{
		OFF_SMG_Display();
	}
}

void Fre_Display()
{
	switch(smg_state)
	{
		case 0 :
		{
			SMG_Display(0x8e,0);             //1000 1110
			if(dat_f > 99999)
			{
			SMG_Display(smg_duan[dat_f /100000],2);
			}
			if(dat_f > 9999)
			{
				SMG_Display(smg_duan[dat_f %100000 /10000],3);
			}
			if(dat_f > 999)
			{
				SMG_Display(smg_duan[dat_f %10000 /1000],4);
			}
			if(dat_f > 99)
			{
				SMG_Display(smg_duan[dat_f %1000 /100],5);
			}
			if(dat_f > 9)
			{
				SMG_Display(smg_duan[dat_f %100 /10],6);
			}
			SMG_Display(smg_duan[dat_f % 10],7);  
			SMG_Display(0xff,7);
		}
		break;
		
		case 1 :
			OFF_SMG_Display();
		break;
	}
		
}

//**************************************************

//*****************定时器初始化中断函数*************
void Init_Time()
{
	TMOD = 0x16;   //T0作为计数 8位自动重装 T1作为计时 16位  0001 0110
	
	TH0 = 0xff;
	TL0 = 0xff;
	
	TH1 = (65535 - 50000)/256;
	TL1 = (65535 - 50000)%256;
	
	EA = 1;
	ET1 = 1;
	ET0 = 1;
	TR1 = 1;
	TR0 = 1;
}

void Service_Time0() interrupt 1       
{
	count_f = count_f + 1;
}

void Service_Time1() interrupt 3
{
	TH1 = (65535 - 50000)/256;
	TL1 = (65535 - 50000)%256;
	count_t++;
	if(count_t == 10)
	{
		dat_f = count_f * 2;
		count_f = 0;
		count_t = 0;
	}
}

//**************************************************


void key()
{
	if(S7 == 0)
	{
		Delay100us(10);
		if(S7 == 0)        //S7 按键 数码管显示功能控制
		{
			smg_state++;
			if(smg_state == 2)
			{
				smg_state = 0;
			}
			while(S7 == 0)
			{
				if(smg_state == 1){OFF_SMG_Display();}
			}
		}
	}
	if(S6 == 0)
	{
		Delay100us(10);
		if(S6 == 0)        //S6 按键 LED指示灯功能控制
		{
			LED_state++;
			if(LED_state == 2)
			{
				LED_state = 0;
			}
			while(S6 == 0)
			{
				if(state == 1){Fre_Display();}
				if(state == 0){ADC_Display();}
			}
		}
	}
	if(S5 == 0)
	{
		Delay100us(10);
		if(S5 == 0)        //S5 按键 输出模式切换
		{
			DAC_state++;
			if(DAC_state == 2)
			{
				DAC_state = 0;
			}
			while(S5 == 0)
			{
				if(state == 0){ADC_Display();}
				if(state == 1){Fre_Display();}
			}
		}
	}
	if(S4 == 0)
	{
		Delay100us(10);
		if(S4 == 0)        //S4 按键  显示界面切换
		{
			state++;
			if(state == 2)
			{
				state = 0;
			}
			while(S4 == 0)
			{
				if(state == 1){ADC_Display();}
				if(state == 0){Fre_Display();}
			}
		}
	}
}

void Output_DAC_PCF8591()
{
	switch(DAC_state)
	{
		case 0 :
		{
			DAC_PCF8591(200*0.51);
		}
		break;
		
		case 1 :
		{
			Read_PCF8591();DAC_PCF8591(ADC);
		}
		break;
	}
}

void LedDisplay()
{
	switch(LED_state)
	{
		case 0 :
		{
			if(state == 0)
			{
				P0 = 0xfe;  //1111 1110
				LED3();
			}
			else
			{
				P0 = 0xfd;
				LED4();
			}
			
			if(DAC_state == 0)
			{
				P0 |= 0x10;     //0001 0000
				P0 &= 0xfb;     //1111 1011
			}
			else
			{
				P0 &= 0xef;
			}
			
			Init_138(4);
		}
		break;
		
		case 1 :
		{
			Init_138(4);
			P0 = 0xff;
		}
		break;
	}
}

void LED3()
{
	if(ADC_v < 150)
		P0 |= 0x04;   // 0000 0100
	else if(ADC_v < 250)
		P0 &= 0xfb;   // 1111 1011
	else if(ADC_v < 350)
		P0 |= 0x04;   // 0000 0100
	else
		P0 &= 0xfb;   // 1111 1011
}

void LED4()
{
	if(dat_f < 1000)
		P0 |= 0x08;   // 0000 1000
	else if(dat_f < 5000)
		P0 &= 0xf7;   // 1111 0111
	else if(dat_f < 10000)
		P0 |= 0x08;   // 0000 1000
	else
		P0 &= 0xf7;   // 1111 0111
}

void Init()
{
	Init_138(5);
	P0 = 0x00;
}

void main()
{
	Init();
	Init_Time();
	while(1)
	{
		switch(state)
		{
			case 0 :
				ADC_Display();LedDisplay();Output_DAC_PCF8591();key();
			break;
			
			case 1 :
				Fre_Display();LedDisplay();Output_DAC_PCF8591();key();
			break;
		}
	}
}
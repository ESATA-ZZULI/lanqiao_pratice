#include "reg52.h"
#include "iic.h"
#include "intrins.h"

unsigned char code smg_duan[10] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
unsigned int num ;
unsigned int num1 ;
unsigned char count ;
short int canshu = 0;
unsigned int jishu = 0;
unsigned char a;   //用于判断是否满足计数
unsigned char b;   //用来判断L1灯是否点亮

//public enum Num {x1 = 1,x2,x3}x;  哈哈，原来这就是枚举  

sbit R1 = P3^2;
sbit R2 = P3^3;

sbit M1 = P3^5;
sbit M2 = P3^4;

sbit L1 = P0^0;
sbit L2 = P0^1;

void Dat_arrange();
void SMG_Display(unsigned char dat,unsigned char pos);
void Key();
void Canshu_Display();
void jishu_Display();
void Voltage_Display();

//***************************************************
void Delay100us(unsigned int n)		//在11.0592MHz下的延时函数
{
	unsigned char i, j;
    for( ; n>0 ;n--)
	{
	_nop_();
	_nop_();
	i = 2;
	j = 15;
	do
	{
		while (--j);
	} while (--i);
	}
}

void Init_138(unsigned char n)    //锁存器选择
{
	switch(n)
	{
		case 4 : P2 = (P2 & 0x1f) | 0x80 ;break;
		case 5 : P2 = (P2 & 0x1f) | 0xa0 ;break;
		case 6 : P2 = (P2 & 0x1f) | 0xc0 ;break;
		case 7 : P2 = (P2 & 0x1f) | 0xe0 ;break;
		case 0 : P2 = (P2 & 0x1f) | 0x00 ;break;
	}
}

void SMG_Display(unsigned char dat,unsigned char pos)  //数码管基本显示函数
{
	Init_138(6);
	P0 = 0x01 << pos;
	Init_138(7);
	P0 = dat;
	
	Delay100us(10);
}

//***************************************************

//******************PCF8591相关函数******************
void Init_PCF8591()   //初始化PCF8591函数
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x03);
	IIC_WaitAck();
	IIC_Stop();
}

void Read_PCF8591()    //读取PCF8591的数值
{
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	num = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
}

//***************************************************

//*****************EEPROM相关函数********************
void Write_EEPROM(unsigned int dat,unsigned char addr)    //数据写入EEPROM函数
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

void Read_EEPROM(unsigned char addr)      //数据从EEPROM中读取函数
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	canshu = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
}

//***************************************************

void Key()        //按键扫描函数，这里只使用了四个按键
{
	R1 = 0;
	R2 = 1 ;
	M1 = M2 =1 ;
	if(M1 == 0)
	{
		Delay100us(10);
		if(M1 == 0 )     //S13按键 清零
		{
			jishu = 0;	
		}
	}
	if(M2 == 0)
	{
		Delay100us(10);
		if(M2 == 0 )      //S17按键 减
		{
			if(count == 1)     //只有在参数页面才能触发
			{
			canshu = canshu - 50;
			if(canshu == -50)
			{
				canshu = 500;
			}
			while(M2 == 0)
			{
				Canshu_Display();
			}
			
			}
		}
	}
	R1 = 1;
	R2 = 0 ;
	M1 = M2 =1 ;
	if(M1 == 0)
	{
		Delay100us(10);
		if(M1 == 0 )     //S12按键 显示页面切换
		{
			count = count + 1;
			if(count == 3)
			{	
				count = 0;
			}
			while(M1 == 0)
			{
				if(count == 1){Voltage_Display();}
				if(count == 2){Canshu_Display();}
				if(count == 0){jishu_Display();}
			}
		}
	}
	if(M2 == 0)
	{
		Delay100us(10);
		if(M2 == 0 )      //S16按键 加
		{
			if(count == 1)    //只有在参数页面才能触发
			{
			canshu = canshu + 50 ;
			if(canshu == 550 )
			{
				canshu = 0;
			}
			while(M2 == 0)
			{
				Canshu_Display();
			}
		    }
		}
	}
}

void Dat_arrange()      //数据处理，将其转化为电压值
{
	num1 = num * 1.961;    //  500 / 255 = 1.961   将数据扩大100倍 
}

void Voltage_Display()               //电压显示
{
	Read_PCF8591();
	Dat_arrange();
	SMG_Display(0xc1,0);   //1100 0001
	SMG_Display(smg_duan[num1 / 100]& 0x7f,5);
	SMG_Display(smg_duan[num1 % 100 / 10] ,6);
	SMG_Display(smg_duan[num1 % 10],7);
	SMG_Display(0xff,7);
}

void Canshu_Display()                 //参数显示
{
	SMG_Display(0x8c,0);   //    1000 1100
	SMG_Display(smg_duan[canshu / 100]& 0x7f,5);
	SMG_Display(smg_duan[canshu % 100 / 10] ,6);
	SMG_Display(smg_duan[canshu % 10],7);
	SMG_Display(0xff,7);
}

void jishu_Display()                   //计数显示
{
	SMG_Display(0xc8,0);   //    1100 1000  
	SMG_Display(smg_duan[jishu / 100],5);
	SMG_Display(smg_duan[jishu % 100 / 10] ,6);
	SMG_Display(smg_duan[jishu % 10],7);
	SMG_Display(0xff,7);
	
}

void Init_Read_canshu()      //参数初始化读取
{
	Read_EEPROM(0);
	canshu = canshu * 10;
}

void count_judgment()        //判断函数
{
	if(num1 >= canshu)
	{
		a = 1;
	}
	if(a == 1)
	{
		if(num1 < canshu)
		{
			jishu = jishu + 1;
			a = 0;
		}
	}
	if(jishu % 2 == 0)
	{
		Init_138(4);
		L2 = 1;     
	}
	if(jishu % 2 == 1)
	{
		Init_138(4);
		L2 = 0;
	}
	if(b == 1)
	{
		Init_138(4);
		L1 = 0;
	}
	if(b == 0)
	{
		Init_138(4);
		L1 = 1;
	}
}


//*******************定时器函数**********************
void Init_Time0()
{
	TMOD = 0X01;
	TH0 = (65536-50000)/256;
	TL0 = (65536-50000)%256;
	
	ET0 = 1;
	EA  = 1;
	TR0 = 1;
}

void Serive_Time0() interrupt 1
{
	static unsigned int i = 0;
	
	TH0 = (65535 - 50000)/256;
    TL0 = (65535 - 50000)%256;
	
	if(num1 < canshu)
	i++;
	else
	{
		b = 0;   //L1 熄灭
		i = 0;
	} 
	if(i == 100)
	{
		b = 1;  //L1 点亮
		i = 0;
	}
}

//***************************************************

void main()
{
	Init_138(5);
	P0 = 0x00;
	Init_138(4);
	P0 = 0xff;
	Init_PCF8591();
	Init_Read_canshu();
	Init_Time0();
	while(1)
	{
		switch(count)
		{
			case 0 : Key();
			         Read_PCF8591();
	                 Dat_arrange();
			         count_judgment();
			         Voltage_Display();
			break;
			case 1 : Key();
			         Read_PCF8591();
	                 Dat_arrange();
			         count_judgment();
			         Canshu_Display() ; 
			         Write_EEPROM(canshu * 0.1,0);//参数扩大10倍，由于原参数已扩大100倍，所以这里乘于0.1
			break;   
			          
			case 2 : Key();
			         Read_PCF8591();
	                 Dat_arrange();
			         count_judgment();
			         jishu_Display()  ;
			break;
		}
	}
}


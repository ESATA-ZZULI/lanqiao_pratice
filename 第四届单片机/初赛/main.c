#include <STC15F2K60S2.H>
#include <ds1302.H>
#include <iic.H>

typedef unsigned char uchar;
typedef unsigned int uint;
	
sbit L1=P0^0;
sbit L2=P0^1;
sbit S7=P3^0;
sbit S6=P3^1;
sbit S5=P3^2;
sbit S4=P3^3;

uchar code dpnum[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0xbf};
uchar code dpdotnum[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
uchar code Time_Write[]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
uchar code Time_Read[]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
uchar code Time_Init[]={0x00,0x30,0x08,0x24,0x03,0x04,0x22};
uchar Time_temp[7]={0};

uchar humth=50; //湿度阈值
uchar Dout=0;
uchar Vout_hum=0;
uchar S7state=1;//默认自动工作模式
uchar S6state1=2;
uchar S6state2=1;

void System_Init();
void DS1302_Init();
void DS1302_Read();
void delayms(uchar ms);
void display_bit(uchar pos,uchar dat);
void display();
uchar PCF8591_Read(uchar add);
void JDQ_working();
void KeyS7();
void key_working();
void AT24C02_Write(uchar add,uchar dat);
uchar AT24C02_Read(uchar add);
void FMQ_Working();
void S6state2_Working();

void main()
{
	System_Init();
	AT24C02_Write(0x01,50);
	DS1302_Init();
	while(1)
	{
		DS1302_Read();
		Dout=PCF8591_Read(0x03);
		Vout_hum=Dout*99/255;
		//display();
		KeyS7();
		key_working();
	}
}

void System_Init()
{
	P2 = (P2 & 0x1f) | 0x80;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xa0;
	P0 = 0x00;
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}
void DS1302_Init()
{
	uchar i;
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(Time_Write[i],Time_Init[i]);
	}
}
void DS1302_Read()
{
	uchar i;
	for(i=0;i<7;i++)
	{
		Time_temp[i]=Read_Ds1302_Byte(Time_Read[i]);
	}
}
void delayms(uchar ms)
{
	uint i,j;
	for(i=ms;i>0;i--)
		for(j=810;j>0;j--);
}
void display_bit(uchar pos,uchar dat)
{
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0x01 << pos;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = dat;
}
void display()
{
	display_bit(0,dpnum[Time_temp[2]/16]);
	delayms(1);
	display_bit(1,dpnum[Time_temp[2]%16]);
	delayms(1);
	display_bit(2,dpnum[11]);
	delayms(1);
	display_bit(3,dpnum[Time_temp[1]/16]);
	delayms(1);
	display_bit(4,dpnum[Time_temp[1]%16]);
	delayms(1);
	display_bit(5,dpnum[10]);
	delayms(1);
	display_bit(6,dpnum[Vout_hum/10]);
	delayms(1);
	display_bit(7,dpnum[Vout_hum%10]);
	delayms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}
uchar PCF8591_Read(uchar add)
{
	uchar temp;
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	
	IIC_SendByte(add);
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	
	temp=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	return temp;
}
void JDQ_working()
{
	humth=AT24C02_Read(0x01);
	if(Vout_hum<humth)
	{
		P2 = (P2 & 0x1f) | 0xa0;
		P0 = 0x10;
	}
	else if(Vout_hum>=humth)
	{
		P2 = (P2 & 0x1f) | 0xa0;
		P0 = 0x00;
	}
}
void KeyS7()
{
	if(S7==0)
	{
		delayms(5);
		if(S7==0)
		{
			if(S7state==1)
				S7state=2; //1为自动工作模式，2为手动工作模式
			else if(S7state==2)
				S7state=1;
			while(S7==0);
		}
	}
	switch(S7state)
	{
		case 1:
			P2 = (P2 & 0x1f) | 0x80;
			P0 = 0xfe;
			if(S6==0)
			{
				delayms(3);
				if(S6==0)
				{
					if(S6state1==1)
						S6state1=2; //自动工作模式下，退出设置阈值界面
					else if(S6state1==2)
						S6state1=1; //自动工作模式下，进入设置阈值界面
					while(S6==0);
				}
			}
			JDQ_working();
			break;
		case 2:
			P2 = (P2 & 0x1f) | 0x80;
			//L2=0;
		  P0 &=0xfd;
//			S6state2_Working();
//			P2 = (P2 & 0x1f) | 0x00;
//			P0 = 0x00;
//			P2 = (P2 & 0x1f) | 0xa0;
//			P0 &= 0xbF;
			if(S6==0)
			{
				delayms(3);
				if(S6==0)
				{
					if(S6state2==1)
						S6state2=2;  //手动模式下，关闭蜂鸣器提醒
					else if(S6state2==2)
						S6state2=1;  //手动模式下，打开蜂鸣器提醒
					while(S6==0);  
				}
			}
			else if(S5==0)
			{
				delayms(5);//加上延时之后继电器会叫，真的叫
				if(S5==0)
				{
					P2 = (P2 & 0x1f) | 0xa0;
					P0 |= 0x10;
				}
			}
			else if(S4==0)
			{
				//delayms(5);
				if(S4==0)
				{
					P2 = (P2 & 0x1f) | 0xa0;
					P0 &= 0xef;
				}
			}
			break;
	}	
}
void S6state2_Working()
{
	switch(S6state2)
	{
		case 1:
			FMQ_Working();
			break;
		case 2:
			P2 = (P2 & 0x1f) | 0xa0;
			P0 &= 0xbF;
			break;
	}
}
void key_working()
{
	humth=AT24C02_Read(0x01);
	switch(S6state1)
	{
		case 1:
			if(S5==0)
			{
				delayms(5);
				if(S5==0)
				{
					humth++;
					while(S5==0);
				}
			}
			else if(S4==0)
			{
				delayms(5);
				if(S4==0)
				{
					humth--;
					while(S4==0);
				}
			}
			if(humth>99)
				humth=0;
			AT24C02_Write(0x01,humth);
			display_bit(0,dpnum[11]);
			delayms(1);
			display_bit(1,dpnum[11]);
			delayms(1);
			display_bit(2,dpnum[10]);
			delayms(1);
			display_bit(3,dpnum[10]);
			delayms(1);
			display_bit(4,dpnum[10]);
			delayms(1);
			display_bit(5,dpnum[10]);
			delayms(1);
			display_bit(6,dpnum[humth/10]);
			delayms(1);
			display_bit(7,dpnum[humth%10]);
			delayms(1);
			P2 = (P2 & 0x1f) | 0xc0;
			P0 = 0xff;
			P2 = (P2 & 0x1f) | 0xe0;
			P0 = 0xff;
			break;
		case 2:
			AT24C02_Write(0x01,humth);
			display();
			break;
	}
}
void AT24C02_Write(uchar add,uchar dat)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	
	IIC_SendByte(add);
	IIC_WaitAck();
	
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}
uchar AT24C02_Read(uchar add)
{
	uchar temp;
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	
	IIC_SendByte(add);
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	
	temp=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	return temp;
}
void FMQ_Working()
{
	humth=AT24C02_Read(0x01);
	if(Vout_hum<humth)
	{
		P2 = (P2 & 0x1f) | 0xa0;
		P0 |= 0x40; //打开蜂鸣器
	}
	else if(Vout_hum>=humth)
	{
		P2 = (P2 & 0x1f) | 0xa0;
		P0 &= 0xbF;
	}
}
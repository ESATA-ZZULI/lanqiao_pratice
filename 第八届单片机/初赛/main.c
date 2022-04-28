#include <STC15F2K60S2.H>
#include "onewire.h"
#include "ds1302.h"

typedef unsigned char uchar;
typedef unsigned int uint;
	
sbit S7 = P3^0;
sbit S6 = P3^1;
sbit S5 = P3^2;
sbit S4 = P3^3;

uchar code dpnum[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0xb6};
uchar code DS1302_Writeadd[]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
uchar code DS1302_Readadd[]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
uchar code Time_init_array[7]={0x50,0x59,0x23,0x06,0x04,0x03,0x22};
uchar Time_save_array[7]={0};
uchar naozhong_set[3]={0x00,0x00,0x00};
uchar Interface=1;
uchar count_t;
bit LED_flag=0;
bit smg_flag=0;
uchar S7_state=3;
uchar S6_state=3;

void System_Init();
void DS1302_Init();
void DS1302_Read();
void delayms(uchar ms);
void display_bit(uchar pos,uchar dat);
void Face();
void Timer0_Init();
void smg_shamshuo();
void key();
void smg_display_all();
void naozhong_display_all();
void naozhong_shanshuo();

void main()
{
	System_Init();
	Timer0_Init();
	DS1302_Init();
	while(1)
	{
		DS1302_Read();
		key();
		Face();
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
	Write_Ds1302_Byte(0x8e,0x00);//允许向寄存器写入数据
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(DS1302_Writeadd[i],Time_init_array[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);//禁止向寄存器写入数据
}
void DS1302_Read()
{
	uchar i;
	for(i=0;i<7;i++)
	{
		Time_save_array[i]=Read_Ds1302_Byte(DS1302_Readadd[i]);
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
void Face()
{
	switch(Interface)
	{
		case 1:
			smg_shamshuo();
			break;
		case 2:
			naozhong_shanshuo();
			break;
		case 3:
			break;
	}
}
void Timer0_Init()
{
	TMOD |= 0X00;
	TH0 = (65536-10000)/256;
	TL0 = (65536-10000)%256;
	TR0 = 1;
	ET0 = 1;
	EA = 1;
}
void Timer0() interrupt 1
{
	count_t++;
	if(count_t%10==0)
	{
		LED_flag=~LED_flag;
	}
	if(count_t==50)
	{
		count_t=0;
		smg_flag=~smg_flag;
	}
}
void smg_shamshuo()
{
	if(smg_flag==0)
	{
		switch(S7_state)
		{
			case 0:
				display_bit(0,dpnum[11]);
				delayms(1);
				display_bit(1,dpnum[11]);
				delayms(1);
				display_bit(2,dpnum[10]);
				delayms(1);
				display_bit(3,dpnum[Time_save_array[1]/16]);
				delayms(1);
				display_bit(4,dpnum[Time_save_array[1]%16]);
				delayms(1);
				display_bit(5,dpnum[10]);
				delayms(1);
				display_bit(6,dpnum[Time_save_array[0]/16]);
				delayms(1);
				display_bit(7,dpnum[Time_save_array[0]%16]);
				delayms(1);
				P2 = (P2 & 0x1f) | 0xc0;
				P0 = 0xff;
				P2 = (P2 & 0x1f) | 0xe0;
				P0 = 0xff;
				break;
			case 1:
				display_bit(0,dpnum[Time_save_array[2]/16]);
				delayms(1);
				display_bit(1,dpnum[Time_save_array[2]%16]);
				delayms(1);
				display_bit(2,dpnum[10]);
				delayms(1);
				display_bit(3,dpnum[11]);
				delayms(1);
				display_bit(4,dpnum[11]);
				delayms(1);
				display_bit(5,dpnum[10]);
				delayms(1);
				display_bit(6,dpnum[Time_save_array[0]/16]);
				delayms(1);
				display_bit(7,dpnum[Time_save_array[0]%16]);
				delayms(1);
				P2 = (P2 & 0x1f) | 0xc0;
				P0 = 0xff;
				P2 = (P2 & 0x1f) | 0xe0;
				P0 = 0xff;
				break;
			case 2:
				display_bit(0,dpnum[Time_save_array[2]/16]);
				delayms(1);
				display_bit(1,dpnum[Time_save_array[2]%16]);
				delayms(1);
				display_bit(2,dpnum[10]);
				delayms(1);
				display_bit(3,dpnum[Time_save_array[1]/16]);
				delayms(1);
				display_bit(4,dpnum[Time_save_array[1]%16]);
				delayms(1);
				display_bit(5,dpnum[10]);
				delayms(1);
				display_bit(6,dpnum[11]);
				delayms(1);
				display_bit(7,dpnum[11]);
				delayms(1);
				P2 = (P2 & 0x1f) | 0xc0;
				P0 = 0xff;
				P2 = (P2 & 0x1f) | 0xe0;
				P0 = 0xff;
				break;
			case 3:
				smg_display_all();
				break;
		}
	}
	else
	{
		smg_display_all();
	}
}
void smg_display_all()
{
	display_bit(0,dpnum[Time_save_array[2]/16]);
	delayms(1);
	display_bit(1,dpnum[Time_save_array[2]%16]);
	delayms(1);
	display_bit(2,dpnum[10]);
	delayms(1);
	display_bit(3,dpnum[Time_save_array[1]/16]);
	delayms(1);
	display_bit(4,dpnum[Time_save_array[1]%16]);
	delayms(1);
	display_bit(5,dpnum[10]);
	delayms(1);
	display_bit(6,dpnum[Time_save_array[0]/16]);
	delayms(1);
	display_bit(7,dpnum[Time_save_array[0]%16]);
	delayms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}
void key()
{
	if(S7==0)
	{
		delayms(5);
		if(S7==0)
		{
			Interface=1;
			S7_state++;
			if(S7_state>3)
				S7_state=0;
			while(S7==0);
		}
	}
	else if(S6==0)
	{
		delayms(5);
		if(S6==0)
		{
			Interface=2;
			S6_state++;
			if(S6_state>3)
				S6_state=0;
			while(S6==0);
		}
	}
	else if(S5==0)
	{
		delayms(5);
		if(S5==0)
		{
			if(Interface==2)
			{
				switch(S6_state)
				{
					case 0:
						naozhong_set[2]++;
						if(naozhong_set[2]>23)
							naozhong_set[2]=0;
						break;
					case 1:
						naozhong_set[1]++;
						if(naozhong_set[1]>59)
							naozhong_set[1]=0;
						break;
					case 2:
						naozhong_set[0]++;
						if(naozhong_set[0]>59)
							naozhong_set[0]=0;
						break;
				}
			}
			while(S5==0);
		}
//	else if(S4==0)
//	{
//		delayms(5);
//		if(S4==0)
//		{
//			if(Interface==2)
//			{
//				switch(S6_state)
//				{
//					case 0:
//						naozhong_set[2]--;
//						if(naozhong_set[2]<0)
//							naozhong_set[2]=23;
//						break;
//					case 1:
//						naozhong_set[1]--;
//						if(naozhong_set[1]<0)
//							naozhong_set[1]=59;
//						break;
//					case 2:
//						naozhong_set[0]--;
//						if(naozhong_set[0]<0)
//							naozhong_set[0]=59;
//						break;
//				}
//			}
//			while(S4==0);
//		}
//	}
}
void naozhong_shanshuo()
{
	if(smg_flag==0)
	{
		switch(S6_state)
		{
			case 0:
				display_bit(0,dpnum[11]);
				delayms(1);
				display_bit(1,dpnum[11]);
				delayms(1);
				display_bit(2,dpnum[10]);
				delayms(1);
				display_bit(3,dpnum[naozhong_set[1]/10]);
				delayms(1);
				display_bit(4,dpnum[naozhong_set[1]%10]);
				delayms(1);
				display_bit(5,dpnum[10]);
				delayms(1);
				display_bit(6,dpnum[naozhong_set[0]/10]);
				delayms(1);
				display_bit(7,dpnum[naozhong_set[0]%10]);
				delayms(1);
				P2 = (P2 & 0x1f) | 0xc0;
				P0 = 0xff;
				P2 = (P2 & 0x1f) | 0xe0;
				P0 = 0xff;
				break;
			case 1:
				display_bit(0,dpnum[naozhong_set[2]/10]);
				delayms(1);
				display_bit(1,dpnum[naozhong_set[2]%10]);
				delayms(1);
				display_bit(2,dpnum[10]);
				delayms(1);
				display_bit(3,dpnum[11]);
				delayms(1);
				display_bit(4,dpnum[11]);
				delayms(1);
				display_bit(5,dpnum[10]);
				delayms(1);
				display_bit(6,dpnum[naozhong_set[0]/10]);
				delayms(1);
				display_bit(7,dpnum[naozhong_set[0]%10]);
				delayms(1);
				P2 = (P2 & 0x1f) | 0xc0;
				P0 = 0xff;
				P2 = (P2 & 0x1f) | 0xe0;
				P0 = 0xff;
				break;
			case 2:
				display_bit(0,dpnum[naozhong_set[2]/10]);
				delayms(1);
				display_bit(1,dpnum[naozhong_set[2]%10]);
				delayms(1);
				display_bit(2,dpnum[10]);
				delayms(1);
				display_bit(3,dpnum[naozhong_set[1]/10]);
				delayms(1);
				display_bit(4,dpnum[naozhong_set[1]%10]);
				delayms(1);
				display_bit(5,dpnum[10]);
				delayms(1);
				display_bit(6,dpnum[11]);
				delayms(1);
				display_bit(7,dpnum[11]);
				delayms(1);
				P2 = (P2 & 0x1f) | 0xc0;
				P0 = 0xff;
				P2 = (P2 & 0x1f) | 0xe0;
				P0 = 0xff;
				break;
			case 3:
				naozhong_display_all();
				break;
		}
	}
	else
	{
		naozhong_display_all();
	}
}
void naozhong_display_all()
{
	display_bit(0,dpnum[naozhong_set[2]/10]);
	delayms(1);
	display_bit(1,dpnum[naozhong_set[2]%10]);
	delayms(1);
	display_bit(2,dpnum[10]);
	delayms(1);
	display_bit(3,dpnum[naozhong_set[1]/10]);
	delayms(1);
	display_bit(4,dpnum[naozhong_set[1]%10]);
	delayms(1);
	display_bit(5,dpnum[10]);
	delayms(1);
	display_bit(6,dpnum[naozhong_set[0]/10]);
	delayms(1);
	display_bit(7,dpnum[naozhong_set[0]%10]);
	delayms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}
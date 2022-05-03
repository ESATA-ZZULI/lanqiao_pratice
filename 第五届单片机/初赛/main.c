#include <STC15F2K60S2.H>
#include "onewire.h"

typedef unsigned char uchar;
typedef unsigned int uint;

sbit L1 = P0^0;
sbit L2 = P0^1;
sbit H1 = P3^0;
sbit H2 = P3^1;
sbit H3 = P3^2;
sbit H4 = P3^3;
sbit R4 = P3^4;
sbit R3 = P3^5;
sbit R2 = P4^2;
sbit R1 = P4^4;

uchar code dpnum[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
uchar Tset_Init[4]={0};
uchar Tset[4]={11,11,11,11};
uint temperature=0;
uchar Tmax_Init;
uchar Tmin_Init;
uchar Tmaxtemp=30;
uchar Tmintemp=20;
uchar Tstate;
uchar count_t=0;
uchar LEDstate;
uchar JDQstate;
uchar keyword;
bit Interface = 0;
uchar Tsetnum=0;
uchar keycount=5;
bit keystate = 0;

void System_Init();
void delayms(uchar ms);
uint DS18B20_Read();
void display_bit(uchar pos,uchar dat);
void display();
void compare();
void compare_work();
void Timer0_Init();
void key();
void Face();
void display_setup();

void main()
{
	System_Init();
	Timer0_Init();
	while(1)
	{
		temperature=DS18B20_Read();//注意修改onewire.c里面的延时时间
		compare();
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
	display_bit(0,dpnum[10]);
	delayms(1);
	display_bit(1,dpnum[Tstate]);
	delayms(1);
	display_bit(2,dpnum[10]);
	delayms(1);
	display_bit(3,dpnum[11]);
	delayms(1);
	display_bit(4,dpnum[11]);
	delayms(1);
	display_bit(5,dpnum[11]);
	delayms(1);
	display_bit(6,dpnum[temperature/10]);
	delayms(1);
	display_bit(7,dpnum[temperature%10]);
	delayms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}
uint DS18B20_Read()
{
	uchar LSB,MSB;
	uint temp;
	init_ds18b20(); //复位
	Write_DS18B20(0xcc);//跳过ROM指令
	Write_DS18B20(0x44);//开始温度转换
	
	delayms(1);
	
	init_ds18b20(); //复位
	Write_DS18B20(0xcc);//跳过ROM指令
	Write_DS18B20(0xbe);//读取高速暂存器
	
	LSB=Read_DS18B20();
	MSB=Read_DS18B20();
	init_ds18b20(); //复位
	
	temp=(MSB << 8) | LSB;
	temp >>= 4; //取整数
	
	return temp;
}
void compare()
{
	if(temperature<Tmin_Init)
		Tstate=0;
	else if(temperature>=Tmin_Init && temperature<=Tmax_Init)
		Tstate=1;
	else if(temperature>Tmax_Init)
		Tstate=2;
	compare_work();
}
void compare_work()
{
	switch(Tstate)
	{
		case 0:
			P2 = (P2 & 0x1f) | 0xa0;
			P0 = 0x00;
			break;
		case 1:
			P2 = (P2 & 0x1f) | 0xa0;
			P0 = 0x00;
			break;
		case 2:
			P2 = (P2 & 0x1f) | 0xa0;
			P0 = 0x10;
			break;
	}
}
void Timer0_Init()
{
	TMOD = 0X01;
	TH0 = (65536-10000)/256;
	TL0 = (65536-10000)%256;
	TR0 = 1;
	TF0 = 0;
	ET0 = 1;
	EA = 1;
}
//void Timer0() interrupt 1
//{
//	TH0 = (65536-10000)/256;
//	TL0 = (65536-10000)%256;
//	count_t++;
//	if(count_t==20)
//	{
//		TF0 = 1;
//		count_t=0;
//		P2 = (P2 & 0x1f) | 0x80;
//		P0 |= 0xfe;
//		L1 = ~L1;
//		LEDstate = L1;
//		TF0 = 0;
//}
//}
void Timer0() interrupt 1
{
	TH0 = (65536-10000)/256;
	TL0 = (65536-10000)%256;
	count_t++;
//	P2 = (P2 & 0x1f) | 0x80;
//	P0 |= (0xfe|LEDstate);
//	P0 |= 0xff;
//	L1 = LEDstate;
	if(count_t>40)
		count_t=0;
	switch(Tstate)
	{
		case 2:
			if(count_t==10)
			{
				TF0 = 1;
				count_t=0;
				P2 = (P2 & 0x1f) | 0x80;
				P0 |= 0xff;
				L1 = ~L1;
				LEDstate = L1;
				TF0 = 0;
			}
			break;
		case 1:
			if(count_t==20)//定时器产生时间不准确，应该是受其他地方寄存器影响,可以在期望值基础上加上50ms左右
			{
//				count_t=0;
//				L1 = ~L1;
//				LEDstate = L1;
				TF0 = 1;
				count_t=0;
				P2 = (P2 & 0x1f) | 0x80;
				P0 |= 0xfe;
				L1 = ~L1;
				LEDstate = L1;
				TF0 = 0;
			}
			break;
		case 0:
			if(count_t==40)
			{
				count_t=0;
//				TF0 = 1;
//				P2 = (P2 & 0x1f) | 0x80;
//				P0 |= 0xfe;
				L1 = ~L1;
				LEDstate = L1;
			}
			break;	
	}
//	switch(Tstate)
//	{
//		case 2:
//			if(count_t==10)
//				TF0 = 1;
//			break;
//		case 1:
//			if(count_t==20)
//				TF0 = 1;
//			break;
//		case 0:
//			if(count_t==40)
//				TF0 = 1;
//			break;	
//	}
//	if(TF0 == 1)
//	{
//		count_t=0;
////		P2 = (P2 & 0x1f) | 0x80;
////		P0 |= 0xff;
//		L1 = ~L1;
//		LEDstate = L1;
//		TF0 = 0;
//	}
}
void key()
{
	keystate = 0;
	R1 = 0;
	R2 = R3 = R4 = 1;
	H1 = H2 = H3 = H4 = 1;
	if(H1 == 0)
	{
		delayms(5);
		if(H1 == 0)
		{
			keyword=0;
			keystate = 1;
			while(H1==0);
		}
	}
	else if(H2 == 0)
	{
		delayms(5);
		if(H2 == 0)
		{
			keyword=3;
			keystate = 1;
			while(H2==0);
		}
	}
	else if(H3 == 0)
	{
		delayms(5);
		if(H3 == 0)
		{
			keyword=6;
			keystate = 1;
			while(H3==0);
		}
	}
	else if(H4 == 0)
	{
		delayms(5);
		if(H4 == 0)
		{
			keyword=9;
			keystate = 1;
			while(H4==0);
		}
	}
	R2 = 0;
	R1 = R3 = R4 = 1;
	H1 = H2 = H3 = H4 = 1;
	if(H1 == 0)
	{
		delayms(5);
		if(H1 == 0)
		{
			keyword=1;
			keystate = 1;
			while(H1==0);
		}
	}
	else if(H2 == 0)
	{
		delayms(5);
		if(H2 == 0)
		{
			keyword=4;
			keystate = 1;
			while(H2==0);
		}
	}
	else if(H3 == 0)
	{
		delayms(5);
		if(H3 == 0)
		{
			keyword=7;
			keystate = 1;
			while(H3==0);
		}
	}
	else if(H4 == 0)
	{
		delayms(5);
		if(H4 == 0)
		{
			Interface=~Interface;
			while(H4==0);
		}
	}
	R3 = 0;
	R1 = R2 = R4 = 1;
	H1 = H2 = H3 = H4 = 1;
	if(H1 == 0)
	{
		delayms(5);
		if(H1 == 0)
		{
			keyword=2;
			keystate = 1;
			while(H1==0);
		}
	}
	else if(H2 == 0)
	{
		delayms(5);
		if(H2 == 0)
		{
			keyword=5;
			keystate = 1;
			while(H2==0);
		}
	}
	else if(H3 == 0)
	{
		delayms(5);
		if(H3 == 0)
		{
			keyword=8;
			keystate = 1;
			while(H3==0);
		}
	}
	else if(H4 == 0)
	{
		delayms(5);
		if(H4 == 0)
		{
			Tset[0]=11;
			Tset[1]=11;
			Tset[2]=11;
			Tset[3]=11;
			keycount=0; //从第一位开始亮
			while(H4==0);
		}
	}
	if(keystate == 1)
	{
		if(keycount>4)
		{
			keycount=0;
		}
		keycount++;
	}
}

void Face()
{
	if(Interface==0)
	{
		display();
		Tmax_Init=Tmaxtemp;
		Tmin_Init=Tmintemp;//设置温度阈值
		if(Tmintemp<Tmaxtemp)
		{
			P2 = (P2 & 0x1f)| 0x80;
			P0 |= 0xfc;
			L2 = 1;
		}
	}
	else if(Interface==1)
	{
		display_setup();
		if(keycount<5)
		{
			Tset[keycount-1]=keyword;
			Tmaxtemp=Tset[0]*10+Tset[1];
			Tmintemp=Tset[2]*10+Tset[3];
		}
		if(Tset[3]!=11)
		{
			if(Tmintemp>Tmaxtemp)
			{
				P2 = (P2 & 0x1f)| 0x80;
				P0 |= 0xfc;
				L2 = 0;
			}
		}
		if(Tset[3]==11)
		{
			P2 = (P2 & 0x1f)| 0x80;
			P0 |= 0xfc;
			L2 = 1;
		}
	}
}
void display_setup()
{
	display_bit(0,dpnum[10]);
	delayms(1);
	display_bit(1,dpnum[Tset[0]]);
	delayms(1);
	display_bit(2,dpnum[Tset[1]]);
	delayms(1);
	display_bit(3,dpnum[11]);
	delayms(1);
	display_bit(4,dpnum[11]);
	delayms(1);
	display_bit(5,dpnum[10]);
	delayms(1);
	display_bit(6,dpnum[Tset[2]]);
	delayms(1);
	display_bit(7,dpnum[Tset[3]]);
	delayms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}

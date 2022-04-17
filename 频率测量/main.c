#include <STC15F2K60S2.h>

#define uchar unsigned char
#define uint unsigned int
	
uchar code number[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0x8e,0xff};
uint count_f=0;  
uint dat_f=0;   //此处数据类型用int ,因为char 不够用，此处也是关键
uchar count_t=0;

void display();
void display_bit(uchar pos , uchar dat);
void SMGdelayms(uchar ms);
void Timer01_Init();
void System_Init();

void main()
{
	System_Init();
	Timer01_Init();
	while(1)
	{
		display();
	}
}

void System_Init()
{
	P2 = (P2 & 0x1f) | 0x80;
	P0=0xff;
	P2 = (P2 & 0x1f) | 0xa0;
	P0=0x00;
	P2 = (P2 & 0x1f) | 0xc0;
	P0=0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0=0xff;
}
void Timer01_Init()  //定时器0用来计数，定时器1用来计时
{
	TMOD = 0X16;
	
	TH0 = 255;  //定时器0用8位自动重装模式
	TL0 = 255;
	
	TH1 = (65536-50000)/256;
	TL1 = (65536-50000)%256;
	
	TR0 = 1;  //定时器0开始计数
	TR1 = 1;  //定时器1开始计时
	ET0 = 1;  //定时器0使能中断打开
	ET1 = 1;  //定时器1使能中断打开
	EA = 1;   //使能总中断打开
}

void Timer0() interrupt 1
{
	count_f++;
}
void Timer1() interrupt 3
{
	TH1 = (65536-50000)/256; //重新装载计时初值
	TL1 = (65536-50000)%256;
	count_t++;
	if(count_t==20)
	{
		count_t=0;
		dat_f=count_f;
		count_f=0;
	}
}
void SMGdelayms(uchar ms)
{
	uint i,j;
	for(i=ms;i>0;i--)
		for(j=815;j>0;j--);
}
void display_bit(uchar pos , uchar dat)
{
	P0 = 0xff;  //数码管消隐
	P2 = (P2 & 0x1f) | 0xc0;
	P0=0x01 << pos;
	P2 = (P2 & 0x1f) | 0xe0;
	P0=number[dat];
}
void display()
{
	display_bit(0,11);
	SMGdelayms(1);
	display_bit(1,12);
	SMGdelayms(1);
	display_bit(2,12);
	SMGdelayms(1);
	if(dat_f > 9999)   //如果高位为0则不显示，因此用if语句判断
	{
		display_bit(3,dat_f / 10000);
		SMGdelayms(1);
	}
	if(dat_f > 999)
	{
		display_bit(4,(dat_f / 1000) %10);
		SMGdelayms(1);
	}
	if(dat_f > 99)
	{
		display_bit(5,(dat_f / 100) % 10);
		SMGdelayms(1);
	}
	if(dat_f > 9)
	{
		display_bit(6,(dat_f / 10) %10);
		SMGdelayms(1);
	}
	display_bit(7,dat_f % 10);
	SMGdelayms(1);
}
#include "reg52.h"
#include "iic.h"
#include "intrins.h"

unsigned char code smg_duan[10] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
unsigned int num ;
unsigned int num1 ;
unsigned char count ;
short int canshu = 0;
unsigned int jishu = 0;
unsigned char a;   //�����ж��Ƿ��������
unsigned char b;   //�����ж�L1���Ƿ����

//public enum Num {x1 = 1,x2,x3}x;  ������ԭ�������ö��  

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
void Delay100us(unsigned int n)		//��11.0592MHz�µ���ʱ����
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

void Init_138(unsigned char n)    //������ѡ��
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

void SMG_Display(unsigned char dat,unsigned char pos)  //����ܻ�����ʾ����
{
	Init_138(6);
	P0 = 0x01 << pos;
	Init_138(7);
	P0 = dat;
	
	Delay100us(10);
}

//***************************************************

//******************PCF8591��غ���******************
void Init_PCF8591()   //��ʼ��PCF8591����
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x03);
	IIC_WaitAck();
	IIC_Stop();
}

void Read_PCF8591()    //��ȡPCF8591����ֵ
{
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	num = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
}

//***************************************************

//*****************EEPROM��غ���********************
void Write_EEPROM(unsigned int dat,unsigned char addr)    //����д��EEPROM����
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

void Read_EEPROM(unsigned char addr)      //���ݴ�EEPROM�ж�ȡ����
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

void Key()        //����ɨ�躯��������ֻʹ�����ĸ�����
{
	R1 = 0;
	R2 = 1 ;
	M1 = M2 =1 ;
	if(M1 == 0)
	{
		Delay100us(10);
		if(M1 == 0 )     //S13���� ����
		{
			jishu = 0;	
		}
	}
	if(M2 == 0)
	{
		Delay100us(10);
		if(M2 == 0 )      //S17���� ��
		{
			if(count == 1)     //ֻ���ڲ���ҳ����ܴ���
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
		if(M1 == 0 )     //S12���� ��ʾҳ���л�
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
		if(M2 == 0 )      //S16���� ��
		{
			if(count == 1)    //ֻ���ڲ���ҳ����ܴ���
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

void Dat_arrange()      //���ݴ�������ת��Ϊ��ѹֵ
{
	num1 = num * 1.961;    //  500 / 255 = 1.961   ����������100�� 
}

void Voltage_Display()               //��ѹ��ʾ
{
	Read_PCF8591();
	Dat_arrange();
	SMG_Display(0xc1,0);   //1100 0001
	SMG_Display(smg_duan[num1 / 100]& 0x7f,5);
	SMG_Display(smg_duan[num1 % 100 / 10] ,6);
	SMG_Display(smg_duan[num1 % 10],7);
	SMG_Display(0xff,7);
}

void Canshu_Display()                 //������ʾ
{
	SMG_Display(0x8c,0);   //    1000 1100
	SMG_Display(smg_duan[canshu / 100]& 0x7f,5);
	SMG_Display(smg_duan[canshu % 100 / 10] ,6);
	SMG_Display(smg_duan[canshu % 10],7);
	SMG_Display(0xff,7);
}

void jishu_Display()                   //������ʾ
{
	SMG_Display(0xc8,0);   //    1100 1000  
	SMG_Display(smg_duan[jishu / 100],5);
	SMG_Display(smg_duan[jishu % 100 / 10] ,6);
	SMG_Display(smg_duan[jishu % 10],7);
	SMG_Display(0xff,7);
	
}

void Init_Read_canshu()      //������ʼ����ȡ
{
	Read_EEPROM(0);
	canshu = canshu * 10;
}

void count_judgment()        //�жϺ���
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


//*******************��ʱ������**********************
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
		b = 0;   //L1 Ϩ��
		i = 0;
	} 
	if(i == 100)
	{
		b = 1;  //L1 ����
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
			         Write_EEPROM(canshu * 0.1,0);//��������10��������ԭ����������100���������������0.1
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


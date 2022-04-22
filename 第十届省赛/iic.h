#ifndef __IIC_H__
#define __IIC_H__

void IIC_Start(void); 
void IIC_Stop(void); 

bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit);

void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 

#endif
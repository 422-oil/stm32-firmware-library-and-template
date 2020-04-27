
#ifndef __USART485_H__
#define __USART485_H__

#include<stm32f10x.h>

typedef enum 
{
	RATE_9600 = 0, 
	RATE_19200 = 1,
	RATE_38400=2
} BoundRate;

typedef enum 
{
	READ485 = 0, 
	WRITE485 = !READ485
} RW485State;

void usart2_change_bound(BoundRate boundrate);

void usart3_RW485(RW485State NewState);//使能usart3的rs485 读/写
void usart2_RW485(RW485State NewState);//使能usart2的rs485 读/写
void usart1_RW485(RW485State NewState);//使能usart1的rs485 读/写
void usartInit(void);//串口初始化
void usart3_write(u8 *src, u16 num);
void usart2_write(u8 *src, u16 num);
void usart1_write(u8 *src, u16 num);

#endif //__USART485_H__







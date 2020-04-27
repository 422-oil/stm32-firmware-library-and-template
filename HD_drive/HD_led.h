#ifndef __LED_H__
#define __LED_H__


#include <stm32f10x.h>

//打开LED就可以使用宏定义 LED1(LED_ON),关闭调用LED1(LED_OFF)
	#define LED_ON    Bit_RESET 
	#define LED_OFF   Bit_SET
	
	#define LED1(x)   GPIO_WriteBit(GPIOA, GPIO_Pin_4, x)
	#define LED2(x)   GPIO_WriteBit(GPIOA, GPIO_Pin_5, x)
	#define LED3(x)   GPIO_WriteBit(GPIOA, GPIO_Pin_6, x)
	#define LED4(x)   GPIO_WriteBit(GPIOA, GPIO_Pin_7, x)
	#define LED5(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_0, x)


void init_LED(void);//LED初始化
void LED_ALL_ON(void);
void LED_ALL_OFF(void);

#endif //__LED_H__



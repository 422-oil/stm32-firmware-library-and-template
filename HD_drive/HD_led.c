/**
  ******************************************************************************
  * @file    HD_led.c
  * @author  Helone
  * @version V1.0
  * @date    2018-7-10
  * @brief   主板5个LED灯的底层驱动程序
  ******************************************************************************
	* @attention:
	*文件工程使用 KEIL4.72编写 
	*源代码文字编码使用 ANSI  or   UTF-8 without signature， 
	*当出现乱码时在 Edit->Configuration->Editor->Encoding 下修改
  */
#include "led.h"


/*******************************************************************************
* 功能: 初始化LED
* 参数：none
* 返回：none
*******************************************************************************/
void init_LED(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//声明一个结构体变量，用来初始化GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		

	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_Init(GPIOB,&GPIO_InitStructure);  //

	GPIO_WriteBit(GPIOA, GPIO_Pin_4,Bit_SET);//
	GPIO_WriteBit(GPIOA, GPIO_Pin_5,Bit_SET);//
	GPIO_WriteBit(GPIOA, GPIO_Pin_6,Bit_SET);//
	GPIO_WriteBit(GPIOA, GPIO_Pin_7,Bit_SET);//
	GPIO_WriteBit(GPIOB, GPIO_Pin_0,Bit_SET);//

}

void init_LED1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//声明一个结构体变量，用来初始化GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
		

	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //LED1

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_Init(GPIOC,&GPIO_InitStructure);  //LED2

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
	GPIO_Init(GPIOC,&GPIO_InitStructure);  //LED3

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitStructure);  //LED4

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
	GPIO_Init(GPIOB,&GPIO_InitStructure);  //LED5

	GPIO_WriteBit(GPIOA, GPIO_Pin_0,Bit_SET);//
	GPIO_WriteBit(GPIOC, GPIO_Pin_14,Bit_SET);//
	GPIO_WriteBit(GPIOC, GPIO_Pin_13,Bit_SET);//
	GPIO_WriteBit(GPIOB, GPIO_Pin_9,Bit_SET);//
	GPIO_WriteBit(GPIOB, GPIO_Pin_8,Bit_SET);//
}

/*******************************************************************************
* 功能: 5个LED全部打开
* 参数：none
* 返回：none
*******************************************************************************/
void LED_ALL_ON(void)
{
	LED1(LED_ON);
	LED2(LED_ON);
	LED3(LED_ON);
	LED4(LED_ON);
	LED5(LED_ON);

}

/*******************************************************************************
* 功能: 5个LED全部关闭
* 参数：none
* 返回：none
*******************************************************************************/
void LED_ALL_OFF(void)
{
	LED1(LED_OFF);
	LED2(LED_OFF);
	LED3(LED_OFF);
	LED4(LED_OFF);
	LED5(LED_OFF);

}





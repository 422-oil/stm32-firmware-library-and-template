/**
  ******************************************************************************
  * @file    HD_timer.c
  * @author  Helone
  * @version V1.0
  * @date    2018-7-10
  * @brief   定时器的初始化
  ******************************************************************************
	* @attention:
	*文件工程使用 KEIL4.72编写 
	*源代码文字编码使用 ANSI  or   UTF-8 without signature， 
	*当出现乱码时在 Edit->Configuration->Editor->Encoding 下修改
  */
#include "timer.h"

/*******************************************************************************
* 功能: 定时器初始化
* 参数: none
* 返回: none
*******************************************************************************/
void time_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	 //声明一个结构体变量，用来初始化GPIO

	NVIC_InitTypeDef NVIC_InitStructure;

	/* 开启定时器3时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);//清除TIMx的中断待处理位:TIM 中断源
	TIM_TimeBaseInitStructure.TIM_Period = 700;//设置自动重装载寄存器周期的值 50ms
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7199;//设置用来作为TIMx时钟频率预分频值，10Khz计数频率，每0.1ms计数一次
	TIM_TimeBaseInitStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);	
	TIM_Cmd(TIM3,ENABLE); //使能或者失能TIMx外设
	/* 设置中断参数，并打开中断 */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE );	//使能或者失能指定的TIM中断
	
	/* 设置NVIC参数 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //打开TIM3_IRQn的全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;  //响应优先级为1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能
	NVIC_Init(&NVIC_InitStructure);	
}

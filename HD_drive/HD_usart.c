/**
  ******************************************************************************
  * @file    HD_usart.c
  * @author  Helone
  * @version V1.0
  * @date    2018-7-10
  * @brief   串口的初始化以及底层通信服务函数，经过测试，STM32的串口的硬件奇偶校验，
	* 只能实现发送时候自动添加校验位，中断接收的时候不能自动校验，会正常接收校验错误的数据。
  ******************************************************************************
	* @attention:
	*文件工程使用 KEIL4.72编写 
	*源代码文字编码使用 ANSI  or   UTF-8 without signature， 
	*当出现乱码时在 Edit->Configuration->Editor->Encoding 下修改
  */

#include "public.h"
#include "usart.h"
#include "systick.h"


/*******************************************************************************
* 功能: 串口1的485总线输入输出控制
* 参数: 
*		NewState: READ485->接收模式， WRITE485->发送模式
* 返回: none
*******************************************************************************/
void usart1_RW485(RW485State NewState)
{//与加油机、读卡器通信    READ485：485读使能   WRITE485：485写使能
	if(NewState == READ485)
	{
		delay_ms(1);
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);//enable 485 read
		
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_8);//enable 485 write
		delay_ms(1);
	}
	
}


/*******************************************************************************
* 功能: 串口2的485总线输入输出控制
* 参数: 
*		NewState: READ485->接收模式， WRITE485->发送模式
* 返回: none
*******************************************************************************/
void usart2_RW485(RW485State NewState)
{//与加油机、读卡器通信    READ485：485读使能   WRITE485：485写使能
	if(NewState == READ485)
	{
		delay_us(500);
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);//enable 485 read
		
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_1);//enable 485 write
		delay_ms(1);
	}
	
}


/*******************************************************************************
* 功能: 串口3的485总线输入输出控制
* 参数: 
*		NewState: READ485->接收模式， WRITE485->发送模式
* 返回: none
*******************************************************************************/
void usart3_RW485(RW485State NewState)
{//与PC通信 485写使能 READ485：485读使能   WRITE485：485写使能
	if(NewState == READ485)
	{
		delay_ms(1);
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);//enable 485 read
	
	}
	else
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_1);//enable 485 write
		delay_ms(1);
	}
	
}




/*******************************************************************************
* 功能: 串口1初始化， 与加油机通信， 波特率9600，奇校验
* 参数: none
* 返回: none
*******************************************************************************/
void usart1_init(void)
{
	
	GPIO_InitTypeDef   GPIO_InitStructure;	//声明一个结构体变量，用来初始化GPIO
	USART_InitTypeDef  USART_InitStructure;	//串口结构体定义
	NVIC_InitTypeDef   NVIC_InitStructure;  //中断结构体定义

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//打开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	//485读写控制
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;				//PA8
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;	    //推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);  				//初始化串口输入IO 
//	GPIO_SetBits(GPIOA, GPIO_Pin_8);  //enable 485 write
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);//enable 485 read
	
	
	// 配置GPIO的模式和IO口 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;     	//串口输出PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);               //初始化串口输入IO 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;  		//串口输入PA3
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //模拟输入
	GPIO_Init(GPIOA,&GPIO_InitStructure);  				  // 初始化GPIO 
	

	//  配置USART的模式和IO口
	USART_InitStructure.USART_BaudRate   = 9600;   //9600
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;   //9位数据长
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;		//1停止位
	USART_InitStructure.USART_Parity     = USART_Parity_Even;		//偶效验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //失能硬件流
	USART_InitStructure.USART_Mode       = USART_Mode_Rx|USART_Mode_Tx;	 //开启发送和接受模式
	USART_Init(USART1,&USART_InitStructure);	  //初始化USART1
	USART_Cmd(USART1, ENABLE);	        	      //使能USART1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//使能或者失能指定的USART中断 接收中断
	USART_ClearFlag(USART1,USART_FLAG_TC);        //清除USARTx的待处理标志位

	// 设置NVIC参数 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);    
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; 	       //打开USART1的全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 		   //响应优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE; 		   //使能
	NVIC_Init(&NVIC_InitStructure);
}





/*******************************************************************************
* 功能: 串口2初始化， 与读卡器通信 19200  偶校验
* 参数: none
* 返回: none
*******************************************************************************/
void usart2_init(void)
{
	
	GPIO_InitTypeDef   GPIO_InitStructure;	//声明一个结构体变量，用来初始化GPIO
	USART_InitTypeDef  USART_InitStructure;	//串口结构体定义
	NVIC_InitTypeDef   NVIC_InitStructure;  //中断结构体定义

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//打开时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	//485读写控制
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;				//PA1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;	    //推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);  				//初始化串口输入IO 
//	GPIO_SetBits(GPIOA, GPIO_Pin_1);  //enable 485 write
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);//enable 485 read
	
	
	// 配置GPIO的模式和IO口 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;     	//串口输出PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);               //初始化串口输入IO 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;  		//串口输入PA3
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //模拟输入
	GPIO_Init(GPIOA,&GPIO_InitStructure);  				  // 初始化GPIO 
	

	//  配置USART的模式和IO口
	USART_InitStructure.USART_BaudRate   = 19200;   //19200
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;   //9位数据长
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;		//1停止位
	USART_InitStructure.USART_Parity     =USART_Parity_Even;//偶校验  // USART_Parity_Odd;		//奇效验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //失能硬件流
	USART_InitStructure.USART_Mode       = USART_Mode_Rx|USART_Mode_Tx;	 //开启发送和接受模式
	USART_Init(USART2,&USART_InitStructure);	  //初始化USART2
	USART_Cmd(USART2, ENABLE);	        	      //使能USART2
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//使能或者失能指定的USART中断 接收中断
	USART_ClearFlag(USART2,USART_FLAG_TC);        //清除USARTx的待处理标志位

	// 设置NVIC参数 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);    
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; 	       //打开USART2的全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 		   //响应优先级为1
	NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE; 		   //使能
	NVIC_Init(&NVIC_InitStructure);
}



/*******************************************************************************
* 功能: 串口3初始化， 与PC 通信  9600 偶校验
* 参数: none
* 返回: none
*******************************************************************************/
void usart3_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;				//声明一个结构体变量，用来初始化GPIO
	USART_InitTypeDef  USART_InitStructure;	 			//串口结构体定义
	NVIC_InitTypeDef NVIC_InitStructure;				//中断结构体定义
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//打开时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//485读写控制引脚
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;      	 //PB1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;	 //复用推挽输出
	GPIO_Init(GPIOB,&GPIO_InitStructure);			     //初始化串口输入IO 
//	GPIO_SetBits(GPIOB, GPIO_Pin_1);  //enable 485 write
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);//enable 485 read
	
	//配置GPIO的模式和IO口 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;        	//TX串口输出PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;	    //复用推挽输出
	GPIO_Init(GPIOB,&GPIO_InitStructure);  					// 初始化串口输入IO */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11; 			//RX 串口输入PB11
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;	//模拟输入
	GPIO_Init(GPIOB,&GPIO_InitStructure); 					//初始化GPIO 
	
	//  配置USART的模式和IO口
	USART_InitStructure.USART_BaudRate   = 9600;   					//波特率设置为9600	 
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;	    //9数据长
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;		//1停止位
	USART_InitStructure.USART_Parity     = USART_Parity_Even;//USART_Parity_Even;		//偶效验 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //失能硬件流
	USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;	 				  //开启发送和接受模式
	USART_Init(USART3,&USART_InitStructure);	//初始化USART3
	USART_Cmd(USART3, ENABLE);		  			//使能USART3
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//使能或者失能指定的USART中断 接收中断
	USART_ClearFlag(USART3,USART_FLAG_TC);//清除USARTx的待处理标志位
	
	//设置NVIC参数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);    
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; 	         //打开USART3的全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 	 //抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 		     //响应优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			 //使能
	NVIC_Init(&NVIC_InitStructure);
}
/*******************************************************************************
* 功能: 串口初始化，在main()函数中调用
* 参数: none
* 返回: none
*******************************************************************************/
void usartInit(void)
{
	usart1_init();
	usart2_init();
	usart3_init();
	
	usart1_RW485(READ485);
	usart2_RW485(READ485);
	usart3_RW485(READ485);
}
/*******************************************************************************
* 功能: 串口1发送指定长度的数据
* 参数: 
*		src: 被发送数据的地址
*		num: 被发送数据的长度
* 返回: none
*******************************************************************************/
void usart1_write(u8 *src, u16 num)
{
	u16 i=0;
	usart1_RW485(WRITE485);//485  发送
	for(i=0; i<num; i++)
	{//发送数据
		
		USART1->DR = (src[i] & (uint16_t)0x01FF);//发一个字节
		while((USART1->SR & USART_FLAG_TXE)==Bit_RESET);//等待发送完成
		USART1->SR = (uint16_t)~USART_FLAG_TXE;//清除标志位
	}
	usart1_RW485(READ485);//485  接收
}



/*******************************************************************************
* 功能: 串口2发送指定长度的数据
* 参数: 
*		src: 被发送数据的地址
*		num: 被发送数据的长度
* 返回: none
*******************************************************************************/
void usart2_write(u8 *src, u16 num)
{
	u16 i=0;
	usart2_RW485(WRITE485);//485  发送
	for(i=0; i<num; i++)
	{//发送数据
		USART2->DR = (src[i] & (uint16_t)0x01FF);//发一个字节
		while((USART2->SR & USART_FLAG_TXE)==Bit_RESET);//等待发送完成
		USART2->SR = (uint16_t)~USART_FLAG_TXE;//清除标志位

	}
	
	
	usart2_RW485(READ485);//485  接收
}

/*******************************************************************************
* 功能: 串口3发送指定长度的数据
* 参数: 
*		src: 被发送数据的地址
*		num: 被发送数据的长度
* 返回: none
*******************************************************************************/
void usart3_write(u8 *src, u16 num)
{
	u16 i=0;
	usart3_RW485(WRITE485);//485  发送
	for(i=0; i<num; i++)
	{
		USART3->DR = (src[i] & (uint16_t)0x01FF);//发一个字节
		while((USART3->SR & USART_FLAG_TXE)==Bit_RESET);//等待发送完成
		USART3->SR = (uint16_t)~USART_FLAG_TXE;//清除标志位
	}
	
	
	usart3_RW485(READ485);//485  接收
}




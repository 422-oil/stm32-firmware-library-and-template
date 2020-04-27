
/**
  ******************************************************************************
  * @file    system.c
  * @author  Helone
  * @version V1.0
  * @date    2018-7-10
  * @brief   此文件定义了系统的一些通用功能函数
  ******************************************************************************
	* @attention:
	*文件工程使用 KEIL4.72编写 
	*源代码文字编码使用 ANSI  or   UTF-8 without signature， 
	*当出现乱码时在 Edit->Configuration->Editor->Encoding 下修改
  */


#include "system.h"
#include "public.h"
#include "ds1302.h"
#include "eeprom.h"
#include "flash.h"
#include "oil.h"
#include "pc.h"
#include "reader.h"
#include "systick.h"

/*******************************************************************************
* 功能： 将二进制数转换成 BCD码
* 参数： 
* 	number: 字节长度  total_data: 二进制数  pointer：存放BCD码结果的数组指针
* 返回：none
*******************************************************************************/
void get_bcd(u8 number,u32 total_data,u8 *pointer)
{
	u8 high_me,low_me,i;
	u32 temp_long;
		
	temp_long=1;
	for(i=1;i<(2*number);i++)
	{
		temp_long=temp_long*10;
	}

	for(i=0;i<number;i++)
	{
		high_me=(total_data/temp_long);
		total_data=(total_data%temp_long);
		temp_long=(temp_long/10);
		
		low_me=(total_data/temp_long);
		total_data=(total_data%temp_long);
		temp_long=(temp_long/10);
		
		*pointer=(high_me<<4)+low_me;
		pointer++;
	}
}
//

/*******************************************************************************
* 功能： 将BCD码转换成 二进制数
* 参数： 
* 	number: 字节长度   pointer：存放BCD码结果的数组指针
* 返回:
*		二进制数
*******************************************************************************/
u32 get_total(u8 number,u8 *pointer)	//得到总和
{
	u8 i;
	u32 total_data,temp_long;

	temp_long=1;
	for(i=1;i<2*number;i++)
	{
		temp_long=temp_long*10;
	}
	
	total_data=0;
	for(i=0;i<number;i++)
	{
		total_data=total_data+((*pointer)>>4)*temp_long;
		temp_long=temp_long/10;
		total_data=total_data+((*pointer)&0x0f)*temp_long;
		temp_long=temp_long/10;
		pointer=pointer+1;
	}
	return(total_data);
}
//

/*******************************************************************************
* 功能：校验是否为BCD码
* 参数：
*		pointer 数据指针
*		number  数据总数
* 返回：
*		true  是BCD
*		false 不是BCD
*******************************************************************************/
u8 verify_bcd(u8 *pointer,u8 number)
{
	u8 i;

	for(i=0;i<number;i++)
	{
		if(((*pointer)&0x0f)>9)
		{
			return(false);
		}
		if(((*pointer)>>4)>9)
		{
			return(false);
		}
	}
	return(true);
}
//

/*******************************************************************************
* 功能：切换系统当前的工作状态
* 参数： 
* 	choice: A B面选择
*		state: 状态
* 返回: none
*******************************************************************************/
void set_system_choice(u8 choice, u8 state)
{
	g_ui_choice_time_out[choice]=0;//超时计数器 清零
	g_uc_oil_index[choice]=0;//
	g_uc_oil_poll_change[choice]=0;

	if(g_uc_system_choice[choice]!=state)
	{//当前状态与预设状态不同
		g_b_oil_rece_chuli_flag=false;//禁止处理数据
		init_oil_rece();//准备接受数据
	}

	g_uc_system_choice[choice]=state;//改变系统状态
	g_uc_system_choice1[choice]=state;//变量未创建
	g_uc_system_choice_bak[choice]=state;//变量未创建
}
//

/*******************************************************************************
* 功能：校验时间是否超过合法范围(比如，月份是否超过12，小时是否超过23等)，数据顺序 月日时分秒
* 参数： 
* 	*pointer: 指向时间数组
* 返回: 
*		true:  合法
*		false: 不合法
*******************************************************************************/
u8 verify_time(u8 *pointer)
{
	pointer++;
	if((*pointer)>0x12)
	{//月
		return(false);
	}
	pointer++;
	if((*pointer)>0x31)
	{//日
		return(false);
	}
	pointer++;
	if((*pointer)>0x23)
	{//时
		return(false);
	}
	pointer++;
	if((*pointer)>0x59)
	{//分
		return(false);
	}
	pointer++;
	if((*pointer)>0x59)
	{//秒
		return(false);
	}
	return true;
}
//

/*******************************************************************************
* 功能: 校验时间是否超过合法范围(比如，月份是否超过12，小时是否超过23等)，数据顺序 秒分时日月
* 参数： 
* 	*pointer: 指向时间数组
* 返回: 
*		true:  合法
*		false: 不合法
*******************************************************************************/
static u8 verify_time1(u8 *pointer)
{//校验时间格式是否正确，例如：月是否超过了12等。是否秒分时日月年
	if((*pointer)>0x59)
	{//秒
		return(false);
	}
	pointer++;
	if((*pointer)>0x59)
	{//分
		return(false);
	}
	pointer++;
	if((*pointer)>0x23)
	{//时
		return(false);
	}
	pointer++;
	if((*pointer)>0x31)
	{//日
		return(false);
	}
	pointer++;
	if((*pointer)>0x12)
	{//月
		return(false);
	}
	return true;
}
//

/*******************************************************************************
* 功能：校验时间是否在出厂日期之后
* 参数： 
* 	year: 年  month: 月  day: 日  （BCD码格式）
* 返回: 
*		true:  合法，在出厂日期之后
*		false: 不合法， 在出厂日期之前
*******************************************************************************/
u8 verify_indate(u8 year,u8 month,u8 day)
{
	u32 temp_data;

	temp_data=(((u32)(year))<<16)+(((u32)(month))<<8)+((u32)(day));

	if(temp_data<0x00180710) //2018年 07月 10日 ， 如果时间在这之前则非法
	{
		return(false);
	}
	else
	{
		return(true);
	}
}
//

/*******************************************************************************
* 功能：将系统时间写入DS1302
* 参数：none
* 返回: none
*******************************************************************************/
void set_rtc_clock(void)
{
	TIME_TypeDef time;
	
	
	time.year=g_uc_temp_time_buf[5];			//是时钟的年时间
	time.month=g_uc_temp_time_buf[4];			//是时钟的月时间
	time.date=g_uc_temp_time_buf[3];			//是时钟的日期时间
	time.hour=g_uc_temp_time_buf[2];			//是时钟的小时时间
	time.min=g_uc_temp_time_buf[1];				//是时钟的分钟时间
	time.sec=(g_uc_temp_time_buf[0]&0x7f);		//是时钟的秒时间
	
	time.week=1;//周，用不到，这个不需要设置
	
	DS1302_WrintTime(&time);
}
//


/*******************************************************************************
* 功能：从DS1302读取时间
* 参数：none
* 返回: none
*******************************************************************************/
void get_rtc_clock(void)
{
	TIME_TypeDef time;
	u8 i,temp_time[6];
	DS1302_ReadTime(&time) ;//从ds1302中读取时间


	temp_time[5]=time.year;			//年时间
	temp_time[4]=time.month;			//月时间
	temp_time[3]=time.date;			//日期时间

	temp_time[2]=time.hour;			//小时时间
	temp_time[1]=time.min;		//分钟时间
	temp_time[0]=time.sec;		//秒时间



	//脱机情况下，只有当读出的时间有效，才能使用
	if((verify_bcd(temp_time,6)==true)&&(verify_time1(temp_time)==true))
	{//校验时间是否是BCD码，校验时间是否有效
		if(verify_indate(temp_time[5],temp_time[4],temp_time[3])==true)
		{
			for(i=0;i<6;i++)
			{
				g_uc_temp_time_buf[i]=temp_time[i];
			}
			return;
		}
	}
}
//

/*******************************************************************************
* 功能：校验时间数据，g_uc_time_bak[7]的最后一个字节存放的是前6个字节的校验和，
				此函数通过验证此校验和来确认时间是否可用。
* 参数：none
* 返回: 
*		true: 验证成功
*		false: 验证失败
*******************************************************************************/
static u8 verify_time_parity(void)
{
	u8 i,temp;
	
	temp=0;
	for(i=0;i<6;i++)
	{
		temp=temp+g_uc_time_bak[i];
	}
	if(temp==g_uc_time_bak[6])
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
//

/*******************************************************************************
* 功能：初始化系统时间。 如果g_uc_time_bak[7]中有合法时间数据则使用此时间，否则初始化为出厂时间 2018年07月10日
* 参数：none
* 返回: none
*******************************************************************************/
void init_system_time(void)
{
	u8 i;
	u8 rel[4]={0};

	rel[0] = verify_bcd(g_uc_time_bak,6);//
	rel[1] = verify_time(g_uc_time_bak);
	rel[2] = verify_time_parity();
	rel[3] = verify_indate(g_uc_time_bak[0],g_uc_time_bak[1],g_uc_time_bak[2]);
	
	if(rel[0] && rel[1] && rel[2] && rel[3])
	{
	}
	else
	{
		g_uc_time_bak[0]=0x18;	//2018年
		g_uc_time_bak[1]=0x07;  //07月
		g_uc_time_bak[2]=0x10;	//10日
		g_uc_time_bak[3]=0;
		g_uc_time_bak[4]=0;
		g_uc_time_bak[5]=0;

		g_uc_time_bak[6]=0;
		for(i=0;i<6;i++)
		{//计算校验和
			g_uc_time_bak[6]=g_uc_time_bak[6]+g_uc_time_bak[i];
		}

		for(i=0;i<7;i++)
		{//存入eeprom
			s_uc_time_bak_RD24C04(&g_uc_time_bak[i],i);
		}
	}
	
	for(i=0;i<6;i++)
	{
		g_uc_temp_time_buf[i]=g_uc_time_bak[5-i];
	}
}
///

/*******************************************************************************
* 功能：将eeprom里备份的RAM变量恢复。 在系统掉电后可以保护RAM变量不会消失
* 参数：none
* 返回: none
*******************************************************************************/
void init_static_var(void)
{
	u8 i=0,j=0;
	

	g_uc_test_data1_RD24C04(&g_uc_test_data1);
	g_uc_test_data2_RD24C04(&g_uc_test_data2);
	

	if((g_uc_test_data1==0x55)&&(g_uc_test_data2==0xaa))
	{//软件复位 或者 中途断电再次上电//恢复数据
		//总量
		
		
		
		for(i=0;i<2;i++)
			for(j=0;j<5;j++)
			{
				s_uc_total_RD24C04(&(g_uc_total[i][j]),i,j);
			}
		s_uc_money_low_RD24C04(&g_uc_money_low);//最低金额(卡内金额低于此不能加油)
		s_uc_volume_low_RD24C04(&g_uc_volume_low);//最低油量(加油量不能比这个数小)
		//A/B面的油品信息（几号汽油或柴油）
		for(i=0;i<2;i++)
		{//4
			s_uc_adding_price_RD24C04(&g_uc_price[i][0],i,0);
			s_uc_adding_price_RD24C04(&g_uc_price[i][1],i,1);
			s_uc_gun_oil_type_RD24C04(&(g_uc_gun_oil_type[i]), i);
		}
		//油品类型；0 代表汽油，其它代表柴油(目前此变量只是存储PC发来的类型数据，并没有实际使用到)
		for(i=0;i<2;i++)
		{//5
			s_uc_oil_type_RD24C04(&(g_uc_oil_type[i]), i);
		}
		//解灰
		s_uc_kill_gray_card_number_RD24C04(&g_uc_kill_gray_card_number);
		s_uc_kill_gray_card_pointer_RD24C04(&g_uc_kill_gray_card_pointer);
		//灰卡
		s_uc_mine_gray_card_number_RD24C04(&g_uc_mine_gray_card_number);
		s_uc_mine_gray_card_pointer_RD24C04(&g_uc_mine_gray_card_pointer);
		//充值
		s_uc_add_money_number_RD24C04(&g_uc_add_money_number);
		s_uc_add_money_pointer_RD24C04(&g_uc_add_money_pointer);
		//加油记录  指针
		for(i=0;i<2;i++)
		{
			s_ui_record_pointer_RD24C04(&(g_ui_record_pointer[i]),i); 
		}
		//加油记录  计数
		for(i=0;i<2;i++)
		{
			s_ui_record_number_RD24C04(&(g_ui_record_number[i]),i);
		}
		//上一笔加油量
		for(i=0;i<2;i++)
			for(j=0;j<3;j++)
			{//14
				s_uc_add_last_data_RD24C04(&(g_uc_add_last_data[i][j]), i,j);
			}
		//加油记录 序列号
		for(i=0;i<2;i++)
		{
			s_ui_serial_RD24C04(&(g_ui_serial[i]),i);
		}
		//卡优惠等级
		for(i=0;i<2;i++)
		{
			s_uc_card_level_RD24C04(&(g_uc_card_level[i]), i);
		}
		
		for(i=0;i<2;i++)
			for(j=0;j<3;j++)
			{//17
				s_uc_adding_card_RD24C04(&(g_uc_adding_card[i][j]), i,j);
			}

		for(i=0;i<2;i++)
			for(j=0;j<4;j++)
			{
				s_uc_adding_card_money_RD24C04(&(g_uc_adding_card_money[i][j]), i,j);
			}

		for(i=0;i<2;i++)
			for(j=0;j<4;j++)
			{
				s_uc_card_jiayouyuan_RD24C04(&(g_uc_card_jiayouyuan[i][j]), i,j);
			}	
		//优惠单价
		for(i=0;i<2;i++)
			for(j=0;j<5;j++)
			{
				s_ui_price_youhui_RD24C04(&(g_ui_price_youhui[i][j]), i, j);
			}
		for(i=0;i<8;i++)
		{//21
			s_uc_jifen_para_RD24C04(&(g_uc_jifen_para[i]), i);
		}
	//	for(i=0;i<2;i++)//没有对应的变量
	//	{//22
	//		s_ui_card_number_RD24C04(&(g_ui_card_number[i]), i);
	//	}
		
		for(i=0;i<48;i++)
		{
			s_uc_oil_name_RD24C04(&(g_uc_oil_name[i]), i);
		}
		for(i=0;i<2;i++)
			for(j=0;j<20;j++)
			{
				s_uc_addinfo_backup_RD24C04(&(g_uc_addinfo_backup[i][j]), i,j);
			}	
		for(i=0;i<2;i++)
			for(j=0;j<12;j++)
			{
				s_uc_addgray_backup_RD24C04(&(g_uc_addgray_backup[i][j]), i,j);
			}		
		for(i=0;i<2;i++)
		{//26
			s_uc_card_type_RD24C04(&(g_uc_card_type[i]), i );
		}
		for(i=0;i<2;i++)
		{
			s_uc_need_find_serial_number_RD24C04(&(g_uc_need_find_serial_number[i]), i);
		}
		
		for(i=0;i<2;i++)
			for(j=0;j<10;j++)
			{
				s_uc_need_find_serial_high_RD24C04(&(g_uc_need_find_serial_high[i][j]), i,j);
			}	
		for(i=0;i<2;i++)
			for(j=0;j<10;j++)
			{
				s_uc_need_find_serial_low_RD24C04(&(g_uc_need_find_serial_low[i][j]), i,j);
			}		
		for(i=0;i<4;i++)
		{
			s_uc_host_num_RD24C04(&(g_uc_host_num[i]),i); 
		}	
		for(i=0;i<7;i++)
		{//31
			s_uc_time_bak_RD24C04(&(g_uc_time_bak[i]), i);
		}	
		//32
		//33

		for(i=0;i<2;i++)
		{
			g_uc_addover_at24c04_flag_RD24C04( &(g_uc_addover_at24c04_flag[i]), i); 
		}	
		for(i=0;i<2;i++)
			for(j=0;j<34;j++)
			{
				g_uc_addover_at24c04_data_RD24C04(&(g_uc_addover_at24c04_data[i][j]), i,j); 
			}	
		
		//36
		s_ul_card_zongshu_RD24C04(&g_ul_card_zongshu); 

		for(i=0;i<2;i++)
			for(j=0;j<2;j++)
			{
				s_uc_adding_price_RD24C04(&(g_uc_price[i][j]),i,j); 
			}	
		for(i=0;i<2;i++)
		{
			s_uc_gun_RD24C04(&(g_uc_gun[i]),i);  
		}	
		
		for(i=0;i<2;i++)
		{//39
			s_uc_hava_record_RD24C04(&(g_uc_hava_record[i]) ,i);  
		}
		for(i=0;i<2;i++)
		{//40
			s_uc_is_init_machine_num_RD24C04(&(g_uc_is_init_machine_num[i]), i); 
		}
		
		
	}
	else
	{//首次启动eeprom
		for(i=0;i<2;i++)
			for(j=0;j<5;j++)
			{
				g_uc_total[i][j]=0;
				s_uc_total_WR24C04(i,j,&(g_uc_total[i][j]));				
			}
		g_uc_money_low=5;
		s_uc_money_low_WR24C04(&g_uc_money_low);

		g_uc_volume_low=1;
		s_uc_volume_low_WR24C04(&g_uc_volume_low);

		for(i=0;i<2;i++)
			{//4 油品信息		
				g_uc_gun_oil_type[i]=OIL_10;
				s_uc_gun_oil_type_WR24C04(i,&g_uc_gun_oil_type[i]);
			}
		//解灰	
		g_uc_kill_gray_card_number=0;
		s_uc_kill_gray_card_number_WR24C04(&g_uc_kill_gray_card_number);
		g_uc_kill_gray_card_pointer=0;
		s_uc_kill_gray_card_pointer_WR24C04(&g_uc_kill_gray_card_pointer);
		//灰卡
		g_uc_mine_gray_card_number=0;
		s_uc_mine_gray_card_number_WR24C04(&g_uc_mine_gray_card_number);		
		g_uc_mine_gray_card_pointer=0;		
		s_uc_mine_gray_card_pointer_WR24C04(&g_uc_mine_gray_card_pointer);
		//充值
		g_uc_add_money_number=0;
		s_uc_add_money_number_WR24C04(&g_uc_add_money_number);		
		g_uc_add_money_pointer=0;
		s_uc_add_money_pointer_WR24C04(&g_uc_add_money_pointer);

		for(i=0;i<2;i++)
		{//加油记录
			
			
			g_ui_record_pointer[i]=0;
			s_ui_record_pointer_WR24C04(i,&g_ui_record_pointer[i]);
			
			g_ui_record_number[i]=0;
			s_ui_record_number_WR24C04(i,&g_ui_record_number[i]);
			
			g_ui_serial[i]=0;
			s_ui_serial_WR24C04(i,&g_ui_serial[i]);
		}
			
		for(i=0;i<2;i++)
		{//上一笔加油的加油量	
			for(j=0;j<3;j++)
			{
				g_uc_add_last_data[i][j]=0;
				s_uc_add_last_data_WR24C04(i,j,&g_uc_add_last_data[i][j]);
			}			
		}

		//卡优惠等级
		for(i=0;i<2;i++)
		{//16
			g_uc_card_level[i]=0;//无 优惠
			s_uc_card_level_WR24C04(i, &(g_uc_card_level[i]));
		}

		for(i=0;i<2;i++)
		{
			for(j=0;j<5;j++)
			{//优惠单价
				g_ui_price_youhui[i][j]=100;
				s_ui_price_youhui_WR24C04(i,j,&g_ui_price_youhui[i][j]);
			}
			clear_gray_add_info(i);
		}

		for(i=0;i<8;i++)
		{//积分参数
			g_uc_jifen_para[i]=0;		
			s_uc_jifen_para_WR24C04(i,&g_uc_jifen_para[i]);
		}

		for(i=0;i<48;i++)
		{//油品名称
			g_uc_oil_name[i]=0x20;
			s_uc_oil_name_WR24C04(i,&g_uc_oil_name[i]);
		}

		for(i=0;i<2;i++)
		{//加油信息备份	
			g_uc_addinfo_backup[i][19]=INVALID;
			s_uc_addinfo_backup_WR24C04(i,19,&g_uc_addinfo_backup[i][19]);//数据不可用
		}
		for(i=0;i<2;i++)
		{//加灰信息备份		
			g_uc_addgray_backup[i][11]=INVALID;
			s_uc_addgray_backup_WR24C04(i ,11,&g_uc_addgray_backup[i][11]);						
		}

		for(i=0;i<2;i++)
		{//查找序列号
			g_uc_need_find_serial_number[i]=0;
			s_uc_need_find_serial_number_WR24C04(i,&g_uc_need_find_serial_number[i]);			
		}
		g_ul_card_zongshu=0;
		s_ul_card_zongshu_WR24C04(&g_ul_card_zongshu);

		for(i=0;i<2;i++)
		{
			g_uc_hava_record[i]=false;
			s_uc_hava_record_WR24C04(i,&g_uc_hava_record[i]);
		}

		for(i=0;i<2;i++)
		{
			g_uc_is_init_machine_num[i]=true;
			s_uc_is_init_machine_num_WR24C04(i,&g_uc_is_init_machine_num[i]);
		}

		
		//初始化时间
		g_uc_temp_time_buf[5]=0x18;	//年
		g_uc_temp_time_buf[4]=0x07;
		g_uc_temp_time_buf[3]=0x10;
		g_uc_temp_time_buf[2]=0;
		g_uc_temp_time_buf[1]=0;
		g_uc_temp_time_buf[0]=0;
		set_rtc_clock();
		
		
		g_uc_test_data1=0x55;
		g_uc_test_data2=0xaa;
		g_uc_test_data1_WR24C04(&g_uc_test_data1);
		g_uc_test_data2_WR24C04(&g_uc_test_data2);
		
		
	
	}

}
//


/*******************************************************************************
* 功能：验证一个长度为3的数组不为零
* 参数：
*		*pointer 指向数组
* 返回: 
*		true 不为零   false 为零
*******************************************************************************/
u8 begin_adding(u8 *pointer)		
{
	u8 i;

	for(i=0;i<3;i++)
	{
		if((*pointer)!=0)
		{
			return(true);
		}
		pointer=pointer+1;
	}
	return(false);
}

//

/*******************************************************************************
* 功能：初始化机号, 先从eeprom读取机号，如果没有则从flash读取
* 参数：none
* 返回：
*		TRUE 	成功
*		FALSE  失败
*******************************************************************************/
u8 init_machine_number(void)//初始化机号
{
	u8 tmp_a;
	//机号1
	s_uc_host_num_RD24C04(&(g_uc_host_num[0]), 0);
	s_uc_host_num_RD24C04(&(g_uc_host_num[1]), 1);
	if((g_uc_host_num[0]>0) && (g_uc_host_num[0]<=99) && (g_uc_host_num[0]==(~g_uc_host_num[1])))
	{//主机号 在 0~99之间
		g_uc_machine_number[0]=g_uc_host_num[0];
		g_uc_machine_number_bak[0]=g_uc_host_num[0];
	}
	else
	{
		tmp_a = read_machine_num(0);//从flash读取机号
		if((tmp_a>0)&&(tmp_a<=99))
		{//读到合法 机号 放入变量
			g_uc_machine_number[0]=tmp_a;
			g_uc_machine_number_bak[0]=tmp_a;
			
			g_uc_host_num[0]=g_uc_machine_number[0];
			s_uc_host_num_WR24C04(0,&(g_uc_host_num[0]));
			
			g_uc_host_num[1]=255-g_uc_machine_number[0];	//存放	g_uc_host_num[0] 的反码
			s_uc_host_num_WR24C04(1,&(g_uc_host_num[1]));		
		}
		else
		{//未读到合法 机号 初始化机号
			g_uc_is_init_machine_num[0]=true;
			s_uc_is_init_machine_num_WR24C04(0,&g_uc_is_init_machine_num[0]);
			g_uc_host_num[0]=DEFAULT_MACHINE_NUMBER0;
			s_uc_host_num_WR24C04(0,&g_uc_host_num[0]);
			
			g_uc_host_num[1]=~(g_uc_host_num[0]);//存放	g_uc_host_num[0] 的反码
			s_uc_host_num_WR24C04(1,&g_uc_host_num[1]);
			
			g_uc_machine_number[0]=DEFAULT_MACHINE_NUMBER0;
			g_uc_machine_number_bak[0]=DEFAULT_MACHINE_NUMBER0;
		}
	}

	
	//机号2
	s_uc_host_num_RD24C04(&g_uc_host_num[2], 2);
	s_uc_host_num_RD24C04(&g_uc_host_num[3], 3);
	if((g_uc_host_num[2]>0) && (g_uc_host_num[2]<=99) && (g_uc_host_num[2]==(~g_uc_host_num[3])))
	{//g_uc_host_num[3]存放的g_uc_host_num[2]的反码
			g_uc_machine_number[1]=g_uc_host_num[2];
			g_uc_machine_number_bak[1]=g_uc_host_num[2];
	}
	else
	{
		tmp_a = read_machine_num(1);//从flash读取机号
		if((tmp_a>0)&&(tmp_a<=99))
		{
			g_uc_machine_number[1]=tmp_a;
			g_uc_machine_number_bak[1]=tmp_a;
			
			g_uc_host_num[2]=g_uc_machine_number[1];
			s_uc_host_num_WR24C04(2,&g_uc_host_num[2]);
			
			g_uc_host_num[3]=255-g_uc_machine_number[1];
			s_uc_host_num_WR24C04(3,&g_uc_host_num[3]);
		}
		else
		{
			g_uc_is_init_machine_num[1]=true;
			s_uc_is_init_machine_num_WR24C04(1,&g_uc_is_init_machine_num[1]);
			g_uc_host_num[2]=DEFAULT_MACHINE_NUMBER1;
			s_uc_host_num_WR24C04(2,&g_uc_host_num[2]);
			
			g_uc_host_num[3]=255-DEFAULT_MACHINE_NUMBER1;
			s_uc_host_num_WR24C04(3,&g_uc_host_num[3]);
			
			g_uc_machine_number[1]=DEFAULT_MACHINE_NUMBER1;
			g_uc_machine_number_bak[1]=DEFAULT_MACHINE_NUMBER1;
		}
	}
	

	return (write_machine_number());//写到flash 并返回
}
///


void test_para(void)			//校验一些关键的数据
{
	u8 i,j;


	for(j=0;j<2;j++)
	{
		if(verify_oil_type(g_uc_gun_oil_type[j])==false)
		{
			g_uc_gun_oil_type[j]=OIL_10;
			s_uc_gun_oil_type_WR24C04(j,&g_uc_gun_oil_type[j]);
		}
						
		for(i=0;i<5;i++)
		{//
			if((g_ui_price_youhui[j][i]==0)||(g_ui_price_youhui[j][i]>9999))
			{
				g_ui_price_youhui[j][i]=100;
				s_ui_price_youhui_WR24C04(j,i,&g_ui_price_youhui[j][i]);
			}
		}
		
		if(g_ui_record_number[j]>MAX_RECORD)
		{
			g_ui_record_number[j]=MAX_RECORD;
			s_ui_record_number_WR24C04(j,&g_ui_record_number[j]);
		}
		
		if(g_ui_record_pointer[j]>=MAX_RECORD)
		{
			g_ui_record_pointer[j]=0;
			s_ui_record_pointer_WR24C04(j,&g_ui_record_pointer[j]);
		}
		
		if(g_uc_machine_number[j]!=g_uc_machine_number_bak[j])
		{
			init_machine_number();
		}
		if((g_uc_machine_number[j]==0)||(g_uc_machine_number[j]>99))
		{
			init_machine_number();
		}
	}
	//解灰
	if(g_uc_kill_gray_card_number>MAX_KILL_GRAY_CARD)
	{
		g_uc_kill_gray_card_number=MAX_KILL_GRAY_CARD;
		s_uc_kill_gray_card_number_WR24C04(&g_uc_kill_gray_card_number);
	}

	if(g_uc_kill_gray_card_pointer>=MAX_KILL_GRAY_CARD)
	{
		g_uc_kill_gray_card_pointer=0;
		s_uc_kill_gray_card_pointer_WR24C04(&g_uc_kill_gray_card_pointer);
	}
	//灰卡
	if(g_uc_mine_gray_card_number>MAX_GRAY_CARD)
	{
		g_uc_mine_gray_card_number=MAX_GRAY_CARD;
		s_uc_mine_gray_card_number_WR24C04(&g_uc_mine_gray_card_number);
	}

	if(g_uc_mine_gray_card_pointer>=MAX_GRAY_CARD)
	{
		g_uc_mine_gray_card_pointer=0;
		s_uc_mine_gray_card_pointer_WR24C04(&g_uc_mine_gray_card_pointer);
	}
	//充值
	if(g_uc_add_money_number>ADD_MONEY_CARD)
	{
		g_uc_add_money_number=ADD_MONEY_CARD;
		s_uc_add_money_number_WR24C04(&g_uc_add_money_number);
	}

	if(g_uc_add_money_pointer>=ADD_MONEY_CARD)
	{
		g_uc_add_money_pointer=0;
		s_uc_add_money_pointer_WR24C04(&g_uc_add_money_pointer);
	}
	
	s_uc_is_init_machine_num_RD24C04(&g_uc_is_init_machine_num[0],0);
	if((g_uc_is_init_machine_num[0]==false) && (g_uc_machine_number[0]==DEFAULT_MACHINE_NUMBER0))
	{
		g_uc_machine_number[0]=get_machineNum_from_record(0);					
		g_uc_machine_number_bak[0]=g_uc_machine_number[0];

		g_uc_host_num[0*2]=g_uc_machine_number[0];
		s_uc_host_num_WR24C04(0*2,&g_uc_host_num[0*2]);//存储机号 至24C04
		
		g_uc_host_num[0*2+1]=255-g_uc_machine_number[0];//存储机号反码 至24C04
		s_uc_host_num_WR24C04(0*2+1,&g_uc_host_num[0*2+1]);		
		write_machine_number();//存出机号到FLASH
	}
	s_uc_is_init_machine_num_RD24C04(&g_uc_is_init_machine_num[1],1);	
	if((g_uc_is_init_machine_num[1]==false) && (g_uc_machine_number[1]==DEFAULT_MACHINE_NUMBER1))
	{
		g_uc_machine_number[1]=get_machineNum_from_record(1);					
		g_uc_machine_number_bak[1]=g_uc_machine_number[1];

		g_uc_host_num[1*2]=g_uc_machine_number[1];
		s_uc_host_num_WR24C04(1*2,&g_uc_host_num[1*2]);//存储机号 至24C04
		
		g_uc_host_num[1*2+1]=255-g_uc_machine_number[1];//存储机号反码 至24C04
		s_uc_host_num_WR24C04(1*2+1,&g_uc_host_num[1*2+1]);		
		write_machine_number();//存出机号到FLASH
	}
	
}


/*******************************************************************************
* 函数：void system_overtime_monitor(u8 i)
* 功能：更改系统状态
* 参数：i 选择 为 0/1
* 返回：无
*******************************************************************************/
void system_overtime_monitor(void)
{
	u8 i;
	for(i=0;i<2;i++)
	{
			
		if(g_uc_system_choice[i]==g_uc_system_choice1[i])
		{
			g_uc_system_choice_bak[i]=g_uc_system_choice[i];
		}
		else
		{
			if(g_uc_system_choice_bak[i]==g_uc_system_choice1[i])
			{
				g_uc_system_choice[i]=g_uc_system_choice_bak[i];
			}
			else
			{
				if(g_uc_system_choice_bak[i]==g_uc_system_choice[i])
				{
					g_uc_system_choice1[i]=g_uc_system_choice[i];
				}
				else
				{//三个各不相等
					g_uc_system_choice[i]=g_uc_system_choice_bak[i];
					g_uc_system_choice1[i]=g_uc_system_choice[i];
				}
			}
		}
		
		
		//系统在各种状态下的超时处理
		if(g_uc_system_choice[i]==SYSTEM_INIT)	//0 系统初使化
		{
			g_ui_choice_time_out[i]=0;
		}
		else if(g_uc_system_choice[i]==SYSTEM_NOWORK)	//1 和PC脱机空闲状态
		{
			g_ui_choice_time_out[i]=0;
		}
		else if(g_uc_system_choice[i]==SYSTEM_VERIFY_CARD)		//2 卡插入，正在验证
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_10S)			//10秒和计算机无法通讯验证，则退出
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_BLACK_CARD)	//3 此卡为黑卡
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_5S)	//5秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_BLACK_NOCARD)		//4 无此卡的数据
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_5S)	//5秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_NO_GRAY_DATA)		//5 无此卡的解灰数据
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_5S)	//5秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_DEL_GRAY)		//6 解灰过程
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_5S)	//5秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_CARD_OK)		//7 卡验证完毕，可以加油
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_40S)	//40秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}		
		else if(g_uc_system_choice[i]==SYSTEM_ADDING_OIL_GRAY)		//8	有加油数据，加灰卡
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_10S)	//40秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_ADDING_OIL)	//9 正在加油
		{
			g_ui_choice_time_out[i]=0;
			test_oil_communication(i);//
		}
		else if(g_uc_system_choice[i]==SYSTEM_SUB_MONEY)	//10 加油结束，扣款解灰
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
		}
		else if(g_uc_system_choice[i]==SYSTEM_DOWNLOAD)		//11 下载卡号，5秒超时
		{
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>8571)	
			{//下载卡时间比较长，超时时间定位10分钟（70ms定时器）
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_CARD_OUT)		//12 加油过程中，卡被拔出
		{
			test_oil_communication(i);//未定义
		}
		else if(g_uc_system_choice[i]==SYSTEM_KEY_ADD_PREPARE)		//13	加油员卡，按键加油准备，按键之后进入此流程，进行加油机的允许加油的操作
		{//加油员卡，按键加油准备，按键之后进入此流程，进行加油机的允许加油的操作
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_20S)	//20秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else if(g_uc_system_choice[i]==SYSTEM_KEY_ADD)		//14 加油员卡，按键加油，按键之后加油过程的操作
		{//加油员卡，按键加油，按键之后加油过程的操作
			g_ui_choice_time_out[i]=0;
			test_oil_communication(i);
		}
		else if(g_uc_system_choice[i]==SYSTEM_NOWORK_JIAYOUYUAN)	//15 加油员卡验证通过后，等待状态
		{//加油员卡验证通过的空闲状态，等待按键或插卡的操作
			g_ui_choice_time_out[i]=g_ui_choice_time_out[i]+1;
			if(g_ui_choice_time_out[i]>TIME_25S)	//24秒
			{
				set_system_choice(i,SYSTEM_NOWORK);
			}
		}
		else
		{
			set_system_choice(i,SYSTEM_INIT);
		}
	
	}

}

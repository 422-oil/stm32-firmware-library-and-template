#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "stm32f10x.h"


u8 verify_bcd(u8 *pointer,u8 number);
void get_bcd(u8 number,u32 total_data,u8 *pointer);
u32 get_total(u8 number,u8 *pointer);

void set_system_choice(u8 choice, u8 state);

u8 verify_time(u8 *pointer);
u8 verify_indate(u8 year,u8 month,u8 day);
void set_rtc_clock(void);
void get_rtc_clock(void);
void init_system_time(void);

void init_static_var(void);
u8 begin_adding(u8 *pointer);

u8 init_machine_number(void);
void test_para(void);
void system_overtime_monitor(void);
#endif //__SYSTEM_H__

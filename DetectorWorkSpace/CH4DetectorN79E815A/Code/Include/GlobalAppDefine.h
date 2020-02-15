/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      
 * @version   V1.0.0
 * @brief     
 * @warning   
 *******************************************************************************
 * @remarks
 * 1. Version                Date                 Author
 *    v1.0.0                 2020年1月1日          Unknown
 *    Modification: 创建文档
 *******************************************************************************
 */
/* NEED FIX */
/*******************************************************************************
 *                 Multi-Include-Prevent Section
 ******************************************************************************/
#ifndef _GLOBAL_APP_DEFINE_H_
#define _GLOBAL_APP_DEFINE_H_

/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "GlobalFormat8051Core.h"
#include "N79E81x.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
/* -------- 主频相关宏定义 -------- */
#define FOSC_221184

/* -------- LED相关宏定义 -------- */
/* 电源灯关 */
#define LED_POWER_OFF (P03 = 1)
/* 电源灯开 */
#define LED_POWER_ON (P03 = 0)        
/* 电源灯翻转 */
#define LED_POWER_TOGGLE (P03 = ~P03) 

/* 故障灯关 */
#define LED_FAULT_OFF (P25 = 1)       
/* 故障灯开 */
#define LED_FAULT_ON (P25 = 0)        
/* 故障灯翻转 */
#define LED_FAULT_TOGGLE (P25 = ~P25) 

/* 报警灯关 */
#define LED_ALARM_OFF (P24 = 1)       
/* 报警灯开 */
#define LED_ALARM_ON (P24 = 0)        
/* 报警灯翻转 */
#define LED_ALARM_TOGGLE (P24 = ~P24) 

/* 传感器寿命灯关 */
#define LED_LIFE_OFF (P26 = 1)    
/* 传感器寿命灯开 */
#define LED_LIFE_ON (P26 = 0) 
/* 传感器寿命翻转 */
#define LED_LIFE_TOGGLE (P26 = ~P26)

/* -------- 蜂鸣器相关宏定义 -------- */
/* 蜂鸣器1开 */
#define SOUND1_ON (P21 = 1)        
/* 蜂鸣器1关 */
#define SOUND1_OFF (P21 = 0)       
/* 蜂鸣器1翻转 */
#define SOUND1_TOGGLE (P21 = ~P21) 

/* 蜂鸣器2开 */
#define SOUND2_ON (P20 = 1)        
/* 蜂鸣器2关 */
#define SOUND2_OFF (P20 = 0)       
/* 蜂鸣器2翻转 */
#define SOUND2_TOGGLE (P20 = ~P20) 

/* -------- 传感器使能控制宏定义 -------- */
/* (UNUSED) 传感器开 P2.7 常高 ZZZ*/
#define SENSER_ON (P3OUT |= 0x08)
/* (UNUSED) 传感器关 */
#define SENSER_OFF (P3OUT &= ~0x08)

/*  */
#define RXTIMEOUT 2
/*  */
#define RX_MIN_LENGTH 4
/*  */
#define UART0_RX_ENABLE SCON |= 0X10, RI = 0
/*  */
#define UART0_RX_DISABLE SCON &= ~0X10, RI = 0

/*
#define Dem_Adress        0x3600    //标定存储开始地址
#define DOT_Test_Adress   0x3900    //点检存储开始地址
#define Alarm_Adress      0x3A00    //报警存储开始地址
#define Fault_Adress      0x3D00    //故障存储开始地址
#define Down_Power_Adress 0x3DEF    //掉电存储开始地址
#define Up_Power_Adress   0x3F00    //上电存储开始地址
*/



/* 按键未按下 */
#define KEY_TR_HI (P07 = 1)
/* 按键按下 */  
#define KEY_TR_LOW (P07 = 0)

/* 继电器开 */
#define DELAY_ON (P22 = 1)
/* 继电器关 */
#define DELAY_OFF (P22 = 0)

/* 电磁阀关 */
#define VALVE_OFF (P00 = 0) 
/* 电磁阀开 */
#define VALVE_ON (P00 = 1)  

/* 电磁阀高电平检测 */
#define VALVE_TR_HI (P02 = 1) 
/* 电磁阀低电平检测 */
#define VALVE_TR_LW (P02 = 0) 

/* 标定高电平检测 */
#define DEMED_TR_HI (P06 = 1) 
/* 标定低电平检测 */
#define DEMED_TR_LW (P06 = 0) 

/* 3分钟 */
#define PREHEAT_TIME_COUNT 750

/*  */
#define POWER_DOWN_LIMIT 2

/* Unused */
#define SERIAL_ADDRESS 0x2FF0

/* 国标读命令的帧长度 */
#define GB_READ_FRAME_LEN 6
/* 国标读命令的最大种类数 */
#define GB_COMMAND_MAX 9

/* 所有存储项中的最大的那项记录条数 */
#define RecordLENMAX 20    

/******************************** 串口相关定义和设置 *****************************/
/* 串口相关定义和设置 */
/* 奇偶校验 0=偶校验 1=奇校验 */
#define EVEN_PARITY     0
#define ODD_PARITY      1
/* UART_Sel = 0 选择P1.0 P1.1 作为串口管脚 */
#define UAER_PORT_SELECT 0x00
/* UART设置为 偶校验(EVEN_PARITY) */
#define UART_PARITY_SET EVEN_PARITY

/* 时钟芯片的型号
    - 1 表示时钟芯片为PCF8563; 
    - 0 表示时钟芯片为RX8010SJ;
    */
#define PCF8563 0

/* 0xb00 0.标定数据存储地址 */
#define Dem_Flash_Adress   118 * 128    
/* 0xb80 2.报警数据存储地址 */
#define Alarm_Flash_Adress 119 * 128   
/* 0xc00 3.报警恢复数据存储地址 */
#define Alarm_Back_Adress  120 * 128 
/* 0xc80 4.故障数据存储地址 */
#define Fault_Flash_Adress 121 * 128 
/* 0xd00 5.故障恢复数据存储地址 */
#define Fault_Back_Adress  122 * 128  
/* 0xd80 6.掉电数据存储地址 */
#define Down_Flash_Adress  123 * 128
/* 0xe00 7.上电数据存储地址 */
#define Up_Flash_Adress    124 * 128
/* 0xe80 8.寿命到期存储地址 */
#define Life_Flash_Adress  125 * 128   
/* 0xf00 9.寿命超始时间,存储日月年，在老化时写入 */
#define Life_start_Adress  126 * 128
/* 0xf80 11.用于写入时备份用 */
#define copy_Adress        127 * 128                     

/*  */
#define Life_start_OFFSET_DEMA_sensor_ch4_0    10
#define Life_start_OFFSET_DEMA_sensor_ch4_3500 12

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 Prototype Declare Section
 ******************************************************************************/

#endif /* _GLOBAL_APP_DEFINE_H_ */
/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/


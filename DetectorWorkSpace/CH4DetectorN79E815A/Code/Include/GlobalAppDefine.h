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
 *    v1.0.0                 2020��1��1��          Unknown
 *    Modification: �����ĵ�
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
/* -------- ��Ƶ��غ궨�� -------- */
#define FOSC_221184

/* -------- LED��غ궨�� -------- */
/* ��Դ�ƹ� */
#define LED_POWER_OFF (P03 = 1)
/* ��Դ�ƿ� */
#define LED_POWER_ON (P03 = 0)        
/* ��Դ�Ʒ�ת */
#define LED_POWER_TOGGLE (P03 = ~P03) 

/* ���ϵƹ� */
#define LED_FAULT_OFF (P25 = 1)       
/* ���ϵƿ� */
#define LED_FAULT_ON (P25 = 0)        
/* ���ϵƷ�ת */
#define LED_FAULT_TOGGLE (P25 = ~P25) 

/* �����ƹ� */
#define LED_ALARM_OFF (P24 = 1)       
/* �����ƿ� */
#define LED_ALARM_ON (P24 = 0)        
/* �����Ʒ�ת */
#define LED_ALARM_TOGGLE (P24 = ~P24) 

/* �����������ƹ� */
#define LED_LIFE_OFF (P26 = 1)    
/* �����������ƿ� */
#define LED_LIFE_ON (P26 = 0) 
/* ������������ת */
#define LED_LIFE_TOGGLE (P26 = ~P26)

/* -------- ��������غ궨�� -------- */
/* ������1�� */
#define SOUND1_ON (P21 = 1)        
/* ������1�� */
#define SOUND1_OFF (P21 = 0)       
/* ������1��ת */
#define SOUND1_TOGGLE (P21 = ~P21) 

/* ������2�� */
#define SOUND2_ON (P20 = 1)        
/* ������2�� */
#define SOUND2_OFF (P20 = 0)       
/* ������2��ת */
#define SOUND2_TOGGLE (P20 = ~P20) 

/* -------- ������ʹ�ܿ��ƺ궨�� -------- */
/* (UNUSED) �������� P2.7 ���� ZZZ*/
#define SENSER_ON (P3OUT |= 0x08)
/* (UNUSED) �������� */
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
#define Dem_Adress        0x3600    //�궨�洢��ʼ��ַ
#define DOT_Test_Adress   0x3900    //���洢��ʼ��ַ
#define Alarm_Adress      0x3A00    //�����洢��ʼ��ַ
#define Fault_Adress      0x3D00    //���ϴ洢��ʼ��ַ
#define Down_Power_Adress 0x3DEF    //����洢��ʼ��ַ
#define Up_Power_Adress   0x3F00    //�ϵ�洢��ʼ��ַ
*/



/* ����δ���� */
#define KEY_TR_HI (P07 = 1)
/* �������� */  
#define KEY_TR_LOW (P07 = 0)

/* �̵����� */
#define DELAY_ON (P22 = 1)
/* �̵����� */
#define DELAY_OFF (P22 = 0)

/* ��ŷ��� */
#define VALVE_OFF (P00 = 0) 
/* ��ŷ��� */
#define VALVE_ON (P00 = 1)  

/* ��ŷ��ߵ�ƽ��� */
#define VALVE_TR_HI (P02 = 1) 
/* ��ŷ��͵�ƽ��� */
#define VALVE_TR_LW (P02 = 0) 

/* �궨�ߵ�ƽ��� */
#define DEMED_TR_HI (P06 = 1) 
/* �궨�͵�ƽ��� */
#define DEMED_TR_LW (P06 = 0) 

/* 3���� */
#define PREHEAT_TIME_COUNT 750

/*  */
#define POWER_DOWN_LIMIT 2

/* Unused */
#define SERIAL_ADDRESS 0x2FF0

/* ����������֡���� */
#define GB_READ_FRAME_LEN 6
/* ������������������� */
#define GB_COMMAND_MAX 9

/* ���д洢���е����������¼���� */
#define RecordLENMAX 20    

/******************************** ������ض�������� *****************************/
/* ������ض�������� */
/* ��żУ�� 0=żУ�� 1=��У�� */
#define EVEN_PARITY     0
#define ODD_PARITY      1
/* UART_Sel = 0 ѡ��P1.0 P1.1 ��Ϊ���ڹܽ� */
#define UAER_PORT_SELECT 0x00
/* UART����Ϊ żУ��(EVEN_PARITY) */
#define UART_PARITY_SET EVEN_PARITY

/* ʱ��оƬ���ͺ�
    - 1 ��ʾʱ��оƬΪPCF8563; 
    - 0 ��ʾʱ��оƬΪRX8010SJ;
    */
#define PCF8563 0

/* 0xb00 0.�궨���ݴ洢��ַ */
#define Dem_Flash_Adress   118 * 128    
/* 0xb80 2.�������ݴ洢��ַ */
#define Alarm_Flash_Adress 119 * 128   
/* 0xc00 3.�����ָ����ݴ洢��ַ */
#define Alarm_Back_Adress  120 * 128 
/* 0xc80 4.�������ݴ洢��ַ */
#define Fault_Flash_Adress 121 * 128 
/* 0xd00 5.���ϻָ����ݴ洢��ַ */
#define Fault_Back_Adress  122 * 128  
/* 0xd80 6.�������ݴ洢��ַ */
#define Down_Flash_Adress  123 * 128
/* 0xe00 7.�ϵ����ݴ洢��ַ */
#define Up_Flash_Adress    124 * 128
/* 0xe80 8.�������ڴ洢��ַ */
#define Life_Flash_Adress  125 * 128   
/* 0xf00 9.������ʼʱ��,�洢�����꣬���ϻ�ʱд�� */
#define Life_start_Adress  126 * 128
/* 0xf80 11.����д��ʱ������ */
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


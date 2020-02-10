/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      flash.h
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
#ifndef _FLASH_H_
#define _FLASH_H_

/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "GlobalFormat8051Core.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define  BYTES_OF_PAGE              128

/*  
    �±�0����ѯ�����¼����;
    �±�1��������¼ 200;        200 * 4 = 800 bytes  800 / 128 = 6.25 < 7
    �±�2�������ָ���¼ 200;     200 * 4 = 800 bytes   
    �±�3�����ϼ�¼ 100;        100 * 4 = 400 bytes  400 / 128 = 3.125 < 4
    �±�4�����ϻָ���¼ 100;     100 * 4 = 400 bytes
    �±�5�������¼ 50;         50 * 4 = 200 bytes   200 / 128 = 1.5625 < 2
    �±�6���ϵ��¼ 50;         50 * 4 = 200 bytes
    �±�7��������ʧЧ��¼ 1;     4 bytes
    �±�8���ڲ���ʱ����ǰʱ��

    800 * 2 + 400 * 2 + 200 * 2 = 2800
    (7 * 2 + 4 * 2 + 2 * 2) * 128 = 26 * 128 = 3328 
    (26 + 2) * 128 = 28 * 128 = 3584
    3328 / 1024 = 3.5k
    16 - 3.5 = 12.5k
*/

/* ���ƾŸ���ַ */
#define  ALARM_RECORD_ADDR          100 * 128 /* +7 */
#define  ALARM_RECOVERY_RECORD_ADDR 107 * 128 /* +7 */
#define  FAULT_RECORD_ADDR          114 * 128 /* +4 */
#define  FAULT_RECOVERY_RECORD_ADDR 118 * 128 /* +4 */
#define  POWER_DOWN_RECORD_ADDR     122 * 128 /* +2 */
#define  POWER_ON_RECORD_ADDR       124 * 128 /* +2 */
#define  TEMP_PAGE_ADDR             126 * 128 /* �����ڶ�ҳ */
#define  DEVICE_INFO_ADDR           127 * 128 /* ���һҳ */

/* FLASH�洢��¼�Ŀ�ʼ��ַ */
#define RECORD_START_ADDR           ALARM_RECORD_ADDR

/* ���� DEVICE_INFO_ADDR ����ĵ�ַ */
#define  PRODUCTION_DATE_ADDR       (DEVICE_INFO_ADDR + OFFSET_OF_PRODUCTION_DATE)
#define  ADC_VALUE_OF_CH4_0_ADDR    (DEVICE_INFO_ADDR + OFFSET_OF_CH4_0)
#define  ADC_VALUE_OF_CH4_3500_ADDR (DEVICE_INFO_ADDR + OFFSET_OF_CH4_3500)
#define  SENSOR_EXPIRED_RECORD_ADDR (DEVICE_INFO_ADDR + OFFSET_OF_SENSOR_EXPIRED)
#define  RTC_RECORD_ADDR            (DEVICE_INFO_ADDR + OFFSET_OF_RTC)

/* ��� DEVICE_INFO ҳ�ڵ�ַƫ�� */
#define  OFFSET_OF_PRODUCTION_DATE  0   /* 5 bytes */  
#define  OFFSET_OF_CH4_0            10  /* 2 bytes */  
#define  OFFSET_OF_CH4_3500         12  /* 2 bytes */ 
#define  OFFSET_OF_SENSOR_EXPIRED   14  /* ZZZ 5 bytes */
#define  OFFSET_OF_RTC              30  /* ZZZ 5 bytes */

/* �����¼������Ѱַ�����еı�� */
#define ALL_RECORD_COUNT        0
#define ALARM_RECORD            1
#define ALARM_RECOVERY_RECORD   2
#define FAULT_RECORD            3
#define FAULT_RECOVERY_RECORD   4
#define POWER_DOWN_RECORD       5
#define POWER_ON_RECORD         6
#define SENSOR_EXPIRED_RECORD   7
#define DEVICE_CURRENT_TIME     8

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 Prototype Declare Section
 ******************************************************************************/
/* ÿһ���¼���ܼ�¼�� */
extern code uint8_t max_of_each_record[9];

void flash_erase_page(uint16_t start_addr);
void flash_write_data(uint8_t *p, uint8_t len, uint16_t start_addr, uint8_t offset);
void flash_read_data(uint8_t *p, uint16_t start_addr, uint8_t len);

void flash_write_record(uint8_t record_type);
void flash_read_record(uint8_t record_type, uint8_t record_number);

#endif /* _FLASH_H_ */
/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

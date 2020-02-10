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
 *    v1.0.0                 2020年1月1日          Unknown
 *    Modification: 创建文档
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
    下标0：查询各类记录总数;
    下标1：报警记录 200;        200 * 4 = 800 bytes  800 / 128 = 6.25 < 7
    下标2：报警恢复记录 200;     200 * 4 = 800 bytes   
    下标3：故障记录 100;        100 * 4 = 400 bytes  400 / 128 = 3.125 < 4
    下标4：故障恢复记录 100;     100 * 4 = 400 bytes
    下标5：掉电记录 50;         50 * 4 = 200 bytes   200 / 128 = 1.5625 < 2
    下标6：上电记录 50;         50 * 4 = 200 bytes
    下标7：传感器失效记录 1;     4 bytes
    下标8：内部定时器当前时间

    800 * 2 + 400 * 2 + 200 * 2 = 2800
    (7 * 2 + 4 * 2 + 2 * 2) * 128 = 26 * 128 = 3328 
    (26 + 2) * 128 = 28 * 128 = 3584
    3328 / 1024 = 3.5k
    16 - 3.5 = 12.5k
*/

/* 共计九个地址 */
#define  ALARM_RECORD_ADDR          100 * 128 /* +7 */
#define  ALARM_RECOVERY_RECORD_ADDR 107 * 128 /* +7 */
#define  FAULT_RECORD_ADDR          114 * 128 /* +4 */
#define  FAULT_RECOVERY_RECORD_ADDR 118 * 128 /* +4 */
#define  POWER_DOWN_RECORD_ADDR     122 * 128 /* +2 */
#define  POWER_ON_RECORD_ADDR       124 * 128 /* +2 */
#define  TEMP_PAGE_ADDR             126 * 128 /* 倒数第二页 */
#define  DEVICE_INFO_ADDR           127 * 128 /* 最后一页 */

/* FLASH存储记录的开始地址 */
#define RECORD_START_ADDR           ALARM_RECORD_ADDR

/* 基于 DEVICE_INFO_ADDR 计算的地址 */
#define  PRODUCTION_DATE_ADDR       (DEVICE_INFO_ADDR + OFFSET_OF_PRODUCTION_DATE)
#define  ADC_VALUE_OF_CH4_0_ADDR    (DEVICE_INFO_ADDR + OFFSET_OF_CH4_0)
#define  ADC_VALUE_OF_CH4_3500_ADDR (DEVICE_INFO_ADDR + OFFSET_OF_CH4_3500)
#define  SENSOR_EXPIRED_RECORD_ADDR (DEVICE_INFO_ADDR + OFFSET_OF_SENSOR_EXPIRED)
#define  RTC_RECORD_ADDR            (DEVICE_INFO_ADDR + OFFSET_OF_RTC)

/* 相对 DEVICE_INFO 页内地址偏移 */
#define  OFFSET_OF_PRODUCTION_DATE  0   /* 5 bytes */  
#define  OFFSET_OF_CH4_0            10  /* 2 bytes */  
#define  OFFSET_OF_CH4_3500         12  /* 2 bytes */ 
#define  OFFSET_OF_SENSOR_EXPIRED   14  /* ZZZ 5 bytes */
#define  OFFSET_OF_RTC              30  /* ZZZ 5 bytes */

/* 各类记录类型在寻址数组中的标号 */
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
/* 每一项记录的总记录数 */
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

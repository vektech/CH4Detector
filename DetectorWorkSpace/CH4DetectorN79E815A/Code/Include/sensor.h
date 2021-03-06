/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      sensor.h
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
#ifndef _SENSOR_H_
#define _SENSOR_H_

/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "GlobalFormat8051Core.h"
/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
/* 传感器寿命 单位年 */
#define SENSOR_LIFE 5

/* 采集值过低故障 */
#define Short_Fault_L 35
/* 采集值过高故障 */
#define Short_Fault_H 1000

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 Prototype Declare Section
 ******************************************************************************/
/* 甲烷 零点AD值 */
extern uint16_t sensor_ch4_0;
/* 甲烷 3500点AD值 */
extern uint16_t sensor_ch4_3500;
/* 标定AD值存储数组 */
extern uint8_t sensor_demarcation_result[4];

/* 预热标记 */
extern bit sensor_preheat_flag;
/* 预热时间计数 */
extern uint16_t sensor_preheat_time_count;

/* 传感器寿命到期标志 false 未到期 */
extern bit sensor_expired_flag;

void sersor_demarcation(void);

#endif /* _SENSOR_H_ */
/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

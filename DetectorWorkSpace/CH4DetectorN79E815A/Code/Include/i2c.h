/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      i2c.h
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
#ifndef _I2C_H_ 
#define _I2C_H_

/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "GlobalFormat8051Core.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 Prototype Declare Section
 ******************************************************************************/
extern uint8_t i2c_time_code[7];

void i2c_init(void);
void i2c_start_rtc(void);
void i2c_get_time(void);
void i2c_set_time(void);

#endif /* _I2C_H_ */
/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      utilities.c
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
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "utilities.h"
#include "i2c.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/
uint8_t time_data[7];

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void store_device_time(void)
{
    /* second */
    time_data[0] = i2c_time_code[0];
    /* minute */
    time_data[1] = i2c_time_code[1];
    /* hour */
    time_data[2] = i2c_time_code[2];
    /* day */
    time_data[3] = i2c_time_code[3];
    /* week */
    /* 略去了i2c_time_code[4] 星期 */
    /* month */
    time_data[5] = i2c_time_code[5];
    /* year */
    time_data[6] = i2c_time_code[6];
}

/* 16进制转BCD码 */
uint8_t hex2bcd(uint8_t temp)
{
    return temp % 10 + (temp / 10) * 16;
}

/* BCD转16进制 BCD:0~99 */
uint8_t bcd2hex(uint8_t temp)
{
    return (temp / 16) * 10 + temp % 16;
}

uint8_t get_crc(uint8_t *p, uint8_t num)
{
    uint8_t i = 0;
    uint8_t sum = 0;
    for (i = 0; i < num - 2; i++)
        sum += *(p + i);
    return sum;
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

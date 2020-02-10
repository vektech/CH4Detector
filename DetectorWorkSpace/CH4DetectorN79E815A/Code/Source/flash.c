/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      flash.c
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
#include "N79E81x.h"
#include "flash.h"
#include "device.h"
#include "delay.h"
#include "i2c.h"

#include <intrins.h>

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define ERASE_ALL_DATA 0x22
#define FLASH_READ_DATA 0x00
#define BYTE_PROGRAM_DATA 0x21

#define ERASE_ALL_CONFIG 0xE2
#define FLASH_READ_CONFIG 0xC0
#define BYTE_PROGRAM_CONFIG 0xE1

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Prototype Declare Section ('function')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/
/*  
    下标0：查询各类记录总数;
    下标1：报警记录 200;
    下标2：报警恢复记录 200;
    下标3：故障记录 100;
    下标4：故障恢复记录 100;
    下标5：掉电记录 50;
    下标6：上电记录 50;
    下标7：传感器失效记录 1;
    下标8：内部定时器当前时间
*/

/* 每一项记录的总记录数 */
code uint8_t max_of_each_record[9] = {0xff,200,200,100,100,50,50,1,0xff};

code uint16_t RECORD_FIRST_ADDRESS[11] = 
{
    0xffff,                     /*!< 0.查询各类记录总数 */
    ALARM_RECORD_ADDR,          /*!< 1.报警记录 200 */
    ALARM_RECOVERY_RECORD_ADDR, /*!< 2.报警恢复记录 200 */
    FAULT_RECORD_ADDR,          /*!< 3.故障记录 100 */
    FAULT_RECOVERY_RECORD_ADDR, /*!< 4.故障恢复记录 100 */
    POWER_DOWN_RECORD_ADDR,     /*!< 5.掉电记录 50 */
    POWER_ON_RECORD_ADDR,       /*!< 6.上电记录 50 */
    SENSOR_EXPIRED_RECORD_ADDR, /*!< 7.传感器失效记录 1 */
    0xffff,                     /*!< 8.内部定时器当前时间 */
    TEMP_PAGE_ADDR,             /*!< 9.用于备份存储临时数据的FLASH地址 */
    DEVICE_INFO_ADDR            /*!< 10.设备信息存储地址 */
};

/* 存储压缩之后的时间结果 */
uint8_t zipped_time[4] = {0};
/* 存储解压之后的时间结果 */
uint8_t unzipped_time[5] = {0};

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
/* 打开ISP模式 */
void Enable_ISP_Mode(void)
{
    /* Record EA Status */
    EA_save_bit = EA;
    /* 关闭全局中断 */
    EA = 0;

    /* 关键SFR读写保护 */
    TA = 0xAA;
    TA = 0x55;
    /* 将ISPEN置1 开启ISP功能 
        - 使能ISP功能将以内部的22.1184MHz RC 振荡器为时钟
        - 清ISPEN应该是ISP操作后的最后一条指令 这样可以停止内部RC以减少功耗
        */
    CHPCON |= 0x01;
    /* Resume EA */
    EA = EA_save_bit;
}

/* 关闭ISP模式 */
void Disable_ISP_Mode(void)
{
    /* Disable ISP */
    /* Record EA Status */
    EA_save_bit = EA;
    /* 关闭全局中断 */
    EA = 0;

    /* ISP 功能的关键SFR读写保护 */
    TA = 0xAA;
    TA = 0x55;
    /* 将ISPEN置0 关闭ISP功能 
        - 使能ISP功能将以内部的22.1184MHz RC 振荡器为时钟
        - 清ISPEN应该是ISP操作后的最后一条指令 这样可以停止内部RC以减少功耗
        */
    CHPCON &= 0xFE;
    /* Resume EA */
    EA = EA_save_bit;
}

/* 触发ISP功能 以执行ISP指令 */
void Trigger_ISP(void)
{
    /* Record EA Status */
    EA_save_bit = EA;
    /* 关闭全局中断 */
    EA = 0;

    /* ISP 功能的关键SFR读写保护 */
    TA = 0xAA;
    TA = 0x55;
    /* ISPGO -> ISPTRG[0] 置位 执行ISP
        - 设置该位为1开始执行ISP 该指令后 CPU保持程序计数器PC ISP硬件自动管理控制该过程
        - ISP完成后 程序计数器继续执行 ISPGO位自动清零 保持为0
        */
    ISPTRG |= 0x01;
    /* Resume EA */
    EA = EA_save_bit;
}

/* Flash 页擦除 */
void flash_erase_page(uint16_t start_addr)
{
    Enable_ISP_Mode();

    /* ISPCN = 0010 0010B ISP控制 选择操作APROM或者数据FLASH 进行FLASH页擦除
        - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
        - [5] FOEN      = 1B 选择 页擦除 或者 编程; 0B 选择 读数据;
        - [4] FCEN      = 0B 无其他设置 只能为0;
        - [3:0] FCTRL   = 0010B 页擦除; 0001B 编程; 0000B 读数据;
        */
    ISPCN = ERASE_ALL_DATA;

    /* ISPAL ISP地址低字节 */
    ISPAL = LOBYTE(start_addr);
    /* ISPAH ISP地址高字节*/
    ISPAH = HIBYTE(start_addr);

    Trigger_ISP();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();

    Disable_ISP_Mode();
}

/* 向Flash中写数据 
    - p          源数据地址
    - len        源数据长度
    - start_addr 数据写入的起始地址
    - offset     相较于起始地址的偏移量
    */
void flash_write_data(uint8_t *p, uint8_t len, uint16_t start_addr, uint8_t offset)
{
    uint8_t j;

    if (!len)
        return;
    if (offset >= 128)
        return;
    if ((len + offset) >= 128)
        return;

    /* 擦除 TEMP_PAGE_ADDR 用于备份 */
    flash_erase_page(TEMP_PAGE_ADDR);

    Enable_ISP_Mode();

    /* 将需要更新的页拷贝进 TEMP_PAGE 中 */
    for (j = 0; j < 128; j++)
    {
        /* ISPCN = 0000 0000B ISP控制 选择操作APROM或者数据FLASH 进行FLASH读数据
            - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
            - [5] FOEN      = 0B 选择 读数据; 1B 选择 页擦除 或者 编程;
            - [4] FCEN      = 0B 无其他设置 只能为0;
            - [3:0] FCTRL   = 0000B 读数据; 0010B 页擦除; 0001B 编程; 
            */
        ISPCN = FLASH_READ_DATA;
        /* ISPAL ISP地址低字节 */
        ISPAL = LOBYTE(start_addr + j);
        /* ISPAH ISP地址高字节 */
        ISPAH = HIBYTE(start_addr + j);

        Trigger_ISP();        
        delay_1ms(0);

        /* ISPCN = 0010 0001B ISP控制 选择操作 APROM或者数据FLASH 进行FLASH编程
            - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
            - [5] FOEN      = 1B 选择 页擦除 或者 编程; 0B 选择 读数据;
            - [4] FCEN      = 0B 无其他设置 只能为0;
            - [3:0] FCTRL   = 0001B 编程; 0000B 读数据; 0010B 页擦除; 
            */
        ISPCN = BYTE_PROGRAM_DATA;
        /* ISPAL ISP地址低字节 */
        ISPAL = LOBYTE(TEMP_PAGE_ADDR + j);
        /* ISPAH ISP地址高字节 */
        ISPAH = HIBYTE(TEMP_PAGE_ADDR + j);
        
        Trigger_ISP();
        delay_1ms(0);
    }

    Disable_ISP_Mode();

    /* 擦除需要更新的页 */
    flash_erase_page(start_addr);

    Enable_ISP_Mode();

    for (j = 0; j < 128; j++)
    {
        /* 1. 所要更新的地址序列 到达地址偏移之后 增量添加新数据 长度为len */
        if (j >= offset && (j < (len + offset)))
        {
            /* ISPCN = 0010 0001B ISP控制 选择操作 APROM或者数据FLASH 进行FLASH编程
                - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
                - [5] FOEN      = 1B 选择 页擦除 或者 编程; 0B 选择 读数据;
                - [4] FCEN      = 0B 无其他设置 只能为0;
                - [3:0] FCTRL   = 0001B 编程; 0000B 读数据; 0010B 页擦除; 
                */
            ISPCN = BYTE_PROGRAM_DATA;
            /* ISPAL ISP地址低字节 */
            ISPAL = LOBYTE(start_addr + j);
            /* ISPAH ISP地址高字节 */
            ISPAH = HIBYTE(start_addr + j);
            /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
                - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
                - 读/校验模式下 在ISP完成后从ISPFD读出数据
            */
            ISPFD = *p++;
        }
        else
        {
            /* 2. 不是所需要更新的地址序列 读取暂存页中的数据 */
            /* ISPCN = 0000 0000B ISP控制 选择操作APROM或者数据FLASH 进行FLASH读数据
                - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
                - [5] FOEN      = 0B 选择 读数据; 1B 选择 页擦除 或者 编程;
                - [4] FCEN      = 0B 无其他设置 只能为0;
                - [3:0] FCTRL   = 0000B 读数据; 0010B 页擦除; 0001B 编程; 
                */
            ISPCN = FLASH_READ_DATA;
            /* ISPAL ISP地址低字节 */
            ISPAL = LOBYTE(TEMP_PAGE_ADDR + j);
            /* ISPAH ISP地址高字节 */
            ISPAH = HIBYTE(TEMP_PAGE_ADDR + j);

            Trigger_ISP();

            /* 3. 不是所需要更新的地址序列 将读取的暂存页中的数据 写入需要更新的页中 */
            /* ISPCN = 0010 0001B ISP控制 选择操作 APROM或者数据FLASH 进行FLASH编程
                - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
                - [5] FOEN      = 1B 选择 页擦除 或者 编程; 0B 选择 读数据;
                - [4] FCEN      = 0B 无其他设置 只能为0;
                - [3:0] FCTRL   = 0001B 编程; 0000B 读数据; 0010B 页擦除; 
                */
            ISPCN = BYTE_PROGRAM_DATA;
            /* ISPAL ISP地址低字节 */
            ISPAL = LOBYTE(start_addr + j);
            /* ISPAH ISP地址高字节 */
            ISPAH = HIBYTE(start_addr + j);
        }

        Trigger_ISP();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
    }
    
    Disable_ISP_Mode();
}

/* 从Flash中读取数据
    - p          存放所读数据的地址
    - start_addr 读取数据的起始地址
    - len        读数据的长度
    */
void flash_read_data(uint8_t *p, uint16_t start_addr, uint8_t len)
{
    if (!len)
        return;

    Enable_ISP_Mode();

    /* ISPCN = 0000 0000B ISP控制 选择操作APROM或者数据FLASH 进行FLASH读数据
        - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
        - [5] FOEN      = 0B 选择 读数据; 1B 选择 页擦除 或者 编程;
        - [4] FCEN      = 0B 无其他设置 只能为0;
        - [3:0] FCTRL   = 0000B 读数据; 0010B 页擦除; 0001B 编程; 
        */
    ISPCN = FLASH_READ_DATA;
    /* ISPAL ISP地址低字节 */
    ISPAL = LOBYTE(start_addr);
    /* ISPAH ISP地址高字节 */
    ISPAH = HIBYTE(start_addr);

    while (len--)
    {
        Trigger_ISP();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();

        /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
            - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
            - 读/校验模式下 在ISP完成后从ISPFD读出数据
        */
        *p++ = ISPFD;
        /* ISPAL ISP地址低字节加1 */
        ISPAL++;
    }

    Disable_ISP_Mode();
}

/* 压缩当前时间 因每次存记录时都要读取当前的时间 */
void zip_current_time(uint8_t *p)
{
    uint8_t i;

    /* 暂时存储压缩之后的时间 */
    uint8_t temp_zipped_time[4] = {0};
    /* 暂时存储解压之后的时间 */
    uint8_t temp_unzipped_time[5] = {0};

    /* 调用一次读时间函数 约耗时500uS */
    i2c_get_time();

    /* second */
    // i2c_time_code[0];
    /* minute */
    temp_unzipped_time[4] = i2c_time_code[1];
    /* hour */
    temp_unzipped_time[3] = i2c_time_code[2];
    /* day */
    temp_unzipped_time[2] = i2c_time_code[3];
    /* week */
    // i2c_time_code[4];
    /* month */
    temp_unzipped_time[1] = i2c_time_code[5];
    /* year */
    temp_unzipped_time[0] = i2c_time_code[6];

    /* 通过移位压缩 */

    /* flag & year 0000 00|11 */
    temp_zipped_time[0] = (temp_unzipped_time[0] >> 4);
    /* year & month 1111|1111 */
    temp_zipped_time[1] = temp_unzipped_time[1];
    temp_zipped_time[1] |= (temp_unzipped_time[0] << 4);
    /* day & hour 1111 1|111 */
    temp_zipped_time[2] = temp_unzipped_time[2];
    temp_zipped_time[2] <<= 3;
    temp_zipped_time[2] |= (temp_unzipped_time[3] >> 2);
    /* hour & minute 11|11 1111 */
    temp_zipped_time[3] = temp_unzipped_time[4];
    temp_zipped_time[3] |= (temp_unzipped_time[3] << 6);

    /* 拷贝至目标数组中 */
    for ( i = 0; i < 4; i++)
    {
        *(p + i) = temp_zipped_time[i];
    }    
}

void unzip_time(uint8_t *p, uint8_t *q)
{
    uint8_t i;

    /* 暂时存储压缩之后的时间 */
    uint8_t temp_zipped_time[4] = {0};
    /* 暂时存储解压之后的时间 */
    uint8_t temp_unzipped_time[5] = {0};

    /* 拷贝至目标数组中 */
    for ( i = 0; i < 4; i++)
    {
        temp_zipped_time[i] = *(p + i);
    }

    /* flag & year 0000 00|11 */
    temp_unzipped_time[0] = ((temp_zipped_time[0] << 4) | (temp_zipped_time[1] >> 4));
    /* year & month 1111|1111 */
    temp_unzipped_time[1] = (temp_zipped_time[1] & 0x0F);
    /* day & hour 1111 1|111 */
    temp_unzipped_time[2] = ((temp_zipped_time[2] & 0xF8) >> 3);
    /* hour & minute 11|11 1111 */
    temp_unzipped_time[3] = (((temp_zipped_time[2] & 0x07) << 2) | ((temp_zipped_time[3] & 0xC0) >> 6));
    temp_unzipped_time[4] = (temp_zipped_time[3] & 0x3F);

    /* 拷贝至目标数组中 */
    for ( i = 0; i < 5; i++)
    {
        *(q + i) = temp_unzipped_time[i];
    }
}

/*******************************************************************************  
* 函 数 名: uint8_t ReadRecordTotal(uint8_t FirstAddr_index)
* 功能描述: 
* 函数说明: 无
* 调用函数: 无
* 全局变量: 无
* 输    入: FirstAddr_index:（是这个数组RECORD_FIRST_ADDRESS的下标），

* 返    回:  
* 设 计 者： 杜瑞杰                  日期：2015.08.18
* 修 改 者：                         日期：
* 版    本：V1.00
*使用注意事项：
1、既然是下标值，则不能大于数组的下标索引，否则该函数什么也不做。
********************************************************************************/
// uint8_t ReadRecordTotal(uint8_t FirstAddr_index)
// {
//     uint8_t i, k1, k2;

//     /* 读取数据类型超范围 XXX 应大于最后的类型即第8类 返回0 */
//     if (FirstAddr_index <= 0 || (FirstAddr_index >= (sizeof(RECORD_FIRST_ADDRESS) - 1)))
//         return 0;

//     Enable_ISP_Mode();
//     /* ISPCN = 0000 0000B ISP控制 选择操作APROM或者数据FLASH 进行FLASH读数据
//         - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
//         - [5] FOEN      = 0B 选择 读数据; 1B 选择 页擦除 或者 编程;
//         - [4] FCEN      = 0B 无其他设置 只能为0;
//         - [3:0] FCTRL   = 0000B 读数据; 0010B 页擦除; 0001B 编程; 
//         */
//     ISPCN = FLASH_READ_DATA;
//     /* ISPAL ISP地址低字节 */
//     ISPAL = LOBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);
//     /* ISPAH ISP地址高字节 */
//     ISPAH = HIBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);

//     k1 = 0;
//     /* XXX  计算某类记录的总条数 */
//     for (i = 0; i < RecordLEN[FirstAddr_index]; i++)
//     {
//         Trigger_ISP();

//         /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//             - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//             - 读/校验模式下 在ISP完成后从ISPFD读出数据
//         */
//         k2 = ISPFD;
//         /* 如果读取的第一条记录的记录号 大于此类型的最大记录数 或者小于等于k1(上一条的记录号) 或者等于0 */
//         if (k2 > RecordLEN[FirstAddr_index] || (k2 <= k1) || (k2 == 0))
//         {
//             /* 表示存储的记录第一遍还没存满 如 1 0xff 0xff (共3条记录的记录号) */
//             Disable_ISP_Mode();
//             return i;
//         }

//         /* ISPAL ISP地址低字节 加上此类型单条记录所占的字节数 即下一条记录的起始地址 */
//         ISPAL += NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];

//         /* 使用k1 存储此条记录的记录号 */
//         k1 = k2;
//         /* Brown-Out Detector 电源电压检测 */
//         check_BOD();
//     }
//     Disable_ISP_Mode();

//     /* 表示记录已存满 且首地址存的那条记录的记录号正好是从1开始 递增到RecordLEN[FirstAddr_index] 如：1 2 3 (共3条记录的记录号) */
//     if (i == RecordLEN[FirstAddr_index])
//         return RecordLEN[FirstAddr_index];

//     return 0;
// }

/*******************************************************************************  
* 函 数 名: void ReadRecordData(uint8_t FirstAddr_index, uint8_t recordnumber)
* 功能描述: 根据一个首地址的下标值,和记录号，来读一条记录。
* 函数说明: 无
* 调用函数: 无
* 全局变量: 无
* 输    入: FirstAddr_index:（是这个数组RECORD_FIRST_ADDRESS的下标），
            recordnumber:记录号是不能大于RecordLEN[FirstAddr_index]的。
* 返    回:  
* 设 计 者： 杜瑞杰                  日期：2015.08.18
* 修 改 者：                         日期：
* 版    本：V1.00
*使用注意事项：
1、既然是下标值，则不能大于数组的下标索引，否则该函数什么也不做。
********************************************************************************/
// void ReadRecordData(uint8_t FirstAddr_index, uint8_t recordnumber)
// {
//     uint8_t i, j = FirstAddr_index & 0x80;

//     FirstAddr_index = FirstAddr_index & 0x7f;
//     if ((FirstAddr_index >= (sizeof(RECORD_FIRST_ADDRESS))))
//         return;
//     if (recordnumber > RecordLEN[FirstAddr_index])
//         return;

//     demaAD[0] = demaAD[1] = demaAD[2] = demaAD[3] = 0;

//     switch (FirstAddr_index)
//     {
//         /* Check 查询各类记录总数 */
//         case 0:
//         {
//             rxbuf[0] = 0xaa;
//             rxbuf[1] = 0;
//             rxbuf[2] = 0;
//             rxbuf[3] = 8;
//             /* XXX 旧版按照8类数据计算 新版需改为7类 */
//             for (i = 1; i <= 8; i++)
//             {   
//                 /* 调用FLASH读总记录函数 */
//                 rxbuf[i + 3] = ReadRecordTotal(i);
//             }
//             rxbuf[12] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//             rxbuf[13] = 0x55;

//             for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//             {
//                 UART_SEND(rxbuf[i]);
//                 /* 串口输出延时函数 关键参数 不可少 */
//                 delay_1ms(5);
//             }
//             break;
//         }
//         /* Check 查询探测器的当前时间 */
//         case 9:
//         {
//             rxbuf[0] = 0xaa;
//             rxbuf[1] = 0;
//             rxbuf[2] = 9;
//             rxbuf[3] = 6;

//             /* 调用一次读时间函数 约耗时500uS */
//             i2c_get_time();

//             rxbuf[4] = (2000 + i2c_time_code[6]) / 256;
//             rxbuf[5] = (2000 + i2c_time_code[6]) % 256;
//             rxbuf[6] = i2c_time_code[5];
//             rxbuf[7] = i2c_time_code[3];
//             rxbuf[8] = i2c_time_code[2];
//             rxbuf[9] = i2c_time_code[1];
//             rxbuf[10] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//             rxbuf[11] = 0x55;
//             for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//             {
//                 UART_SEND(rxbuf[i]);
//                 /* 串口输出延时函数 关键参数 不可少 */
//                 delay_1ms(5);
//             }
//             break;
//         }
//         /* 查询单项记录号为recordnumber的记录 */
//         default:
//         {
//             /* 非数据记录查询 */
//             if (recordnumber == 0)
//                 return;

//             Enable_ISP_Mode();

//             /* ISPCN = 0000 0000B ISP控制 选择操作APROM或者数据FLASH 进行FLASH读数据
//                 - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
//                 - [5] FOEN      = 0B 选择 读数据; 1B 选择 页擦除 或者 编程;
//                 - [4] FCEN      = 0B 无其他设置 只能为0;
//                 - [3:0] FCTRL   = 0000B 读数据; 0010B 页擦除; 0001B 编程; 
//                 */
//             ISPCN = FLASH_READ_DATA;
//             /* ISPAL ISP地址低字节 根据类型计算数据首地址 */
//             ISPAL = LOBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);
//             /* ISPAH ISP地址高字节 根据类型计算数据首地址 */
//             ISPAH = HIBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);

//             /* 查询的记录数要小于等于该记录所规定的总条数 */
//             for (i = 0; i < RecordLEN[FirstAddr_index]; i++)
//             {
//                 Trigger_ISP();
//                 delay_1ms(0);

//                 /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                     - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                     - 读/校验模式下 在ISP完成后从ISPFD读出数据
//                 */
//                 rxbuf[1] = ISPFD;
//                 /* 查询到所需的记录号则退出 */
//                 if (rxbuf[1] == recordnumber)
//                 {
//                     break;
//                 }
//                 /* ISPAL ISP地址低字节 加上此类型单条记录所占的字节数 即下一条记录的起始地址 */
//                 ISPAL += NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//             }

//             /* 若查询到的记录号小于等于该记录所规定的总条数 */
//             if (i < RecordLEN[FirstAddr_index])
//             {
//                 /* 若读取的记录类型为标定值 */
//                 if (FirstAddr_index == DEM_RECORD)
//                 {
//                     for (i = 0; i < 4; i++)
//                     {
//                         /* ISPAL ISP地址低字节 加1 */
//                         ISPAL++;
//                         /* 读取标定值 */
//                         Trigger_ISP();
//                         _nop_();
//                         _nop_();
//                         _nop_();
//                         _nop_();
//                         _nop_();
//                         delay_1ms(0);
//                         /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                             - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                             - 读/校验模式下 在ISP完成后从ISPFD读出数据
//                         */
//                         /* 将标定值读入数组中 */
//                         demaAD[i] = ISPFD;
//                     }
//                 }

//                 /* 此处发送 年 月 日 时 分 共5个字节的数据 */
//                 for (i = 10; i > 5; i--)
//                 {
//                     /* ISPAL ISP地址低字节 */
//                     ISPAL++;
//                     Trigger_ISP();
//                     _nop_();
//                     _nop_();
//                     _nop_();
//                     _nop_();
//                     _nop_();
//                     delay_1ms(0);
//                     /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                         - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                         - 读/校验模式下 在ISP完成后从ISPFD读出数据
//                     */
//                     rxbuf[i] = ISPFD;
//                 }
//                 Disable_ISP_Mode();

//                 /* 该条记录的记录号n1 */
//                 /* 年高字节 */
//                 rxbuf[5] = (rxbuf[6] + 2000) / 256;
//                 /* 年低字节 */
//                 rxbuf[6] = (rxbuf[6] + 2000) % 256;
//                 rxbuf[0] = 0xaa;
//                 rxbuf[2] = rxbuf[2];
//                 rxbuf[3] = FRAME_DATA_LEN[FirstAddr_index];
//                 rxbuf[4] = rxbuf[1];
//                 rxbuf[11] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//                 rxbuf[12] = 0x55;
//                 if (!j)
//                 {
//                     for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//                     {
//                         UART_SEND(rxbuf[i]);
//                         /* 串口输出延时函数 关键参数 不可少 */
//                         delay_1ms(5);
//                     }
//                 }
//             }
//             /* 若查询到的记录号大于该记录所规定的总条数 */
//             else
//             {
//                 /* 若要读取的是传感器寿命 XXX 需进行修改*/
//                 if (FirstAddr_index == LIFE_RECORD)
//                 {
//                     /* 此处应传输传感器的失效日期 n1表示是否失效 
//                         - n1 = 0 未失效 失效日期均为0 
//                         - n1 = 1 已失效 失效日期为n2 - n7 */
//                     rxbuf[0] = 0xaa;
//                     rxbuf[1] = 1;
//                     rxbuf[3] = FRAME_DATA_LEN[FirstAddr_index];
//                     rxbuf[4] = 0;
//                     rxbuf[5] = 0;
//                     rxbuf[6] = 0;
//                     rxbuf[7] = 0;
//                     rxbuf[8] = 0;
//                     rxbuf[9] = 0;
//                     rxbuf[10] = 0;
//                     rxbuf[11] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//                     rxbuf[12] = 0x55;
//                     for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//                     {
//                         UART_SEND(rxbuf[i]);
//                         /* 串口输出延时函数 关键参数 不可少 */
//                         delay_1ms(5);
//                     }
//                 }
//                 Disable_ISP_Mode();
//                 return;
//             }
//             break;
//         }
//     }
// }

/*******************************************************************************  
* 函 数 名: void WriteRecordData(uint8_t FirstAddr_index)
* 功能描述: 根据一个首地址的下标值,来写一条记录。
* 函数说明: 无
* 调用函数: 无
* 全局变量: 无
* 输    入: FirstAddr_index:（是这个数组RECORD_FIRST_ADDRESS的下标），

* 返    回:  
* 设 计 者： 杜瑞杰                  日期：2015.08.18
* 修 改 者：                         日期：
* 版    本：V1.00
*使用注意事项：
1、既然是下标值，则不能大于数组的下标索引，否则该函数什么也不做。
********************************************************************************/
// void WriteRecordData(uint8_t FirstAddr_index)
// {
//     uint8_t i, j, k, cnt;
//     uint16_t writeaddres, readaddres;

//     if (FirstAddr_index <= 0 || (FirstAddr_index >= (sizeof(RECORD_FIRST_ADDRESS) - 1)))
//         return;

//     /* Erase copy_Adress 用于写入时备份用 */
//     flash_erase_page(TEMP_PAGE_ADDR);
    
//     /* copy to copy_Adress */
//     i = 0;
//     cnt = 0;
//     for (j = 0; j < RecordLEN[FirstAddr_index]; j++)
//     {
//         /* 计算该类型记录中每条记录的首地址 */
//         writeaddres = RECORD_FIRST_ADDRESS[FirstAddr_index] + j * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//     REREAD:
//         Enable_ISP_Mode();
//         /* ISPCN = 0000 0000B ISP控制 选择操作APROM或者数据FLASH 进行FLASH读数据
//             - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
//             - [5] FOEN      = 0B 选择 读数据; 1B 选择 页擦除 或者 编程;
//             - [4] FCEN      = 0B 无其他设置 只能为0;
//             - [3:0] FCTRL   = 0000B 读数据; 0010B 页擦除; 0001B 编程; 
//             */
//         ISPCN = FLASH_READ_DATA;
//         /* ISPAL ISP地址低字节 */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP地址高字节 */
//         ISPAH = HIBYTE(writeaddres);

//         /* 读一条记录 */
//         for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//         {
//             Trigger_ISP();
//             /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                 - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                 - 读/校验模式下 在ISP完成后从ISPFD读出数据
//             */
//             copyeep[k] = ISPFD;
//             /* ISPAL ISP地址低字节 */
//             ISPAL++;
//         }
//         Disable_ISP_Mode();

//         /* copyeep[0] 为该条记录的记录标号 */
//         if ((copyeep[0] != (j + 1)) || copyeep[0] > RecordLEN[FirstAddr_index] || (copyeep[0] <= i) || (copyeep[0] == 0))
//         {
//             /* 表示存储的记录第一遍还没存满。如 1 0xff 0xff 如共3条记录的记录号 */
//             if (copyeep[0] != 0xff)
//             {
//                 if (++cnt <= 2)
//                 {
//                     Delay_1ms(100);
//                     goto REREAD;
//                 }
//             }

//             break;
//         }
//         i = copyeep[0];

//         /* 计算写入的地址 */
//         writeaddres = TEMP_PAGE_ADDR + j * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         Enable_ISP_Mode();

//         /* ISPCN = 0010 0001B ISP控制 选择操作 APROM或者数据FLASH 进行FLASH编程
//             - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
//             - [5] FOEN      = 1B 选择 页擦除 或者 编程; 0B 选择 读数据;
//             - [4] FCEN      = 0B 无其他设置 只能为0;
//             - [3:0] FCTRL   = 0001B 编程; 0000B 读数据; 0010B 页擦除; 
//             */
//         ISPCN = BYTE_PROGRAM_DATA;
//         /* ISPAL ISP地址低字节 */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP地址高字节 */
//         ISPAH = HIBYTE(writeaddres);

//         /* 写入一条记录至 copy_Adress */
//         for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//         {
//             /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                 - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                 - 读/校验模式下 在ISP完成后从ISPFD读出数据
//             */
//             ISPFD = copyeep[k];
//             Trigger_ISP();
//             _nop_();
//             _nop_();
//             _nop_();
//             _nop_();
//             _nop_();
//             /* ISPAL ISP地址低字节 */
//             ISPAL++;
//         }
//         Disable_ISP_Mode();
//     }

//     /* XXX 擦除该记录类型所在的页 */
//     flash_erase_page(RECORD_FIRST_ADDRESS[FirstAddr_index]);

//     /* XXX */
//     if (j < RecordLEN[FirstAddr_index])
//         readaddres = TEMP_PAGE_ADDR;
//     else
//     {
//         /* XXX */
//         readaddres = TEMP_PAGE_ADDR + NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         /* XXX 满记录 */
//         j = RecordLEN[FirstAddr_index] - 1;
//     }

//     /* 在备份页中循环读记录 和 写记录在该类型所属的Flash地址中 */
//     for (i = 0; i < j; i++)
//     {
//         /* 写地址 = 读地址 + 当前记录数i * 每条记录的长度 */
//         writeaddres = readaddres + i * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         Enable_ISP_Mode();
//         /* ISPCN = 0000 0000B ISP控制 选择操作APROM或者数据FLASH 进行FLASH读数据
//             - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
//             - [5] FOEN      = 0B 选择 读数据; 1B 选择 页擦除 或者 编程;
//             - [4] FCEN      = 0B 无其他设置 只能为0;
//             - [3:0] FCTRL   = 0000B 读数据; 0010B 页擦除; 0001B 编程; 
//             */
//         ISPCN = FLASH_READ_DATA;
//         /* ISPAL ISP地址低字节 */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP地址高字节 */
//         ISPAH = HIBYTE(writeaddres);

//         /* 从备份页 XXX 的写地址 中读取一条记录 存入copyeep[]中 */
//         for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//         {
//             Trigger_ISP();
//             /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                 - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                 - 读/校验模式下 在ISP完成后从ISPFD读出数据
//             */
//             copyeep[k] = ISPFD;
//             /* ISPAL ISP地址低字节 */
//             ISPAL++;
//         }
//         Disable_ISP_Mode();

//         /* 写地址 = 该记录类型首地址 + 当前记录数i * 每条记录的长度 */
//         writeaddres = RECORD_FIRST_ADDRESS[FirstAddr_index] + i * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         Enable_ISP_Mode();

//         /* ISPCN = 0010 0001B ISP控制 选择操作 APROM或者数据FLASH 进行FLASH编程
//             - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
//             - [5] FOEN      = 1B 选择 页擦除 或者 编程; 0B 选择 读数据;
//             - [4] FCEN      = 0B 无其他设置 只能为0;
//             - [3:0] FCTRL   = 0001B 编程; 0000B 读数据; 0010B 页擦除; 
//             */
//         ISPCN = BYTE_PROGRAM_DATA;
//         /* ISPAL ISP地址低字节 */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP地址高字节 */
//         ISPAH = HIBYTE(writeaddres);

//         /* 写 第一条 记录至该记录类型的FLASH中 */
//         /* 读地址大于备份页的首地址 */
//         if (readaddres > TEMP_PAGE_ADDR)
//         {
//             /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                 - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                 - 读/校验模式下 在ISP完成后从ISPFD读出数据
//             */
//             /* 新的记录标号 */
//             ISPFD = i + 1;
//             Trigger_ISP();

//             /* ISPAL ISP地址低字节 */
//             ISPAL++;
//             /* 写一条记录 */
//             for (k = 1; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//             {
//                 /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                     - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                     - 读/校验模式下 在ISP完成后从ISPFD读出数据
//                 */
//                 ISPFD = copyeep[k];
//                 Trigger_ISP();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 /* ISPAL ISP地址低字节 */
//                 ISPAL++;
//             }
//         }
//         /* 读地址小于等于备份页的首地址 */
//         else
//         {
//             /* 写一条记录 */
//             for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//             {
//                 /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//                     - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//                     - 读/校验模式下 在ISP完成后从ISPFD读出数据
//                 */
//                 ISPFD = copyeep[k];
//                 Trigger_ISP();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 /* ISPAL ISP地址低字节 */
//                 ISPAL++;
//             }
//         }
//         Disable_ISP_Mode();
//     }

//     /* 写其余记录至该类型的FLASH中 新记录号 = 旧记录号 + 1 */
//     writeaddres = RECORD_FIRST_ADDRESS[FirstAddr_index] + j * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//     Enable_ISP_Mode();

//     /* ISPCN = 0010 0001B ISP控制 选择操作 APROM或者数据FLASH 进行FLASH编程
//         - [7:6] A17:A16 = 00B 00B 选择操作APROM或者数据FLASH; 01B 选择LDROM; 11B CONFIG特殊功能;
//         - [5] FOEN      = 1B 选择 页擦除 或者 编程; 0B 选择 读数据;
//         - [4] FCEN      = 0B 无其他设置 只能为0;
//         - [3:0] FCTRL   = 0001B 编程; 0000B 读数据; 0010B 页擦除; 
//         */
//     ISPCN = BYTE_PROGRAM_DATA;
//     /* ISPAL ISP地址低字节 */
//     ISPAL = LOBYTE(writeaddres);
//     /* ISPAH ISP地址高字节 */
//     ISPAH = HIBYTE(writeaddres);

//     /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 
//         - 编程模式下    用户需要在触发ISP之前写数据到ISPFD里 
//         - 读/校验模式下 在ISP完成后从ISPFD读出数据
//     */
//     /* 更新的记录号 */
//     ISPFD = j + 1;
//     Trigger_ISP();
//     _nop_();
//     _nop_();
//     _nop_();
//     _nop_();
//     _nop_();
//     /* ISPAL ISP地址低字节 */
//     ISPAL++;

//     /* 表示写的是标定记录 比其它的记录多了个0点AD 和标定点AD */
//     if (FirstAddr_index == DEM_RECORD)
//     {
//         /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 */
//         ISPFD = ch4_0;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP地址低字节 */
//         ISPAL++;

//         /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 */
//         ISPFD = ch4_0 >> 8;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP地址低字节 */
//         ISPAL++;

//         /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 */
//         ISPFD = ch4_3500;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP地址低字节 */
//         ISPAL++;

//         /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 */
//         ISPFD = ch4_3500 >> 8;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP地址低字节 */
//         ISPAL++;
//     }

//     /* 记录日期: 年 月 日 时 分 */
//     for (i = 1; i < 6; i++)
//     {
//         /* ISPFD ISP内存数据 该字节包含将要读或写进内存空间的数据 */
//         ISPFD = timeCOPY[i];
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP地址低字节 */
//         ISPAL++;
//     }
//     Disable_ISP_Mode();
// }

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

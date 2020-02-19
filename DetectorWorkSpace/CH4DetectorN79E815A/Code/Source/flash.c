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
#include "uart.h"
#include "utlities.h"

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
/* 数据区域长度 */
code uint8_t length_of_data_zone[9] = {0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x06};

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

code uint16_t RECORD_LAST_ADDRESS[11] = 
{
    0xffff,                               /*!< 0.查询各类记录总数 */
    ALARM_RECORD_ADDR          + 200 * 4, /*!< 1.报警记录 200 */
    ALARM_RECOVERY_RECORD_ADDR + 200 * 4, /*!< 2.报警恢复记录 200 */
    FAULT_RECORD_ADDR          + 100 * 4, /*!< 3.故障记录 100 */
    FAULT_RECOVERY_RECORD_ADDR + 100 * 4, /*!< 4.故障恢复记录 100 */
    POWER_DOWN_RECORD_ADDR     + 50 * 4,  /*!< 5.掉电记录 50 */
    POWER_ON_RECORD_ADDR       + 50 * 4,  /*!< 6.上电记录 50 */
    SENSOR_EXPIRED_RECORD_ADDR,           /*!< 7.传感器失效记录 1 */
    0xffff,                               /*!< 8.内部定时器当前时间 */
    TEMP_PAGE_ADDR,                       /*!< 9.用于备份存储临时数据的FLASH地址 */
    DEVICE_INFO_ADDR                      /*!< 10.设备信息存储地址 */
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
    /* offset 为开始写的起始地址偏移 故注意此处应为 (offset + len - 1) */
    if ((offset + len - 1) >= 128)
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
    /* i2c_time_code[0]; */
    /* minute */
    temp_unzipped_time[4] = i2c_time_code[1];
    /* hour */
    temp_unzipped_time[3] = i2c_time_code[2];
    /* day */
    temp_unzipped_time[2] = i2c_time_code[3];
    /* week */
    /* i2c_time_code[4]; */
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

/* 解压缩时间 p 存储压缩过的时间 q 存储解压后的时间 */
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

void flash_write_record(uint8_t record_type)
{
    uint16_t page_addr;
    uint8_t page_offset;
    uint8_t record_first_byte;

    uint16_t record_total;

    uint16_t start_addr;
    uint16_t record_addr;
    uint16_t newest_addr;

    uint8_t temp_record_total[2];
    uint8_t temp_newest_addr[2];

    /* 找到该类型记录的首地址 */
    start_addr = RECORD_FIRST_ADDRESS[record_type];
#ifdef _DEBUG_
    uart_send((uint8_t)(start_addr >> 8));
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)start_addr);
    delay_1ms_without_BOD(10);
#endif

    /* 读取记录总数 */
    flash_read_data(temp_record_total, start_addr, 2);
    /* 记录总数的高位 */
    record_total = temp_record_total[0];
    /* 记录总数的低位 */
    record_total = (record_total << 8) + temp_record_total[1];
#ifdef _DEBUG_
    uart_send(temp_record_total[0]);
    delay_1ms_without_BOD(10);
    uart_send(temp_record_total[1]);
    delay_1ms_without_BOD(10);
#endif

    /* 读取最新记录的地址 */
    flash_read_data(temp_newest_addr, start_addr + 2, 2);
    /* 最新记录地址的高位 */
    newest_addr = temp_newest_addr[0];
    /* 最新记录地址的低位 */
    newest_addr = (newest_addr << 8) + temp_newest_addr[1];
#ifdef _DEBUG_
    uart_send(temp_newest_addr[0]);
    delay_1ms_without_BOD(10);
    uart_send(temp_newest_addr[1]);
    delay_1ms_without_BOD(10);
#endif

    /* 该记录类型未存储过记录 */
    if ((record_total == 0xFFFF) && (newest_addr == 0xFFFF))
    {
        /* 更新记录总数 */
        record_total = 0x0001;

        /* 计算写新纪录的地址 */
        record_addr = start_addr + 4;
    }
    /* 有已存储的记录 */
    else
    {
        /* 少于所规定的最大记录条数 */
        if (record_total < max_of_each_record[record_type])
        {
            record_total++;
        }

        /* 读取前最新的记录的首字节 */
        flash_read_data(&record_first_byte, newest_addr, 1);
        /* 将此前最新的记录标志改为旧的记录标志 0000 00 */
        record_first_byte &= 0x03;
        /* 将更新标志后的记录首字节写回 */
        page_addr = start_addr + ((newest_addr - start_addr) / 128) * 128;
        page_offset = (newest_addr - start_addr) % 128;
        flash_write_data(&record_first_byte, 1, page_addr, page_offset);

        /* 如果最新记录的地址位于记录区域的末尾 */
        if (newest_addr == RECORD_LAST_ADDRESS[record_type])
        {
            /* 计算写新纪录的地址 回至记录区域首部 +4 byte 即开始的第一条记录 */
            record_addr = start_addr + 4;
        }
        else
        {
            /* 计算写新纪录的地址 */
            record_addr = newest_addr + 4;
        }
    }

    /* 更新最新记录的地址 页地址和页内偏移 */
    newest_addr = record_addr;
    page_addr = start_addr + ((newest_addr - start_addr) / 128) * 128;
    page_offset = (newest_addr - start_addr) % 128;
#ifdef _DEBUG_
    uart_send((uint8_t)(newest_addr >> 8));
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)newest_addr);
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)(page_addr >> 8));
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)page_addr);
    delay_1ms_without_BOD(10);
    uart_send(page_offset);
    delay_1ms_without_BOD(10);
#endif

    /* 压缩当前时间至 zipped_time */
    zip_current_time(zipped_time);
    /* 设置最新数据标志 1000 00 */
    zipped_time[0] &= 0x03;
    zipped_time[0] |= 0x80;
#ifdef _DEBUG_
    uart_send(zipped_time[0]);
    delay_1ms_without_BOD(10);
    uart_send(zipped_time[1]);
    delay_1ms_without_BOD(10);
    uart_send(zipped_time[2]);
    delay_1ms_without_BOD(10);
    uart_send(zipped_time[3]);
    delay_1ms_without_BOD(10);
#endif

    /* 写记录 需要计算页首地址 */
    flash_write_data(zipped_time, 4, page_addr, page_offset);

    /* 写记录总数 */
    temp_record_total[0] = (uint8_t)((record_total >> 8) & 0xFF);
    temp_record_total[1] = (uint8_t)(record_total & 0xFF);
    flash_write_data(temp_record_total, 2, start_addr, 0);
    /* 写最新记录地址 */
    temp_newest_addr[0] = (uint8_t)((newest_addr >> 8) & 0xFF);
    temp_newest_addr[1] = (uint8_t)(newest_addr & 0xFF);
    flash_write_data(temp_newest_addr, 2, start_addr, 2);
}

void flash_read_record(uint8_t record_type, uint8_t record_number)
{
    uint8_t i;

    uint16_t record_total;
    uint16_t storage_length;

    uint16_t start_addr;
    uint16_t record_addr;
    uint16_t newest_addr;

    uint8_t temp_record_total[2];
    uint8_t temp_newest_addr[2];

    switch (record_type)
    {
        /* 查询设备各项记录总条数 */
        case 0x00:
        {
            /* SENSOR_EXPIRED_RECORD 单独处理 */
            for (i = ALARM_RECORD; i < SENSOR_EXPIRED_RECORD; i++)
            {
                /* 找到该类型记录的首地址 */
                start_addr = RECORD_FIRST_ADDRESS[i];
#ifdef _DEBUG_
                uart_send((uint8_t)(start_addr >> 8));
                delay_1ms(10);
                uart_send((uint8_t)start_addr);
                delay_1ms(10);
#endif
                /* 读取记录总数 此处仅用低8位 temp_record_total[1] 即可 */
                flash_read_data(temp_record_total, start_addr, 2);
#ifdef _DEBUG_
                uart_send(temp_record_total[0]);
                delay_1ms(10);
                uart_send(temp_record_total[1]);
                delay_1ms(10);
#endif                

                /* 该记录类型未存储过记录 此处仅用低8位 temp_record_total[1] 即可 */
                if (temp_record_total[1] == 0xFF)
                {
                    uart_buffer[i + 3] = 0x00;
                }
                else
                {
                    uart_buffer[i + 3] = temp_record_total[1];
                }
#ifdef _DEBUG_                
                uart_send(uart_buffer[i + 3]);
                delay_1ms(10);
#endif                
            }

            /* 读取气体传感器失效日期至 uart_buffer[10] */
            flash_read_data(&uart_buffer[10], SENSOR_EXPIRED_RECORD_ADDR, 1);

            /* 检查是否气体传感器已经到期 */
            if (uart_buffer[10] != 0x01)
            {
                /* 未到期的情况下发送 0x00 */
                uart_buffer[10] = 0x00;
            }

            /* 起始符 */
            uart_buffer[0] = 0xaa;
            /* 记录序号 */
            uart_buffer[1] = uart_buffer[1];
            /* 记录类型 */
            uart_buffer[2] = uart_buffer[2];
            /* 数据域长度 除最后一条设备时间其余均为 0x07 */
            uart_buffer[3] = 0x07;

            /* uart_buffer[4] 至 uart_buffer[11] 已处理 */

            /* 校验符 */
            uart_buffer[11] = get_crc(uart_buffer, (0x07 + 0x06));
            /* 结束符 */
            uart_buffer[12] = 0x55;

            for (i = 0; i < (0x07 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* 串口输出延时函数 关键参数 不可少 */
                delay_1ms(10);
            }

            break;
        }
        /* 查询气体传感器失效记录 */
        case 0x07:
        {
            /* 起始符 */
            uart_buffer[0] = 0xaa;
            /* 记录序号 */
            uart_buffer[1] = uart_buffer[1];
            /* 记录类型 */
            uart_buffer[2] = uart_buffer[2];
            /* 数据域长度 除最后一条设备时间其余均为 0x07 */
            uart_buffer[3] = 0x07;

            /* 读取气体传感器失效日期 标志[5] 年[6] 月[7] 日[8] 时[9] 分[10] */
            flash_read_data(&uart_buffer[5], SENSOR_EXPIRED_RECORD_ADDR, 6);
            /* 气体传感器已经失效 */
            if (uart_buffer[5] == 0x01)
            {
                /* 使用 uart_buffer[11] 暂时存储年 用于转化为年的完全格式 采用倒序对齐方式 */
                uart_buffer[11] = uart_buffer[6];

                /* n1 气体传感器失效标志 */
                uart_buffer[4] = uart_buffer[5];
                /* n2 年高字节 */
                uart_buffer[5] = (2000 + uart_buffer[11]) / 256;
                /* n3 年低字节 */
                uart_buffer[6] = (2000 + uart_buffer[11]) % 256;
                /* n4 月 */
                uart_buffer[7] = uart_buffer[7];
                /* n5 日 */
                uart_buffer[8] = uart_buffer[8];
                /* n6 时 */
                uart_buffer[9] = uart_buffer[9];
                /* n7 分 */
                uart_buffer[10] = uart_buffer[10];
            }
            /* 气体传感器未失效 */
            else
            {
                for (i = 4; i < 11; i++)
                {
                    uart_buffer[i] = 0x00;
                }
                
            }

            /* 校验符 */
            uart_buffer[11] = get_crc(uart_buffer, (0x07 + 0x06));
            /* 结束符 */
            uart_buffer[12] = 0x55;

            for (i = 0; i < (0x07 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* 串口输出延时函数 关键参数 不可少 */
                delay_1ms(10);
            }
            break;
        }
        /* 查询设备当前时间 */
        case 0x08:
        {
            /* 获取当前的设备时间 */
            i2c_get_time();

            /* 起始符 */
            uart_buffer[0] = 0xaa;
            /* 记录序号 */
            uart_buffer[1] = uart_buffer[1];
            /* 记录类型 */
            uart_buffer[2] = uart_buffer[2];
            /* 数据域长度 0x06 */
            uart_buffer[3] = 0x06;

            /* n1 年高字节 */
            uart_buffer[4] = (2000 + i2c_time_code[6]) / 256;
            /* n2 年低字节 */
            uart_buffer[5] = (2000 + i2c_time_code[6]) % 256;
            /* n3 月 */
            uart_buffer[6] = i2c_time_code[5];
            /* n4 日 */
            uart_buffer[7] = i2c_time_code[3];
            /* n5 时 */
            uart_buffer[8] = i2c_time_code[2];
            /* n6 分 */
            uart_buffer[9] = i2c_time_code[1];
            /* 校验符 */
            uart_buffer[10] = get_crc(uart_buffer, (0x06 + 0x06));
            /* 结束符 */
            uart_buffer[11] = 0x55;

            for (i = 0; i < (0x06 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* 串口输出延时函数 关键参数 不可少 */
                delay_1ms(10);
            }
            break;
        }
        /* 其他各类查询 */
        default:
        {
            /* 找到该类型记录的首地址 */
            start_addr = RECORD_FIRST_ADDRESS[record_type];

            /* 读取记录总数 */
            flash_read_data(temp_record_total, start_addr, 2);
            /* 记录总数的高位 */
            record_total = temp_record_total[0];
            /* 记录总数的低位 */
            record_total = (record_total << 8) + temp_record_total[1];
#ifdef _DEBUG_            
            uart_send(temp_record_total[0]);
            delay_1ms(10);
            uart_send(temp_record_total[1]);
            delay_1ms(10);
#endif

            /* 读取最新记录的地址 */
            flash_read_data(temp_newest_addr, start_addr + 2, 2);
            /* 最新记录地址的高位 */
            newest_addr = temp_newest_addr[0];
            /* 最新记录地址的低位 */
            newest_addr = (newest_addr << 8) + temp_newest_addr[1];
#ifdef _DEBUG_
            uart_send(temp_newest_addr[0]);
            delay_1ms(10);
            uart_send(temp_newest_addr[1]);
            delay_1ms(10);
#endif

            /* 该记录类型未存储过记录 或 如果查询的记录号大于所存的记录总数 */
            if (((record_total == 0xFFFF) && (newest_addr == 0xFFFF)) || (record_number > temp_record_total[1]))
            {
                /* Option 1: all data set to 0x00; Option 2: just return */
                for (i = 4; i < 11; i++)
                {
                    uart_buffer[i] = 0x00;
                }
            }
            else
            {
                /* 转换记录序号 序号越大记录越新 序号越小记录越旧 01号记录为最旧记录 */
                record_number = temp_record_total[1] + 1 - record_number;
                /* 计算要读取的记录地址 */
                storage_length = max_of_each_record[record_type] * 4;
                /* ZZZ 此计算可以有不同的计算形式 */
                record_addr = (start_addr + 4) + ((storage_length + newest_addr - (record_number - 1) * 4 - (start_addr + 4)) % storage_length);
#ifdef _DEBUG_
                uart_send((uint8_t)((start_addr + 4) >> 8));
                delay_1ms(10);
                uart_send((uint8_t)(start_addr + 4));
                delay_1ms(10);
                uart_send((uint8_t)(storage_length >> 8));
                delay_1ms(10);
                uart_send((uint8_t)storage_length);
                delay_1ms(10);
                uart_send((uint8_t)(newest_addr >> 8));
                delay_1ms(10);
                uart_send((uint8_t)newest_addr);
                delay_1ms(10);
                uart_send((uint8_t)((record_number - 1) >> 8));
                delay_1ms(10);
                uart_send((uint8_t)(record_number - 1));
                delay_1ms(10);
                uart_send((uint8_t)(record_addr >> 8));
                delay_1ms(10);
                uart_send((uint8_t)record_addr);
                delay_1ms(10);
#endif

                flash_read_data(zipped_time, record_addr, 4);
#ifdef _DEBUG_                
                uart_send(zipped_time[0]);
                delay_1ms(10);
                uart_send(zipped_time[1]);
                delay_1ms(10);
                uart_send(zipped_time[2]);
                delay_1ms(10);
                uart_send(zipped_time[3]);
                delay_1ms(10);
#endif

                unzip_time(zipped_time, unzipped_time);

                /* n1 记录序号 */
                uart_buffer[4] = record_number;
                /* n2 年高字节 */
                uart_buffer[5] = (2000 + unzipped_time[0]) / 256;
                /* n3 年低字节 */
                uart_buffer[6] = (2000 + unzipped_time[0]) % 256;
                /* n4 月 */
                uart_buffer[7] = unzipped_time[1];
                /* n5 日 */
                uart_buffer[8] = unzipped_time[2];
                /* n6 时 */
                uart_buffer[9] = unzipped_time[3];
                /* n7 分 */
                uart_buffer[10] = unzipped_time[4];
            }

            /* 起始符 */
            uart_buffer[0] = 0xaa;
            /* 记录序号 */
            uart_buffer[1] = uart_buffer[1];
            /* 记录类型 */
            uart_buffer[2] = uart_buffer[2];
            /* 数据域长度 除最后一条设备时间其余均为 0x07 */
            uart_buffer[3] = 0x07;

            /* uart_buffer[4] 至 uart_buffer[11] 已处理 */

            /* 校验符 */
            uart_buffer[11] = get_crc(uart_buffer, (0x07 + 0x06));
            /* 结束符 */
            uart_buffer[12] = 0x55;

            for (i = 0; i < (0x07 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* 串口输出延时函数 关键参数 不可少 */
                delay_1ms(10);
            }
            break;
        }
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

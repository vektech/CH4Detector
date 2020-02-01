/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      i2c.c
 * @version   V1.0.0
 * @brief     I2C Function(Master)
 *            SCL => P1.2
 *            SDA => P1.3
 *            Output => P1.4 toggle when I2C function pass
 *            I2CEN = 1 enable I2C function
 *            I2C Frequency == Fosc/(I2CLK+1)
 *            Use external OSC Fosc = 11.0592MHz
 *  
 *            <o0.0..7> I2CLK<2-255>
 *            <o1.0..7> I2C Slave Address<160-175>
 *            <h> UART pin Select
 *            <o2.6> Uart pin
 *            <0=> Select P1.0, P1.1 as UART pin(default)
 *            <1=> Select P2.6, P2.7 as UART pin(28 pin only)
 *            </h>
 * @warning   
 *******************************************************************************
 * @remarks
 * 1. Version                Date                 Author
 *    v1.0.0                 2020年1月1日          
 *    Modification: 创建文档
 *******************************************************************************
 */
/* NEED FIX */
/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "N79E81x.h"
#include "i2c.h"
#include "delay.h"
#include "utlities.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define I2C_CLOCK 255

/* 1:表示时钟芯片为PCF8563 0：表示时钟芯片为RX8010SJ */
#define PCF8563  0

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/
static void i2c_start(void);
static void i2c_8563_address(uint8_t sub);
static void i2c_reapeat_start(void);
static void i2c_read_data(uint8_t *temp, uint8_t u8_data);
static void i2c_deal_time_code(void);
static void i2c_stop(void);
// static void i2c_write_data(uint8_t *time, uint8_t length);


/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/
/* 秒--分钟--小时--日--星期--月--年 */
uint8_t i2c_time_code[7] = {0x00};

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void i2c_init(void)
{
    /* I2C 主机模式 时钟设置 f[I2C] = f[PHERI] / (1 + I2CLK) 从机模式该字节无效 可自动同步 400kps 以下的速率 */
    I2CLK = I2C_CLOCK;
    /* 使能 I2C总线 */
    I2CEN = 1;
}

/* 调用一次读函数 约耗时500uS */
void i2c_get_time(void)
{
    uint8_t i = 0;

    i2c_start();
#if PCF8563 == 1
    /* 8563 读、写地址 */
    i2c_8563_address(0xA2);
    i2c_8563_address(0x02);

    /* I2C 停止后重启 */
    i2c_reapeat_start();
    
    /* 8563 读、写地址 */
    i2c_8563_address(0xA3);
    i2c_read_data(i2c_time_code, 7);
    i2c_deal_time_code();
#else
    /* 8563 读、写地址 */
    i2c_8563_address(0x64);
    i2c_8563_address(0x10);
    
    /* I2C 停止后重启 */
    i2c_reapeat_start();

    /* 8563 读、写地址 */
    i2c_8563_address(0x65);
    i2c_read_data(i2c_time_code, 7);
    i2c_time_code[3] += i2c_time_code[4];
    i2c_time_code[4] = i2c_time_code[3] - i2c_time_code[4];
    i2c_time_code[3] = i2c_time_code[3] - i2c_time_code[4];
    i2c_deal_time_code();
#endif

    for (i = 0; i < 7; i++)
    {
        i2c_time_code[i] = bcd2hex(i2c_time_code[i]);
    }
    i2c_stop();
}

// void i2c_set_time(void)
// {
//     uint8_t i = 0;
    
//     i2c_start();
// #if PCF8563 == 1
//     /* 8563 读、写地址 */
//     i2c_8563_address(0xA2);
//     i2c_8563_address(0x02);

//     /* 8563 写入时间数据 */
//     i2c_write_data(i2c_time_code, 7);
// #else
//     /* 8563 读、写地址 */
//     i2c_8563_address(0x64);
//     i2c_8563_address(0x10);

//     i = i2c_time_code[3];
//     i2c_time_code[3] = i2c_time_code[4];
//     i2c_time_code[4] = i;
//     /* 8563 写入时间数据 */
//     i2c_write_data(i2c_time_code, 7);
//     i = i2c_time_code[3];
//     i2c_time_code[3] = i2c_time_code[4];
//     i2c_time_code[4] = i;
// #endif
//     i2c_stop();
// }

// void i2c_start_rtc(void)
// {
//     i2c_start();
// #if PCF8563 == 1
//     /* 8563 读、写地址 */
//     i2c_8563_address(0xA2);
//     i2c_8563_address(0x00);

//     i2c_time_code[0] = 0;
//     i2c_write_data(i2c_time_code, 1);
// #else
//     /* 8563 读、写地址 */
//     i2c_8563_address(0x64);
//     i2c_8563_address(0x1f);

//     i2c_time_code[0] = 0;
//     i2c_write_data(i2c_time_code, 1);
// #endif
//     i2c_stop();
// }

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/
static void i2c_start(void)
{
    uint8_t delay_count = 0;

    /* I2C Start标志 若总线空闲则产生I2C起始信号 */
    STA = 1;

    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }

    /* I2C Start标志 在检测到起始信号后应手动清零 */
    STA = 0;
}

static void i2c_reapeat_start(void)
{
    uint8_t delay_count = 0;

    /* I2C Start标志 若总线空闲则产生I2C起始信号 */
    STA = 1;
    /* STO置1 将在总线上输出停止信号 */
    STO = 1;
    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }

    /* I2C Start标志 在检测到起始信号后应手动清零 */
    STA = 0;
}

static void i2c_stop(void)
{
    uint8_t delay_count = 0;

    /* STO置1 将在总线上输出停止信号 */
    STO = 1;
    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    /* 检测STO停止标志 通过测试得知 等待时间为cnnt=0x0e */
    while ((I2CON & 0x10) == 0x10)
    {
        if (++delay_count > 250)
        {
            break;
        }
    }
}

/* 写寄存器地址 */
static void i2c_8563_address(uint8_t sub)
{
    uint8_t delay_count = 0;

    /* I2C 数据寄存器 只要SI为逻辑1 I2DAT中的数据保持不变 在I2C发送接收过程中 读或写I2DAT的结果都是不确定的 */
    /* XXX Address high for I2C EEPROM */
    I2DAT = sub;
    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }
}

// static void i2c_write_data(uint8_t *time, uint8_t length)
// {
//     uint8_t *ptemp = time;
//     uint8_t i = 0;
//     uint8_t delay_count;

//     for (i = 0; i < length; i++)
//     {
//         /* I2C 数据寄存器 只要SI为逻辑1 I2DAT中的数据保持不变 在I2C发送接收过程中 读或写I2DAT的结果都是不确定的 */
//         I2DAT = *ptemp;
//         /* 串行中断标志SI清零 串行传输暂停 */
//         SI = 0;

//         delay_count = 0;
//         /* 通过测试得知，等待时间为cnnt=0x69。反汇编得知：该行语句共7条指令。总指令数=735.=33uS@22M */
//         /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
//         while (!(I2CON & SET_BIT3))
//         {
//             if (++delay_count > 250)
//             {
//                 break;
//             }
//         }

//         ptemp++;
//     }
// }

/* 从I2C中读取固定长度数据放入缓冲数组 */
static void i2c_read_data(uint8_t *temp, uint8_t length)
{
    uint8_t i = 0;
    uint8_t delay_count;

    for (i = 0; i < length; i++)
    {
        /* I2CON[2] -> AA
            - 如果AA标志置位 当I2C设备为接收器时 在应答时钟脉冲期间将返回ACK。
            - 如果AA被清零 当I2C设备是接收器时 在应答时钟脉冲期间将返回NACK (SDA上为高电平)
            */
        AA = 1;

        /* 串行中断标志SI清零 串行传输暂停 */
        SI = 0;

        delay_count = 0;
        /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x69 */
        while (!(I2CON & SET_BIT3))
        {
            if (++delay_count > 250)
            {
                break;
            }
        }
        *temp = I2DAT;
        temp++;
    }

    /* I2CON[2] -> AA
        - 如果AA标志置位 当I2C设备为接收器时 在应答时钟脉冲期间将返回ACK。
        - 如果AA被清零 当I2C设备是接收器时 在应答时钟脉冲期间将返回NACK (SDA上为高电平)
        */
    AA = 0;

    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    delay_count = 0;
    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x69 */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }
}

/* 进行数据转换 */
static void i2c_deal_time_code(void)
{
    /* second */
    i2c_time_code[0] = i2c_time_code[0] & 0x7f;
    /* minuter */
    i2c_time_code[1] = i2c_time_code[1] & 0x7f;
    /* hour */
    i2c_time_code[2] = i2c_time_code[2] & 0x3f;
    /* day */
    i2c_time_code[3] = i2c_time_code[3] & 0x3f;
    /* week */
    i2c_time_code[4] = i2c_time_code[4] & 0x07;
    /* month */
    i2c_time_code[5] = i2c_time_code[5] & 0x1f;
    /* year */
    i2c_time_code[6] = i2c_time_code[6] & 0xff;
}

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

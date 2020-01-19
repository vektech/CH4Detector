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
#include "device.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define I2C_CLOCK 255

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/
static void I2C_Start(void);
static void I2C_Repeat_Start(void);
static void I2C_Stop(void);
static void I2C_8563_Address(uint8_t Sub);
static void I2C_8563_Sla_Address(uint8_t Sla);
static void Write_Data_To_Slave(uint8_t *Str, uint8_t u8Data);
static void Read_Data_from_Slave(uint8_t *Temp, uint8_t u8Data);
static void I2C_Deal_Read_Data(void);

/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/

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

void Master_Write_Data()
{
    UINT8 i = 0;
    I2C_Start();
#if PCF8563 == 1
    /* 8563 读、写地址 */
    I2C_8563_Address(0xA2);
    /* 8563 寄存器地址 */
    I2C_8563_Sla_Address(0x02);
    /* 8563 写入时间数据 */
    Write_Data_To_Slave(Time_Code, 7);
#else
    /* 8563 读、写地址 */
    I2C_8563_Address(0x64);
    /* 8563 寄存器地址 */
    I2C_8563_Sla_Address(0x10);

    i = Time_Code[3];
    Time_Code[3] = Time_Code[4];
    Time_Code[4] = i;
    /* 8563 写入时间数据 */
    Write_Data_To_Slave(Time_Code, 7);
    i = Time_Code[3];
    Time_Code[3] = Time_Code[4];
    Time_Code[4] = i;
#endif
    I2C_Stop();
}

/* 调用一次读函数 约耗时500uS */
void Master_Read_Data()
{
    UINT8 i = 0;

    I2C_Start();
#if PCF8563 == 1
    /* 8563 读、写地址 */
    I2C_8563_Address(0xA2);
    /* 8563 寄存器地址 */
    I2C_8563_Sla_Address(0x02);
    I2C_Repeat_Start();
    /* 8563 读、写地址 */
    I2C_8563_Address(0xA3);
    Read_Data_from_Slave(Time_Code, 7);
    I2C_Deal_Read_Data();
#else
    /* 8563 读、写地址 */
    I2C_8563_Address(0x64);
    /* 8563 寄存器地址 */
    I2C_8563_Sla_Address(0x10);
    I2C_Repeat_Start();
    /* 8563 读、写地址 */
    I2C_8563_Address(0x65);
    Read_Data_from_Slave(Time_Code, 7);
    Time_Code[3] += Time_Code[4];
    Time_Code[4] = Time_Code[3] - Time_Code[4];
    Time_Code[3] = Time_Code[3] - Time_Code[4];
    I2C_Deal_Read_Data();
#endif

    for (i = 0; i < 7; i++)
    {
        Time_Code[i] = bcd2hex(Time_Code[i]);
    }
    I2C_Stop();
}

void I2C_StartRTCClock(void)
{
    I2C_Start();
#if PCF8563 == 1
    /* 8563 读、写地址 */
    I2C_8563_Address(0xA2);
    /* 8563 寄存器地址 */
    I2C_8563_Sla_Address(0x00);
    Time_Code[0] = 0;
    Write_Data_To_Slave(Time_Code, 1);
#else
    /* 8563 读、写地址 */
    I2C_8563_Address(0x64);
    /* 8563 寄存器地址 */
    I2C_8563_Sla_Address(0x1f);
    Time_Code[0] = 0;
    Write_Data_To_Slave(Time_Code, 1);
#endif
    I2C_Stop();
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/
static void I2C_Start(void)
{
    UINT8 cntt = 0;

    /* I2C Start标志 若总线空闲则产生I2C起始信号 */
    STA = 1;

    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++cntt > 250)
        {
            break;
        }
    }

    /* I2C Start标志 在检测到起始信号后应手动清零 */
    STA = 0;
}

static void I2C_Repeat_Start(void)
{
    UINT8 cntt = 0;

    /* I2C Start标志 若总线空闲则产生I2C起始信号 */
    STA = 1;
    /* STO置1 将在总线上输出停止信号 */
    STO = 1;
    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++cntt > 250)
        {
            break;
        }
    }

    /* I2C Start标志 在检测到起始信号后应手动清零 */
    STA = 0;
}

static void I2C_Stop(void)
{
    UINT8 cntt = 0;

    /* STO置1 将在总线上输出停止信号 */
    STO = 1;
    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    /* 检测STO停止标志 通过测试得知 等待时间为cnnt=0x0e */
    while ((I2CON & 0x10) == 0x10)
    {
        if (++cntt > 250)
        {
            break;
        }
    }
}

static void I2C_8563_Address(uint8_t Sub)
{
    UINT8 cntt = 0;

    /* I2C 数据寄存器 只要SI为逻辑1 I2DAT中的数据保持不变 在I2C发送接收过程中 读或写I2DAT的结果都是不确定的 */
    /* XXX Address high for I2C EEPROM */
    I2DAT = Sub;
    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++cntt > 250)
        {
            break;
        }
    }
}

/* 寄存器地址 */
static void I2C_8563_Sla_Address(uint8_t Sla)
{
    UINT8 cntt = 0;

    /* I2C 数据寄存器 只要SI为逻辑1 I2DAT中的数据保持不变 在I2C发送接收过程中 读或写I2DAT的结果都是不确定的 */
    /* XXX Address high for I2C EEPROM */
    I2DAT = Sla;
    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++cntt > 250)
        {
            break;
        }
    }
}

static void Write_Data_To_Slave(uint8_t *Str, uint8_t u8Data)
{
    UINT8 *Ptemp = Str;
    UINT8 i = 0;
    UINT8 cntt;

    for (i = 0; i < u8Data; i++)
    {
        /* I2C 数据寄存器 只要SI为逻辑1 I2DAT中的数据保持不变 在I2C发送接收过程中 读或写I2DAT的结果都是不确定的 */
        I2DAT = *Ptemp;
        /* 串行中断标志SI清零 串行传输暂停 */
        SI = 0;

        cntt = 0;
        /* 通过测试得知，等待时间为cnnt=0x69。反汇编得知：该行语句共7条指令。总指令数=735.=33uS@22M */
        /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x0e */
        while (!(I2CON & SET_BIT3))
        {
            if (++cntt > 250)
            {
                break;
            }
        }

        Ptemp++;
    }
}

static void Read_Data_from_Slave(uint8_t *Temp, uint8_t u8Data)
{
    UINT8 i = 0;
    UINT8 cntt;

    for (i = 0; i < u8Data; i++)
    {
        /* I2CON[2] -> AA
            - 如果AA标志置位 当I2C设备为接收器时 在应答时钟脉冲期间将返回ACK。
            - 如果AA被清零 当I2C设备是接收器时 在应答时钟脉冲期间将返回NACK (SDA上为高电平)
            */
        AA = 1;

        /* 串行中断标志SI清零 串行传输暂停 */
        SI = 0;

        cntt = 0;
        /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x69 */
        while (!(I2CON & SET_BIT3))
        {
            if (++cntt > 250)
            {
                break;
            }
        }
        *Temp = I2DAT;
        Temp++;
    }

    /* I2CON[2] -> AA
        - 如果AA标志置位 当I2C设备为接收器时 在应答时钟脉冲期间将返回ACK。
        - 如果AA被清零 当I2C设备是接收器时 在应答时钟脉冲期间将返回NACK (SDA上为高电平)
        */
    AA = 0;

    /* 串行中断标志SI清零 串行传输暂停 */
    SI = 0;

    cntt = 0;
    /* 检测串行中断标志SI是否置位 置位后跳出循环 通过测试得知 等待时间为cnnt=0x69 */
    while (!(I2CON & SET_BIT3))
    {
        if (++cntt > 250)
        {
            break;
        }
    }
}

/* 进行数据转换 */
static void I2C_Deal_Read_Data(void)
{
    /* second */
    Time_Code[0] = Time_Code[0] & 0x7f;
    /* minuter */
    Time_Code[1] = Time_Code[1] & 0x7f;
    /* hour */
    Time_Code[2] = Time_Code[2] & 0x3f;
    /* day */
    Time_Code[3] = Time_Code[3] & 0x3f;
    /* week */
    Time_Code[4] = Time_Code[4] & 0x07;
    /* month */
    Time_Code[5] = Time_Code[5] & 0x1f;
    /* year */
    Time_Code[6] = Time_Code[6] & 0xff;
}

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

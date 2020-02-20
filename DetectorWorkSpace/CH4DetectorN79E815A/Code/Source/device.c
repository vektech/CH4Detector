/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      device.c
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
#include "GlobalAppDefine.h"
#include "device.h"

#include "delay.h"
#include "uart.h"
#include "sensor.h"
#include "timer.h"
#include "adc.h"
#include "i2c.h"
#include "flash.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define set_BOF   \
    EA_save_bit = EA;      \
    EA = 0;       \
    TA = 0xAA;    \
    TA = 0x55;    \
    PMCR |= SET_BIT3;\
    EA = EA_save_bit;

#define clr_BOF   \
    EA_save_bit = EA;      \
    EA = 0;       \
    TA = 0xAA;    \
    TA = 0x55;    \
    PMCR &= CLR_BIT3;\
    EA = EA_save_bit;

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/
/* 设备第一次上电标志 */
bit device_first_power_on = false;

/* 设备掉电计数 */
uint8_t device_power_down_count = 0;

/* 全局中断存储位 */
bit EA_save_bit;

/* 设备的阀状态 */
bit device_valve_state = false;
/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void device_init(void)
{
    /* 全局中断使能位 0 = 关闭所有中断 */
    EA = 0;

    /* P0.0配置为推挽输出模式 其他P0端口配置为准双向模式 
        - 设置成推挽输出，有大电流驱动能力
        - P0.0 电磁阀控制IO
        */
    /* 端口0输出模式配置1 */
    P0M1 = 0x00;
    /* 端口0输出模式配置2 */
    P0M2 = 0x01;

    /* P1.6 P1.7配置为推挽输出模式 其他P1端口配置为准双向模式 
        - I2C的PxM1,PxM2相应位需要设置成1
        - P1.6 ICPDAT
        - P1.7 ICPCLK
        */
    /* 端口1输出模式配置1 */
    P1M1 |= 0xC0;
    /* 端口1输出模式配置2 */
    P1M2 |= 0xC0;

    /* P2.2 P2.4配置为推挽输出模式 其他P2端口配置为准双向模式 
        - 设置成推挽输出，有大电流驱动能力
        - P2.2 继电器控制IO
        - P2.4 电源灯控制IO
        - P2.7 传感器控制IO
        */
    /* 端口2输出模式配置1 */
    P2M1 = 0x00;
    /* 端口2输出模式配置2 */
    P2M2 = 0x94;

    /* P3M1->P1S位 使能端口P1的史密特触发输入缓冲 增强手刺抑制能力 主要针对I2C */
    P3M1 |= 0x20;

    /* 打开传感器开关 */
    SENSER_ON;

    /* P2.2 = 0 继电器关 */
    DELAY_OFF;
    /* P0.0 = 0 电磁阀关 */
    VALVE_OFF;

    delay_1ms_without_BOD(3000);

    /* P2.2 = 0 继电器关 */
    DELAY_OFF;
    /* P0.0 = 0 电磁阀关 */
    VALVE_OFF;

    /* 附加功能选择寄存器 选择串口管脚
        - DPS      = 0 选择标准8051单DPTR 
        - DisP26   = 0 打开P2.6数字输入输出功能
        - UART_Sel = 0 选择P1.0 P1.1 作为串口管脚
        - SPI_Sel  = 0 选择P1.7 P1.6 P1.4 P0.0 作为SPI管脚
        */
    AUXR1 = UAER_PORT_SELECT;

    /* 关键SFR读写保护 */
    TA = 0xAA;
    TA = 0x55;
    /* PMCR电源检测控制寄存器设置 PMCR = 1100 0000B 
        - BODEN[7]       = 1B 使能欠压检测
        - BOV[6]         = 1B 结合CBOV 设置欠压检测为3.8V
        - Reserved3[5]   = 0B
        - BORST[4]       = 0B 当VDD下降或上升至VBOD 禁止欠压检测复位 当VDD下降至VBOD以下 芯片将置位BOF 
        - BOF[3]         = 0B 欠压检测标志位 当VDD下降或上升至VBOD置位
        - Reserved2[2]   = 0B 必须为0
        - Reserved1[1:0] = 00B 
        */
    PMCR = 0xc0;

    /* 全局中断使能位 1 = 打开所有中断 */
    EA = 1;

    /* EBO 为BOD电源电压检测的中断使能位 开中断 sbit EBO = IE ^ 5 */
    EBO = 1;
    /* 使能串口中断 1 = 设定TI(SCON.1)或RI(SCON.0)*/
    ES = 1;
    /* 定时器2运行控制 1 = 打开定时器2 */
    TR2 = 1;
    /* 拓展中断使能寄存器<EIE> 1 = 打开定时器2中断 */
    ET2 = 1;
    /* 中断优先级寄存器高字节 PBODH = 1 设置BOD检测中断高优先级为最高优先级 */
    IPH |= 0X20;

    delay_1ms_without_BOD(1000);

    // /* 9600 Baud Rate @ 22.1184MHz */
    // uart_init(9600);

    /* 4800 Baud Rate @ 22.1184MHz */
    uart_init(4800);

    /* 定时器2初始化 */
    timer2_init();

    /* ADC初始化 */
    adc_init();
    
    /* I2C初始化 */
    i2c_init();
}

/* Brown-Out Detector 电源电压检测  Power-On Reset 上电复位 */
void check_BOD(void)
{
    /* 如果设备掉电计数超限 进行掉电处理函数 */
    if (device_power_down_count >= POWER_DOWN_LIMIT)
    {
        /* 继电器关 */
        DELAY_OFF;
        /* 电磁阀关 */
        VALVE_OFF;
        /* 传感器开关关闭 */
        SENSER_OFF;
        /* 故障灯关 */
        LED_FAULT_OFF;
        /* 报警灯关 */
        LED_ALARM_OFF;
        /* 传感器寿命灯关 */
        LED_LIFE_OFF;
        /* 蜂鸣器1关 */
        SOUND1_OFF;
        /* 蜂鸣器2关 */
        SOUND2_OFF;
        /* 电源灯关 */
        LED_POWER_OFF;

        /* 串口电平拉低 */
        P10 = 0;
        P11 = 0;
        
        /* 向FLASH中写掉电记录 */
        flash_write_record(POWER_DOWN_RECORD);

        /* 掉电计数复位 */
        device_power_down_count = 0;
        /* 预热标记复位 */
        sensor_preheat_flag = false;
        /* 预热时间清零 */
        sensor_preheat_time_count = 0;

        /* 软件复位 */
        while (1)
        {
            /* 关闭全局中断 */
            EA = 0;

            /* ISP 功能的关键SFR读写保护 */
            TA = 0xAA;
            TA = 0x55;
            /* 将CHPCON中的 BS = 0 
                - 写：定义了复位后MCU启动的区块 0 = 下一次从APROM启动 1 = 下一次从LDROM启动 
                - 读：定义了前次复位后MCU启动的区块 0 = 前次从APROM启动 1 = 前次从LDROM启动 */
            CHPCON &= 0xfd;

            /* ISP 功能的关键SFR读写保护 */
            TA = 0xAA;
            TA = 0x55;
            /* 将CHPCON中的 SWRST = 1 设置该位为逻辑1将引起软件复位 复位完成后由硬件自动清零 */
            CHPCON |= 0x80;
        }
    }
}

/* Brown-Out Detector 电源电压检测中断服务程序 Vecotr @ 0x43 */
void BOD_ISR(void) interrupt 8
{
    /* 清楚欠压检测标志 */
    clr_BOF;
    /* 设备检测到掉电计数 */
    device_power_down_count++;
    if (device_power_down_count >= POWER_DOWN_LIMIT)
    {
        device_power_down_count = POWER_DOWN_LIMIT;
        /* EBO 为BOD电源电压检测的中断使能位 关中断 sbit EBO = IE ^ 5 */
        EBO = 0;
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

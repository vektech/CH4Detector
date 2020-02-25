/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      timer.c
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
#include "GlobalAppDefine.h"

#include "timer.h"
#include "uart.h"
#include "sensor.h"

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
bit timer2_second_flag;
bit timer2_second_expired_flag;
bit timer2_life_hour_flag;
bit key_long_press_flag;

uint8_t timer2_key_long_press_count;

uint16_t timer2_count;
uint8_t timer2_second_count;


uint16_t timer2_life_second_count;



/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
// void timer0_init(void)
// {
//     /* 定时器0和1模式配置 
//         - [7]   = 0B  定时器1 门控制 当TR1=1时 定时器1 时钟运行不管~INT1的逻辑电平
//         - [6]   = 0B  定时器1 计数器/定时器选择 定时器1随内部时钟而递增
//         - [5:4] = 00B 模式0: 8位定时器/计数器带5位预分频 TL1[4:0]
//         - [3]   = 0B  定时器0 门控制 当TR0=1时 定时器0 时钟运行不管~INT0的逻辑电平
//         - [2]   = 0B  定时器0 计数器/定时器选择 定时器0随内部时钟而递增
//         - [1:0] = 00B 模式0: 8位定时器/计数器带5位预分频 TL1[4:0]
//     */
//     /* ZZZ 定时器0工作在16位定时模式 */
//     TMOD = 0x00;
//     /* 时钟控制
//         - [4] = 0B 定时器1 的时钟选择为 1/12 系统时钟
//         - [3] = 0B 定时器0 的时钟选择为 1/12 系统时钟
//         */
//     CKCON = 0x00;
//     /* 定时器计数值 0x9C40 = 40000 */
//     /* 定时器0 高字节 */
//     TH0 = 0x9C;
//     /* 定时器0 低字节 */
//     TL0 = 0x40;

//     /* 打开定时器0 */
//     TR0 = 1;
// }

void timer2_init(void)
{
    /* 定时器2模式 
        - [7]   = 1B   在定时器2溢出或发生输入捕获事件时, 使能将 RCOMP2H 和 RCOMP2L 加载到 TH2 和 TL2, 比较模式下该位无效
        - [6:4] = 000B timer2 clock divider is 1/4
        - [3]   = 0B   发生捕获事件后, 定时器2继续计数 
        - [2]   = 0B   发生比较匹配时, 定时器2继续计数
        - [1:0] = 00B  定时器2溢出重加载
        */
    T2MOD = 0x80;
    /* 定时器计数值 0x2806 = 10246 */
    /* 10ms */
    /* 定时器2 高字节 */
    TH2 = 0x28;
    /* 定时器2 低字节 */
    TL2 = 0x06;

    /* 定时器2 设置为比较模式 当定时器2向上计数, TH2和TL2与RCOMP2H和RCOMP2L值匹配, TF3(T2CON.7) 将由硬件置位以示有比较匹配时间发生 */
    CP_RL2 = 1;
    /* 定时器2 重加载/比较低字节 */
    RCOMP2L = TL2;
    /* 定时器2 重加载/比较高字节 */
    RCOMP2H = TH2;

    /* 打开定时器2 */
    TR2 = 1;
}

/* 定时器T2 做为预热灯闪烁控制定时器和间隔5秒发一次数据定时器 定时器T1 做为串口波特率加载用定时器 */
/* TIMER2 中断服务函数 Vecotr @ 0x2B 中断一次12ms */
void Timer2_ISR(void) interrupt 5
{
    /* 定时器2溢出标志 必须软件清零 */
    TF2 = 0;

    /* 接收超时 */
    if (++rx_count > RXTIMEOUT)
    {
        rx_count = 0;
        /* 获取了4个以上字节的数据 */
        if ((rx_index >= RX_MIN_LENGTH))
        {
            if (rx_finished == false)
            {
                /* 关闭串口0的接收功能 */
                UART0_RX_DISABLE;
                /* 接收标志置为1 */
                rx_finished = true;
            }
        }
        else
        {
            rx_finished = false;
            rx_index = 0;
        }
    }

    /* 时间计数加1 */
    timer2_count++;
    /* 秒计数加1 */
    timer2_second_count++;

    /* 预热状态灯闪烁频率 */
    if (timer2_count == 20)
    {   
        /* 预热状态下 */
        if (sensor_preheat_flag == false)
        {
            /* 电源灯翻转 */
            LED_POWER_TOGGLE;
        }
        timer2_count = 0;
        sensor_preheat_time_count++;
    }

    /* 预热完成 标志位置 PREHEAT_TIME_COUNT = 900 为3分钟*/
    if (sensor_preheat_time_count == PREHEAT_TIME_COUNT)
    {
        sensor_preheat_flag = true;
    }

    /* 计时100次完成 */
    if (timer2_second_count == 100)
    {
        timer2_second_count = 0;

        /* 秒到期标志 */
        timer2_second_flag = true;
        /* 检查是否已写入RTC和出厂日期 传感器寿命过期检查标志 */
        timer2_second_expired_flag = true;

        /* 寿命到期计时数 */
        timer2_life_second_count++;

        if (key_long_press_flag)
        {
            timer2_key_long_press_count++;
        }
    }

    /* 1小时标志位 ZZZ 3600 150 1 */
    if (timer2_life_second_count == 1)
    {
        /* 小时到期标志 */
        timer2_life_hour_flag = true;
        timer2_life_second_count = 0;
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      delay.c
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

#include "delay.h"
#include "device.h"
#include "uart.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#ifdef FOSC_110592
/* 27*4/11.0592 = 10 uS */
#define VALUE_10us 65536 - 27
/* 2769*4/11.0592 = 1 mS */
#define VALUE_1ms 65536 - 2769
#endif

#ifdef FOSC_184320
/* 45*4/18.432 = 10 uS */
#define VALUE_10us 65536 - 45
/* 4608*4/18.432 = 1 mS */
#define VALUE_1ms 65536 - 4608
#endif

#ifdef FOSC_221184
/* 54*4/22.1184 = 10 uS */
#define VALUE_10us 65536 - 54
/* 5529*4/22.1184 = 1 mS */
#define VALUE_1ms 65536 - 5529
#endif

#ifdef FOSC_368640
/* 93*4/36.864 = 10 uS */
#define VALUE_10us 65536 - 93
/* 9216*4/36.864 = 1 mS */
#define VALUE_1ms 65536 - 9216
#endif

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

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void delay_10us(uint16_t count)
{
    /* 打开定时器0 */
    TR0 = 1;
    while (count != 0)
    {
        /* 定时器0 低字节 */
        TL0 = LOBYTE(VALUE_10us);
        /* 定时器0 高字节 */
        TH0 = HIBYTE(VALUE_10us);
        /* TF0->TCON[5] 定时器0溢出标志
            - 在定时器0溢出时该位置1
            - 当程序响应定时器0中断执行相应的中断服务程序时 该位自动清0
            - 软件也可对其写1或写0
            */
        /* 等待定时器溢出 阻塞式延时*/
        while (TF0 != 1)
        {   
            ;
        }
        /* 溢出标志清零 */
        TF0 = 0;
        count--;
    }
    /* 关闭定时器0 */
    TR0 = 0;
}

/* Delay with BOD */
void delay_1ms(uint16_t count)
{
    /* Brown-Out Detector 电源电压检测 */
    check_BOD();
    /* 打开定时器0 */
    TR0 = 1;
    while (count != 0)
    {
        /* 定时器0 低字节 */
        TL0 = LOBYTE(VALUE_1ms);
        /* 定时器0 高字节 */
        TH0 = HIBYTE(VALUE_1ms);
        /* TF0->TCON[5] 定时器0溢出标志
            - 在定时器0溢出时该位置1
            - 当程序响应定时器0中断执行相应的中断服务程序时 该位自动清0
            - 软件也可对其写1或写0
            */
        /* 等待定时器溢出 阻塞式延时*/
        while (TF0 != 1)
        {
            ;
        }
        /* 溢出标志清零 */
        TF0 = 0;
        count--;
    }
    /* 关闭定时器0 */
    TR0 = 0;
}

// #ifdef _DEBUG_
/*  Delay without BOD for short timer use. 
    Because in BOD check, the write record function will be called. 
    And therefore it result in a recursive call. */
void delay_1ms_without_BOD(uint16_t count)
{
    /* 打开定时器0 */
    TR0 = 1;
    while (count != 0)
    {
        /* 定时器0 低字节 */
        TL0 = LOBYTE(VALUE_1ms);
        /* 定时器0 高字节 */
        TH0 = HIBYTE(VALUE_1ms);
        /* TF0->TCON[5] 定时器0溢出标志
            - 在定时器0溢出时该位置1
            - 当程序响应定时器0中断执行相应的中断服务程序时 该位自动清0
            - 软件也可对其写1或写0
            */
        /* 等待定时器溢出 阻塞式延时*/
        while (TF0 != 1)
        {
            ;
        }
        /* 溢出标志清零 */
        TF0 = 0;
        count--;
    }
    /* 关闭定时器0 */
    TR0 = 0;
}
// #endif

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/


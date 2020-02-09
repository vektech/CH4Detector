/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      uart.c
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
#include "uart.h"
#include "delay.h"

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
uint8_t uart_buffer[20] = {0x00};
uint8_t rx_index;
uint8_t rx_finished;

/* 用于在 Timer2 中检查串口接受超时 每次接收清零 两次 Timer2 中断后检查接收的结果 */
uint8_t rx_count;

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
/* ---- Use timer1 as baudrate generator ---- */
void uart_init(uint32_t bandrate)
{
    /* SCON = 11010010B 串口控制
        - SM0/SM1[7:6] = 11B: Mode3 异步11位帧长度 定时器1溢出时间 除以32或除以16 
        - SM2[5]       = 0B:  接收有效, 无论第9位是否有逻辑电平
        - REN[4]       = 1B:  打开串口的接收功能,接收完成后需软件清除该位
        - TB8[3]       = 0B:  Mode2和Mode3中要被发送的第九位数据
        - RB8[2]       = 0B:  Mode2和Mode3中要接收到的第九位数据
        - TI[1]        = 1B:  发送中断标志位, 除Mode0外, 在串口发送到停止位时置位, 该位必须由软件来清除
        - RI[0]        = 0B:  接收中断标志, Mode2和Mode3下, 接收到第九位时置位, 该位必须由软件来清除
        */
    SCON = 0xD2;

    /* PCON = 0000 0000B 电源控制
        - SMOD[7]  = 0 关闭波特率加倍 在模式1,2或3中串口波特率加倍使能位 仅适用于定时器1溢出作为为波特率时钟源时
        - SMOD0[6] = 0 关闭帧错误检测功能
        - POF[4]   = 0 上电复位后该位置1 表示一次冷复位 上电复位完成 在其他复位后该位保持不变 建议软件清该标志
        - GF1[3]   = 0 通用标志1 通用标志可由用户置位或清零
        - GF0[2]   = 0 通用标志0 通用标志可由用户置位或清零
        - PD[1]    = 0 设置该位使MCU进入掉电模式 在此模式下 CPU和外设时钟停止 程序计数器PC挂起 此时为最小功耗
        - IDL[0]   = 0 设置该位使MCU进入空闲模式 在此模式下 CPU时钟停止 且程序计数器PC挂起
        */
    PCON = 0x00;

    /* TMOD = 0010 0000B 定时器0和1模式配置 
        - [7]   = 0B  定时器1 门控制 当TR1=1时 定时器1 时钟运行不管~INT1的逻辑电平
        - [6]   = 0B  定时器1 计数器/定时器选择 定时器1随内部时钟而递增
        - [5:4] = 10B 模式2: 8位定时器/计数器带自动从TH1重载模式
        - [3]   = 0B  定时器0 门控制 当TR0=1时 定时器0 时钟运行不管~INT0的逻辑电平
        - [2]   = 0B  定时器0 计数器/定时器选择 定时器0随内部时钟而递增
        - [1:0] = 00B 模式0: 8位定时器/计数器带5位预分频 TL1[4:0]
        */
    TMOD = 0x20;

#ifdef FOSC_110592
    TH1 = 256 - (28800 / bandrate); /* 11.059M/384=28800 */
#endif
#ifdef FOSC_184320
    TH1 = 256 - (48000 / bandrate); /* 18.4320M/384=48000 */
#endif
    /* 使用该配置 在可IDE中进行定义　FOSC_221184 */
#ifdef FOSC_221184
    TH1 = 256 - (57600 / bandrate); /* 22.1184M/384=57600 */
#endif
#ifdef FOSC_331776
    TH1 = 256 - (86400 / bandrate); /* 33.1776M/384=86400 */
#endif
#ifdef FOSC_368640
    TH1 = 256 - (96000 / bandrate); /* 36.8640M/384=96000 */
#endif
    /* 开启定时器1 */
    TR1 = 1;
}

void uart_send(uint8_t byte)
{
    /* 奇偶校验 0 = 偶校验, 1 = 奇校验 */
#if (UART_PARITY_SET == EVEN_PARITY)
    /* 将发送的数据放至ACC中 */
    /* PSW 中的 P位 为奇偶标志 累加结果为奇数时为1 偶数时为0 */
    ACC = byte;
    /* 偶校验 TB8 存储要发送的校验位 */
    TB8 = P;
#else
    /* 将发送的数据放至ACC中 */
    /* PSW 中的 P位 为奇偶标志 累加结果为奇数时为1 偶数时为0 */
    ACC = byte;
    /* 奇校验 TB8 存储要发送的校验位 */
    TB8 = ~P;
#endif
    /* SBUF 串行口接收或者发送的数据都放在这个寄存器中
        - 实际上改地址上有2个独立的8位寄存器 一个用于接收数据 一个用于发送数据
        - 对它进行读操作将会接收串行数据 对它进行写操作则发送串行数据 
        - 每次向SBUF写入一字节数据 启动一次发送 
        */
    SBUF = byte;
}

/* UART 中断服务函数 */
void UART_ISR(void) interrupt 4
{
    uint8_t i = 0;
    bit parity_bit = 0;

    /* 读过程 RI 即 Reception Interrupt Flag */
    if (RI == 1)
    {
        /* Clear reception flag for next reception */
        RI = 0;

        /* SBUF 串行口接收或者发送的数据都放在这个寄存器中
            - 实际上改地址上有2个独立的8位寄存器 一个用于接收数据 一个用于发送数据
            - 对它进行读操作将会接收串行数据 对它进行写操作则发送串行数据 
            - 每次向SBUF写入一字节数据 启动一次发送 
            */
        ACC = SBUF;

        /* PSW 中的 P位 为奇偶标志 累加结果为奇数时为1 偶数时为0 */
        if (P)
        {
            /* 为奇数时置1 */
            parity_bit = 1;
        }

    /* 奇偶校验 0=偶校验 1=奇校验 */
#if (UART_PARITY_SET == EVEN_PARITY)
        /* 偶校验 */
        if (RB8 == (parity_bit))
        {
            /* 读SBUF 存入uart_buffer中 */
            uart_buffer[rx_index++] = SBUF;
            /* Rx 存储空间将满 */
            if (rx_index >= (sizeof(uart_buffer) - 1))
            {
                /* UART0 关闭接收功能 */
                UART0_RX_DISABLE;
                /* 接收完成 */
                rx_finished = true;
            }
        }
#else
        /* 奇校验 */
        if (RB8 == (~parity_bit))
        {
            /* 读SBUF 存入uart_buffer中 */
            uart_buffer[rx_index++] = SBUF;
            /* Rx 存储空间满 XXX 为何和前项不同 */
            if (rx_index >= sizeof(uart_buffer))
            {
                /* UART0 关闭接收功能 */
                UART0_RX_DISABLE;
                /* 接收完成 */
                rx_finished = true;
            }
        }
#endif
        /* 读串口数据直到存储空间将满为止 */
        rx_count = 0;
    }
    /* 写中断 此处清除写中断标志 */
    else
    {
        /* Clear transmit flag for next transmition */
        TI = 0;
    }
    /* XXX 使能TIME2定时器 因为在蜂鸣器响之前 把TR2关掉了 蜂鸣器独占CPU时间 
       所以一旦接收到数据 则把TIME2打开以增加串口接收完成判断的实时性 */
    TR2 = 1;
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

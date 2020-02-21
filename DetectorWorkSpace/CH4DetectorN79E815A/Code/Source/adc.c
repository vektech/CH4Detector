/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      adc.c
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
#include "adc.h"
#include "device.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/

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
void adc_init(void)
{
    /* AAA Brown-Out Detector 电源电压检测 */
    check_BOD();

    /* 关闭P0.1 的数字逻辑输入输出功能 设置P0.1 为输入(高阻)模式 */
    adc_set_input_mode(E_CHANNEL0);
    /* AADR2 AADR1 AADR2 = 000B 选择ADC0 即P0.1作为ADC采样端口 */
    adc_channel_sel(E_CHANNEL0);

    /* 打开ADC电路 */
    set_ADCEN;
}

void adc_set_input_mode(enum E_ADCCNL_SEL channel)
{
    switch (channel)
    {
    case E_CHANNEL0:
        /* 关闭P0.1 的数字逻辑输入输出功能 */
        P0DIDS |= SET_BIT1;
        /* 设置P0.1 输入(高阻)模式 */
        P0M1 = SET_BIT1;
        P0M2 = 0x00;
        break;
    case E_CHANNEL1:        // ADC1(P0.2) is input-only mode
        P0DIDS |= SET_BIT2; // Disable digital function for P0.2
        P0M1 = SET_BIT2;
        P0M2 = 0x00;
        break;
    case E_CHANNEL2:        // ADC2(P0.3) is input-only mode
        P0DIDS |= SET_BIT3; // Disable digital function for P0.3
        P0M1 = SET_BIT3;
        P0M2 = 0x00;
        break;
    case E_CHANNEL3:        // ADC3(P0.4) is input-only mode
        P0DIDS |= SET_BIT4; // Disable digital function for P0.4
        P0M1 = SET_BIT4;
        P0M2 = 0x00;
        break;
    case E_CHANNEL4:        // ADC4(P0.5) is input-only mode
        P0DIDS |= SET_BIT5; // Disable digital function for P0.5
        P0M1 = SET_BIT5;
        P0M2 = 0x00;
        break;
    case E_CHANNEL5:        // ADC5(P0.6) is input-only mode
        P0DIDS |= SET_BIT6; // Disable digital function for P0.6
        P0M1 = SET_BIT6;
        P0M2 = 0x00;
        break;
    case E_CHANNEL6:        // ADC6(P0.7) is input-only mode
        P0DIDS |= SET_BIT7; // Disable digital function for P0.7
        P0M1 = SET_BIT7;
        P0M2 = 0x00;
        break;
    case E_CHANNEL7:       // ADC7(P2.6) is input-only mode(28 pin only)
        AUXR1 |= SET_BIT3; // Disable digital function for P2.6
        P2M1 = SET_BIT6;
        P2M2 = 0x00;
        break;
    }
}

void adc_channel_sel(enum E_ADCCNL_SEL channel)
{
    switch (channel)
    {
    /* P0.1 (default) */
    case E_CHANNEL0:
        /* AADR2 AADR1 AADR2 = 000B 选择ADC0 即P0.1作为ADC采样端口 */
        clr_AADR2;
        clr_AADR1;
        clr_AADR0;
        break;
    case E_CHANNEL1: // P0.2
        clr_AADR2;
        clr_AADR1;
        set_AADR0;
        break;
    case E_CHANNEL2: // P0.3
        clr_AADR2;
        set_AADR1;
        clr_AADR0;
        break;
    case E_CHANNEL3: // P0.4
        clr_AADR2;
        set_AADR1;
        set_AADR0;
        break;
    case E_CHANNEL4: // P0.5
        set_AADR2;
        clr_AADR1;
        clr_AADR0;
        break;
    case E_CHANNEL5: // P0.6
        set_AADR2;
        clr_AADR1;
        set_AADR0;
        break;
    case E_CHANNEL6: // P0.7
        set_AADR2;
        set_AADR1;
        clr_AADR0;
        break;
    case E_CHANNEL7: // P2.6
        set_AADR2;
        set_AADR1;
        set_AADR0;
        break;
    }
}

/* 选择ADC0 即P0.1作为ADC采样端口 打开ADC中断使能 */
void adc_init_interrupt(void)
{
    /* 关闭P0.1 的数字逻辑输入输出功能 设置P0.1 为输入(高阻)模式 */
    adc_set_input_mode(E_CHANNEL0);
    /* AADR2 AADR1 AADR2 = 000B 选择ADC0 即P0.1作为ADC采样端口 */
    adc_channel_sel(E_CHANNEL0);

    /* 打开ADC中断和全局中断 */
    adc_enable_interrupt();
    /* 打开ADC电路 */
    set_ADCEN;
}

void adc_enable_interrupt(void)
{
    /* 打开ADC中断 */
    EADC = 1;
    /* 打开全局中断 */
    EA = 1;
}

uint16_t adc_sensor(void)
{
    uint16_t temp[10];
    uint8_t i;

    /* 采集10次用来滤波 */
    for (i = 0; i < 10; i++)
    {
        /* AAA Brown-Out Detector 电源电压检测 */
        check_BOD();        
        /* 获取单次的ADC采样结果 */
        temp[i] = adc_single_sample();
    }

    /* 返回滤波后的值 */
    return adc_sensor_filter(temp);
}

uint16_t adc_single_sample(void)
{
    uint16_t u16_ADCL;
    uint16_t u16_ADC;
    adc_trigger_convertion();

    /* 从ADCCON0获取 ADC 10bit 采集值的最后2个bit ADC.1 ADC.0 ADC[1:0]*/
    u16_ADCL = ADCCON0;
    u16_ADCL = u16_ADCL >> 6;
    /* 从ADCH中获取 ADC 转换结果位 ADC[9:2] */
    u16_ADC = ADCH;
    /* 将ADC的采样结果合成 10bit */
    u16_ADC = (u16_ADC << 2) + u16_ADCL;

    return u16_ADC;
}

void adc_trigger_convertion(void)
{
    /* Clear ADC flag 
        - ADCI = 0 ADC空闲
        - ADCI = 1 ADC转换结果已经可以读取 如果中断使能 会产生一个中断 不能由软件置位
        */
    clr_ADCI;

    /* 设置该位开始A/D转换 
        - 如果ADCEX为1 也由STADC置位 当ADC忙时该位保持高 在ADCI置位后立即复位
        - 在置位ADCS之前建议先清ADCI 然而 如果ADCI清零与ADCS置位同时进行 相同通道将开始一次新的A/D 转换
        - 软件清ADCS将中止转换 当ADCS或ADCI为高时 ADC 不能进行新的转换 
        */
    set_ADCS;

    /* 空闲模式 设置该位使MCU进入空闲模式 
        - 在此模式下 CPU时钟停止 且程序计数器PC挂起
        - CPU从空闲模式唤醒后 该位自动由硬件清零 且在系统唤醒之前程序继续执行中断服务程序ISR
        - 从ISR返回后 设备继续执行系统进入掉电模式时所处的指令 
        */
    PCON |= SET_BIT0;

    /* Brown-Out Detector 电源电压检测 */
    check_BOD();
}

/* 可以不使用 ptemp 只使用 p 指针即可 */
uint16_t adc_sensor_filter(uint16_t *p)
{
    uint8_t i;
    uint32_t sum = 0;
    uint16_t maxtemp = 0, mintemp = 0;
    uint16_t *ptemp = p;

    for (i = 0; i < 10; i++)
    {
        /* Brown-Out Detector 电源电压检测 */
        check_BOD();
        /* 第一次采样 */
        if (!i)
        {
            /* 总值等于第一个数组成员的值 */
            sum = *ptemp;
            /* 最大、最小值等于总值 */
            maxtemp = mintemp = sum;
        }
        else
        {
            /* 总值累加 */
            sum += *(ptemp + i);
            /* 获取十次采样中的最大值 */
            if (*(ptemp + i) > maxtemp)
                maxtemp = *(ptemp + i);

            /* 获取十次采样中的最小值 */
            if (*(ptemp + i) < mintemp)
                mintemp = *(ptemp + i);
        }
    }

    /* 总值中去掉最大值和最小值 */
    sum -= (maxtemp + mintemp);
    /* 总值除以8 因为已经去掉两个成员 最大值和最小值 */
    sum = sum >> 3;

    return (uint16_t)sum;
}

/* ADC 采样中断服务程序 Vecotr @ 0x5B */
void ADC_ISR(void) interrupt 11
{   
    /* Clear ADC flag 
        - ADCI = 0 ADC空闲
        - ADCI = 1 ADC转换结果已经可以读取 如果中断使能 会产生一个中断 不能由软件置位
    */
    clr_ADCI;

    /* 设置该位开始A/D转换 
        - 如果ADCEX为1 也由STADC置位 当ADC忙时该位保持高 在ADCI置位后立即复位
        - 在置位ADCS之前建议先清ADCI 然而 如果ADCI清零与ADCS置位同时进行 相同通道将开始一次新的A/D 转换
        - 软件清ADCS将中止转换 当ADCS或ADCI为高时 ADC 不能进行新的转换 
    */
    clr_ADCS;
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/


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
 *    v1.0.0                 2020��1��1��          Unknown
 *    Modification: �����ĵ�
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
    /* AAA Brown-Out Detector ��Դ��ѹ��� */
    check_BOD();

    /* �ر�P0.1 �������߼������������ ����P0.1 Ϊ����(����)ģʽ */
    adc_set_input_mode(E_CHANNEL0);
    /* AADR2 AADR1 AADR2 = 000B ѡ��ADC0 ��P0.1��ΪADC�����˿� */
    adc_channel_sel(E_CHANNEL0);

    /* ��ADC��· */
    set_ADCEN;
}

void adc_set_input_mode(enum E_ADCCNL_SEL channel)
{
    switch (channel)
    {
    case E_CHANNEL0:
        /* �ر�P0.1 �������߼������������ */
        P0DIDS |= SET_BIT1;
        /* ����P0.1 ����(����)ģʽ */
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
        /* AADR2 AADR1 AADR2 = 000B ѡ��ADC0 ��P0.1��ΪADC�����˿� */
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

/* ѡ��ADC0 ��P0.1��ΪADC�����˿� ��ADC�ж�ʹ�� */
void adc_init_interrupt(void)
{
    /* �ر�P0.1 �������߼������������ ����P0.1 Ϊ����(����)ģʽ */
    adc_set_input_mode(E_CHANNEL0);
    /* AADR2 AADR1 AADR2 = 000B ѡ��ADC0 ��P0.1��ΪADC�����˿� */
    adc_channel_sel(E_CHANNEL0);

    /* ��ADC�жϺ�ȫ���ж� */
    adc_enable_interrupt();
    /* ��ADC��· */
    set_ADCEN;
}

void adc_enable_interrupt(void)
{
    /* ��ADC�ж� */
    EADC = 1;
    /* ��ȫ���ж� */
    EA = 1;
}

uint16_t adc_sensor(void)
{
    uint16_t temp[10];
    uint8_t i;

    /* �ɼ�10�������˲� */
    for (i = 0; i < 10; i++)
    {
        /* AAA Brown-Out Detector ��Դ��ѹ��� */
        check_BOD();        
        /* ��ȡ���ε�ADC������� */
        temp[i] = adc_single_sample();
    }

    /* �����˲����ֵ */
    return adc_sensor_filter(temp);
}

uint16_t adc_single_sample(void)
{
    uint16_t u16_ADCL;
    uint16_t u16_ADC;
    adc_trigger_convertion();

    /* ��ADCCON0��ȡ ADC 10bit �ɼ�ֵ�����2��bit ADC.1 ADC.0 ADC[1:0]*/
    u16_ADCL = ADCCON0;
    u16_ADCL = u16_ADCL >> 6;
    /* ��ADCH�л�ȡ ADC ת�����λ ADC[9:2] */
    u16_ADC = ADCH;
    /* ��ADC�Ĳ�������ϳ� 10bit */
    u16_ADC = (u16_ADC << 2) + u16_ADCL;

    return u16_ADC;
}

void adc_trigger_convertion(void)
{
    /* Clear ADC flag 
        - ADCI = 0 ADC����
        - ADCI = 1 ADCת������Ѿ����Զ�ȡ ����ж�ʹ�� �����һ���ж� �����������λ
        */
    clr_ADCI;

    /* ���ø�λ��ʼA/Dת�� 
        - ���ADCEXΪ1 Ҳ��STADC��λ ��ADCæʱ��λ���ָ� ��ADCI��λ��������λ
        - ����λADCS֮ǰ��������ADCI Ȼ�� ���ADCI������ADCS��λͬʱ���� ��ͬͨ������ʼһ���µ�A/D ת��
        - �����ADCS����ֹת�� ��ADCS��ADCIΪ��ʱ ADC ���ܽ����µ�ת�� 
        */
    set_ADCS;

    /* ����ģʽ ���ø�λʹMCU�������ģʽ 
        - �ڴ�ģʽ�� CPUʱ��ֹͣ �ҳ��������PC����
        - CPU�ӿ���ģʽ���Ѻ� ��λ�Զ���Ӳ������ ����ϵͳ����֮ǰ�������ִ���жϷ������ISR
        - ��ISR���غ� �豸����ִ��ϵͳ�������ģʽʱ������ָ�� 
        */
    PCON |= SET_BIT0;

    /* Brown-Out Detector ��Դ��ѹ��� */
    check_BOD();
}

/* ���Բ�ʹ�� ptemp ֻʹ�� p ָ�뼴�� */
uint16_t adc_sensor_filter(uint16_t *p)
{
    uint8_t i;
    uint32_t sum = 0;
    uint16_t maxtemp = 0, mintemp = 0;
    uint16_t *ptemp = p;

    for (i = 0; i < 10; i++)
    {
        /* Brown-Out Detector ��Դ��ѹ��� */
        check_BOD();
        /* ��һ�β��� */
        if (!i)
        {
            /* ��ֵ���ڵ�һ�������Ա��ֵ */
            sum = *ptemp;
            /* �����Сֵ������ֵ */
            maxtemp = mintemp = sum;
        }
        else
        {
            /* ��ֵ�ۼ� */
            sum += *(ptemp + i);
            /* ��ȡʮ�β����е����ֵ */
            if (*(ptemp + i) > maxtemp)
                maxtemp = *(ptemp + i);

            /* ��ȡʮ�β����е���Сֵ */
            if (*(ptemp + i) < mintemp)
                mintemp = *(ptemp + i);
        }
    }

    /* ��ֵ��ȥ�����ֵ����Сֵ */
    sum -= (maxtemp + mintemp);
    /* ��ֵ����8 ��Ϊ�Ѿ�ȥ��������Ա ���ֵ����Сֵ */
    sum = sum >> 3;

    return (uint16_t)sum;
}

/* ADC �����жϷ������ Vecotr @ 0x5B */
void ADC_ISR(void) interrupt 11
{   
    /* Clear ADC flag 
        - ADCI = 0 ADC����
        - ADCI = 1 ADCת������Ѿ����Զ�ȡ ����ж�ʹ�� �����һ���ж� �����������λ
    */
    clr_ADCI;

    /* ���ø�λ��ʼA/Dת�� 
        - ���ADCEXΪ1 Ҳ��STADC��λ ��ADCæʱ��λ���ָ� ��ADCI��λ��������λ
        - ����λADCS֮ǰ��������ADCI Ȼ�� ���ADCI������ADCS��λͬʱ���� ��ͬͨ������ʼһ���µ�A/D ת��
        - �����ADCS����ֹת�� ��ADCS��ADCIΪ��ʱ ADC ���ܽ����µ�ת�� 
    */
    clr_ADCS;
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/


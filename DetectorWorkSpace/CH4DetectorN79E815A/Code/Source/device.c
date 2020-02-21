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
 *    v1.0.0                 2020��1��1��          Unknown
 *    Modification: �����ĵ�
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
/* �豸��һ���ϵ��־ */
bit device_first_power_on = false;

/* �豸������� */
uint8_t device_power_down_count = 0;

/* ȫ���жϴ洢λ */
bit EA_save_bit;

/* �豸�ķ�״̬ */
bit device_valve_state = false;
/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void device_init(void)
{
    /* ȫ���ж�ʹ��λ 0 = �ر������ж� */
    EA = 0;

    /* P0.0����Ϊ�������ģʽ ����P0�˿�����Ϊ׼˫��ģʽ 
        - ���ó�����������д������������
        - P0.0 ��ŷ�����IO
        */
    /* �˿�0���ģʽ����1 */
    P0M1 = 0x00;
    /* �˿�0���ģʽ����2 */
    P0M2 = 0x01;

    /* P1.6 P1.7����Ϊ�������ģʽ ����P1�˿�����Ϊ׼˫��ģʽ 
        - I2C��PxM1,PxM2��Ӧλ��Ҫ���ó�1
        - P1.6 ICPDAT
        - P1.7 ICPCLK
        */
    /* �˿�1���ģʽ����1 */
    P1M1 |= 0xC0;
    /* �˿�1���ģʽ����2 */
    P1M2 |= 0xC0;

    /* P2.2 P2.4����Ϊ�������ģʽ ����P2�˿�����Ϊ׼˫��ģʽ 
        - ���ó�����������д������������
        - P2.2 �̵�������IO
        - P2.4 ��Դ�ƿ���IO
        - P2.7 ����������IO
        */
    /* �˿�2���ģʽ����1 */
    P2M1 = 0x00;
    /* �˿�2���ģʽ����2 */
    P2M2 = 0x94;

    /* P3M1->P1Sλ ʹ�ܶ˿�P1��ʷ���ش������뻺�� ��ǿ�ִ��������� ��Ҫ���I2C */
    P3M1 |= 0x20;

    /* �򿪴��������� */
    SENSER_ON;

    /* P2.2 = 0 �̵����� */
    DELAY_OFF;
    /* P0.0 = 0 ��ŷ��� */
    VALVE_OFF;

    delay_1ms_without_BOD(3000);

    /* P2.2 = 0 �̵����� */
    DELAY_OFF;
    /* P0.0 = 0 ��ŷ��� */
    VALVE_OFF;

    /* ���ӹ���ѡ��Ĵ��� ѡ�񴮿ڹܽ�
        - DPS      = 0 ѡ���׼8051��DPTR 
        - DisP26   = 0 ��P2.6���������������
        - UART_Sel = 0 ѡ��P1.0 P1.1 ��Ϊ���ڹܽ�
        - SPI_Sel  = 0 ѡ��P1.7 P1.6 P1.4 P0.0 ��ΪSPI�ܽ�
        */
    AUXR1 = UAER_PORT_SELECT;

    /* �ؼ�SFR��д���� */
    TA = 0xAA;
    TA = 0x55;
    /* PMCR��Դ�����ƼĴ������� PMCR = 1100 0000B 
        - BODEN[7]       = 1B ʹ��Ƿѹ���
        - BOV[6]         = 1B ���CBOV ����Ƿѹ���Ϊ3.8V
        - Reserved3[5]   = 0B
        - BORST[4]       = 0B ��VDD�½���������VBOD ��ֹǷѹ��⸴λ ��VDD�½���VBOD���� оƬ����λBOF 
        - BOF[3]         = 0B Ƿѹ����־λ ��VDD�½���������VBOD��λ
        - Reserved2[2]   = 0B ����Ϊ0
        - Reserved1[1:0] = 00B 
        */
    PMCR = 0xc0;

    /* ȫ���ж�ʹ��λ 1 = �������ж� */
    EA = 1;

    /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
    EBO = 1;
    /* ʹ�ܴ����ж� 1 = �趨TI(SCON.1)��RI(SCON.0)*/
    ES = 1;
    /* ��ʱ��2���п��� 1 = �򿪶�ʱ��2 */
    TR2 = 1;
    /* ��չ�ж�ʹ�ܼĴ���<EIE> 1 = �򿪶�ʱ��2�ж� */
    ET2 = 1;
    /* �ж����ȼ��Ĵ������ֽ� PBODH = 1 ����BOD����жϸ����ȼ�Ϊ������ȼ� */
    IPH |= 0X20;

    delay_1ms_without_BOD(1000);

    // /* 9600 Baud Rate @ 22.1184MHz */
    // uart_init(9600);

    /* 4800 Baud Rate @ 22.1184MHz */
    uart_init(4800);

    /* ��ʱ��2��ʼ�� */
    timer2_init();

    /* ADC��ʼ�� */
    adc_init();
    
    /* I2C��ʼ�� */
    i2c_init();
}

/* Brown-Out Detector ��Դ��ѹ���  Power-On Reset �ϵ縴λ */
void check_BOD(void)
{
    /* ����豸����������� ���е��紦���� */
    if (device_power_down_count >= POWER_DOWN_LIMIT)
    {
        /* �̵����� */
        DELAY_OFF;
        /* ��ŷ��� */
        VALVE_OFF;
        /* ���������عر� */
        SENSER_OFF;
        /* ���ϵƹ� */
        LED_FAULT_OFF;
        /* �����ƹ� */
        LED_ALARM_OFF;
        /* �����������ƹ� */
        LED_LIFE_OFF;
        /* ������1�� */
        SOUND1_OFF;
        /* ������2�� */
        SOUND2_OFF;
        /* ��Դ�ƹ� */
        LED_POWER_OFF;

        /* ���ڵ�ƽ���� */
        P10 = 0;
        P11 = 0;
        
        /* ��FLASH��д�����¼ */
        flash_write_record(POWER_DOWN_RECORD);

        /* ���������λ */
        device_power_down_count = 0;
        /* Ԥ�ȱ�Ǹ�λ */
        sensor_preheat_flag = false;
        /* Ԥ��ʱ������ */
        sensor_preheat_time_count = 0;

        /* �����λ */
        while (1)
        {
            /* �ر�ȫ���ж� */
            EA = 0;

            /* ISP ���ܵĹؼ�SFR��д���� */
            TA = 0xAA;
            TA = 0x55;
            /* ��CHPCON�е� BS = 0 
                - д�������˸�λ��MCU���������� 0 = ��һ�δ�APROM���� 1 = ��һ�δ�LDROM���� 
                - ����������ǰ�θ�λ��MCU���������� 0 = ǰ�δ�APROM���� 1 = ǰ�δ�LDROM���� */
            CHPCON &= 0xfd;

            /* ISP ���ܵĹؼ�SFR��д���� */
            TA = 0xAA;
            TA = 0x55;
            /* ��CHPCON�е� SWRST = 1 ���ø�λΪ�߼�1�����������λ ��λ��ɺ���Ӳ���Զ����� */
            CHPCON |= 0x80;
        }
    }
}

/* Brown-Out Detector ��Դ��ѹ����жϷ������ Vecotr @ 0x43 */
void BOD_ISR(void) interrupt 8
{
    /* ���Ƿѹ����־ */
    clr_BOF;
    /* �豸��⵽������� */
    device_power_down_count++;
    if (device_power_down_count >= POWER_DOWN_LIMIT)
    {
        device_power_down_count = POWER_DOWN_LIMIT;
        /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
        EBO = 0;
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

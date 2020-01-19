/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      sensor.c
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

#include "sensor.h"
#include "signal.h"
#include "delay.h"
#include "adc.h"
#include "uart.h"

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
/* Ԥ�ȱ�� */
bit sensor_preheat = false;

/* Ԥ��ʱ����� */
uint16_t sensor_preheat_time_count = 0;

/* ���� ���ADֵ */
uint16_t ch4_0 = 0;
/* ���� 3500��ADֵ */
uint16_t ch4_3500 = 712;
/* �궨ADֵ�洢���� */
uint8_t demarcation_result[4] = 0;

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
/* �궨���� */
/*  1.�������� ������
    2.��Դ����������3��,�����뱨���͹�������ͬ��������3����Ԥ��
    3.��ʼ��ȡAD��if ((adcvalue < 41) || (adcvalue > 260))������Ϊ�ǹ��ϣ����ϵ��������궨ʧ��
    4.if (adcvalue < 410)�����Դ�����ϣ�������ȫ�����ȴ�����>=410������ϡ�����������100s������ָʾ
    5.if ((temp_data < 410)||(temp_data > 950))����Ӧֵ���Ϸ������ϵ������궨ʧ��
    6.�궨�ɹ��������Ƴ�����������ѭ���ȴ�����
    */
void sersor_demarcation(void)
{
    uint8_t demarcate_key;
    uint16_t i = 0;
    uint16_t adcvalue = 0;
    uint16_t temp_data = 0;
    uint16_t d1 = 0;

    /* ��ȡ P0.6 ��ֵ sbit P06 = P0 ^ 6 (sbit �� ^ ���� ָΪ�Ĵ���ĳλȡ�ı�� ΪC51�������� �������) */
    demarcate_key = P06 & 0x01;

    /* ���Լ����� */
    if (demarcate_key == 0x00)
    {
        /* ��ʱȥ�� */
        Delay1ms(50);
        /* ��ֹ��ʱ��2->�رյ�Դ����˸ ���λ����ֹ��ʱ��2������ǰ����������TH2��TL2 */
        TR2 = 0;
        /* �ٴλ�ȡP0.6 ��ֵ */
        demarcate_key = P06 & 0x01;
        /* ȷ�ϵ��Լ����� */
        if (demarcate_key == 0x00)
        {
            /* �궨״̬��������ʾ ��������һ�� */
            device_alarm(Alarm_Demarcation);
            
            /* ѡ��ADC0 ��P0.1��ΪADC�����˿� ��ADC�ж�ʹ�� */
            adc_init_interrupt();

            /* �豸Ԥ�ȱ��Ϊ��Ԥ�� */
            sensor_preheat = true;
            
            /* �ȴ� 255 * 2 = 510s XXX Ԥ�� */
            for (i = 0; i < 225; i++)
            {
                /* ��Դ�ƿ� */
                LED_POWER_ON;
                Delay1ms(1500);
                /* ��Դ�ƹ� */
                LED_POWER_OFF;
                Delay1ms(500);

                /* ͨ�����������Ե�ŷ��ͼ̵��� */
                /* ��ȡP0.7 ��ֵ */
                demarcate_key = P07 & 0x01;
                /* P0.7 �������� */
                if (demarcate_key == 0x00)
                {
                    /* ��ʱȥ�� */
                    Delay1ms(30);
                    /* �ٴλ�ȡP0.7 ��ֵ */
                    demarcate_key = P07 & 0x01;
                    /* ȷ��P0.7 �������� */
                    if (demarcate_key == 0x00)
                    {
                        /* ��ŷ��� */
                        VALVE_ON;
                        Delay1ms(1500);
                        /* �̵����� */
                        DELAY_ON;
                    }
                }
                else
                {
                    /* �̵�����  */
                    DELAY_OFF;
                    /* ��ŷ��� */
                    VALVE_OFF;
                }
            }

            /* �õ�ADC�������˲�֮���ֵ */
            adcvalue = Adc_Sensor();
            /* ���ò���ֵ�洢��ch4_0�� ch4_0Ϊ0������0���ֵ */
            ch4_0 = adcvalue;

            /* ��������ôβ�����ֵ */
            uart_send(0x1f);
            Delay1ms(5);
            uart_send(0x1f);
            Delay1ms(5);
            uart_send(ch4_0 >> 8);
            Delay1ms(5);
            uart_send(ch4_0);
            Delay1ms(5);

            /* �ٴεõ�ADC�������˲�֮���ֵ */
            adcvalue = Adc_Sensor();

            while (1)
            {
                /* �õ�ADC�������˲�֮���ֵ */
                adcvalue = Adc_Sensor();
                
                /* ����ֵ�� �������ж� AD������Χ 0.2V-1.27V ����ϵ��� */
                if ((adcvalue < 41) || (adcvalue > 260))
                {
                    Delay1ms(1000);
                    if ((adcvalue < 41) || (adcvalue > 260))
                    {
                        while (1)
                        {
                            LED_FAULT_ON;
                            Delay1ms(500);
                            LED_FAULT_OFF;
                            Delay1ms(500);
                        }
                    }
                }

                /* ����ADC����ֵС��410ʱִ�� ������ֵ����410ʱ �˳���ѭ�� �˾��ڵȴ�����Ũ������ */
                do
                {
                    /* �ٴεõ�ADC�������˲�֮���ֵ */
                    adcvalue = Adc_Sensor();

                    /* <1> ��Դ ���� ���� ���Ƴ��� */
                    /* ��Դ�ƿ� */
                    LED_POWER_ON;
                    /* ���ϵƿ� */
                    LED_FAULT_ON;
                    /* �����ƿ� */
                    LED_ALARM_ON;

                    /* ������ �������������ֵ */
                    if (++d1 > 1200)
                    {
                        d1 = 0;
                        uart_send(0x2f);
                        Delay1ms(5);
                        uart_send(0x2f);
                        Delay1ms(5);
                        uart_send(adcvalue >> 8);
                        Delay1ms(5);
                        uart_send(adcvalue);
                        Delay1ms(5);
                    }
                } while (adcvalue < 410);

                Delay1ms(250);
                Delay1ms(250);

                /* Wait 120s �˾��ڵȴ�����Ũ������ */
                for (i = 0; i <= 300; i++)
                {
                    /* <2> ��Դ�Ƴ��� �����������ŵ���˸ */
                    Delay1ms(500);

                    /* �����ƿ� */
                    LED_ALARM_ON;
                    /* ���ϵƿ� */
                    LED_FAULT_ON;

                    Delay1ms(1500);

                    /* �����ƹ� */
                    LED_ALARM_OFF;
                    /* ���ϵƹ� */
                    LED_FAULT_OFF;

                    /* ��������ôβ�����ֵ */
                    uart_send(0x3f);
                    Delay1ms(5);
                    uart_send(0x3f);
                    Delay1ms(5);
                    uart_send(adcvalue >> 8);
                    Delay1ms(5);
                    uart_send(adcvalue);
                    Delay1ms(5);
                }

                /* �ݴ�õ�ADC�������˲�֮���ֵ */
                temp_data = Adc_Sensor();

                /* ����ֵ �������ж� AD������Χ ADC_result < 410 or  ADC_result > 950 */
                if ((temp_data < 410) || (temp_data > 950))
                {
                    Delay1ms(1000);

                    /* �ٴ�ȷ�� ����ֵ �������ж� AD������Χ ADC_result < 410 or  ADC_result > 950 */
                    if (temp_data < 410 || temp_data > 950)
                    {
                    /* <5> ����״̬ ���ϵƳ��� */
                    ERROR:
                        while (1)
                        {
                            LED_FAULT_ON;
                        }
                    }
                }
                /* ����ֵ �������ж� AD�ڷ�Χ�� 410 <= ADC_result <= 950 ��궨�ɹ� */
                else
                {
                    /* <3> ��Դ�Ƴ��� �����������Źر� */
                    /* ���ϵƹ� */
                    LED_FAULT_OFF;
                    /* �����ƹ� */
                    LED_ALARM_OFF;
                    /* �ݴ� �õ�ADC�������˲�֮���ֵ */
                    temp_data = Adc_Sensor();

                    /* ���ݴ��ADC����ֵ ����ch4_3500�� ch4_3500Ϊ������ 3500 XXX ��ֵ */
                    ch4_3500 = temp_data;

                    demarcation_result[0] = ch4_0;
                    demarcation_result[1] = ch4_0 >> 8;
                    demarcation_result[2] = ch4_3500;
                    demarcation_result[3] = ch4_3500 >> 8;

                    /* XXX ��ch4_0 �� ch4_3500 ����FLASH�� */
                    WriteData(demarcation_result, 4, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], Life_start_OFFSET_DEMA_CH4_0);

                    /* ��I2Cʱ��оƬ�ж�ȡʱ��� */
                    Master_Read_Data();

                    /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
                    EBO = 0;
                    /* ���ƴ�ʱ��оƬ��ȡ��ʱ��� */
                    timecodeTotimecopy();
                    /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
                    EBO = 1;

                    /* �洢�궨��¼���� */
                    WriteRecordData(DEM_RECORD);
                    
                    /* XXX ��Flash�ж�ȡ ch4_0 �� ch4_3500 ���� �����бȶ� */
                    ReadData(Time_Code, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD] + Life_start_OFFSET_DEMA_CH4_0, 4);

                    if (Time_Code[0] != demarcation_result[0])
                        goto ERROR;
                    if (Time_Code[1] != demarcation_result[1])
                        goto ERROR;
                    if (Time_Code[2] != demarcation_result[2])
                        goto ERROR;
                    if (Time_Code[3] != demarcation_result[3])
                        goto ERROR;
                    
                    /* ���ͱ궨�ı��������� ch4_3500 */
                    uart_send(0x4f);
                    Delay1ms(5);
                    uart_send(0x4f);
                    Delay1ms(5);
                    uart_send(ch4_3500 >> 8);
                    Delay1ms(5);
                    uart_send(ch4_3500);
                    Delay1ms(5);
                }

                /* <4> �궨��� ��Դ �����Ƴ��� ���ϵƹر� */
                while (1)
                {
                    /* �����ƿ� */
                    LED_ALARM_ON;
                }
            }
        }
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

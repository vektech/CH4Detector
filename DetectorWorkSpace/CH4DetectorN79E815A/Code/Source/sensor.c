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
#include "i2c.h"
#include "utlities.h"
#include "flash.h"

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
bit sensor_preheat_flag = false;

/* Ԥ��ʱ����� */
uint16_t sensor_preheat_time_count = 0;

/* ���� ���ADֵ */
uint16_t sensor_ch4_0 = 0;
/* ���� 3500��ADֵ */
uint16_t sensor_ch4_3500 = 712;
/* �궨ADֵ�洢���� */
uint8_t sensor_demarcation_result[4] = 0;

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
/* �궨���� */
/*  1.�������� ������
    2.��Դ����������3��,�����뱨���͹�������ͬ��������3����Ԥ��
    3.��ʼ��ȡAD��if ((adc_value < 41) || (adc_value > 260))������Ϊ�ǹ��ϣ����ϵ��������궨ʧ��
    4.if (adc_value < 410)�����Դ�����ϣ�������ȫ�����ȴ�����>=410������ϡ�����������100s������ָʾ
    5.if ((temp_data < 410)||(temp_data > 950))����Ӧֵ���Ϸ������ϵ������궨ʧ��
    6.�궨�ɹ��������Ƴ�����������ѭ���ȴ�����
    */
void sersor_demarcation(void)
{
    uint8_t demarcate_key;
    uint16_t i = 0;
    uint16_t adc_value = 0;
    uint16_t temp_data = 0;
    uint16_t delay_count = 0;

    /* ��ȡ P0.6 ��ֵ sbit P06 = P0 ^ 6 (sbit �� ^ ���� ָΪ�Ĵ���ĳλȡ�ı�� ΪC51�������� �������) */
    demarcate_key = P06 & 0x01;

    /* ���Լ����� */
    if (demarcate_key == 0x00)
    {
        /* ��ʱȥ�� */
        delay_1ms(50);
        /* ��ֹ��ʱ��2->�رյ�Դ����˸ ���λ����ֹ��ʱ��2������ǰ����������TH2��TL2 */
        TR2 = 0;
        /* �ٴλ�ȡP0.6 ��ֵ */
        demarcate_key = P06 & 0x01;
        /* ȷ�ϵ��Լ����� ����궨״̬ */
        if (demarcate_key == 0x00)
        {
            /* �궨״̬��������ʾ ��������һ�� */
            device_alarm(Alarm_Demarcation);
            
            /* ѡ��ADC0 ��P0.1��ΪADC�����˿� ��ADC�ж�ʹ�� */
            /* ZZZ �Ƿ����ֻʹ�� adc_init_interrupt() ������Ҫ adc_init() */
            adc_init_interrupt();

            /* �豸Ԥ�ȱ��Ϊ��Ԥ�� */
            sensor_preheat_flag = true;
            
            /* ---- 1.������Ԥ�� ---- */
            /* �ȴ� 255 * 2 = 510s */
            for (i = 0; i < 225; i++)
            {
                /* ��Դ�ƿ� */
                LED_POWER_ON;
                delay_1ms(1500);

                /* ��Դ�ƹ� */
                LED_POWER_OFF;
                delay_1ms(500);

                /* ͨ������ ���Ե�ŷ��ͼ̵��� */
                /* ��ȡP0.7 ��ֵ */
                demarcate_key = P07 & 0x01;
                /* P0.7 �������� */
                if (demarcate_key == 0x00)
                {
                    /* ��ʱȥ�� */
                    delay_1ms(30);
                    /* �ٴλ�ȡP0.7 ��ֵ */
                    demarcate_key = P07 & 0x01;
                    /* ȷ��P0.7 �������� */
                    if (demarcate_key == 0x00)
                    {
                        /* ��ŷ��� */
                        VALVE_ON;

                        delay_1ms(1500);

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
            adc_value = adc_sensor();
            /* ���ò���ֵ�洢��sensor_ch4_0�� sensor_ch4_0Ϊ0������0���ֵ */
            sensor_ch4_0 = adc_value;

            /* ��������ôβ�����ֵ */
#ifdef _DEBUG_
            uart_send(0x1f);
            delay_1ms(5);
            uart_send(0x1f);
            delay_1ms(5);
            uart_send(sensor_ch4_0 >> 8);
            delay_1ms(5);
            uart_send(sensor_ch4_0);
            delay_1ms(5);
#endif

            /* �ٴεõ�ADC�������˲�֮���ֵ */
            adc_value = adc_sensor();

            /* ZZZ �˴�����ṹ���Ը��� ����Ҫѭ��ȫ������ */
            while (1)
            { 
                /* �õ�ADC�������˲�֮���ֵ */
                adc_value = adc_sensor();
                
                /* ---- 2.�����������ж� ---- */
                /* ���������ߺ������ж� AD������Χ 0.2V-1.27V ����ϵ��� */
                if ((adc_value < 41) || (adc_value > 260))
                {
                    delay_1ms(1000);
                    if ((adc_value < 41) || (adc_value > 260))
                    {
                        while (1)
                        {   
                            /* ���ϵƿ� */
                            LED_FAULT_ON;
                            delay_1ms(500);

                            /* ���ϵƹ� */
                            LED_FAULT_OFF;
                            delay_1ms(500);
                        }
                    }
                }

                /* ---- 3.���ݲɼ���ADֵ �ȴ�����Ũ������ ---- */
                /* ����ADC����ֵС��410ʱִ�� ������ֵ����410ʱ �˳���ѭ�� �˾����ڵȴ�����Ũ��������410���� */
                do
                {
                    /* �ٴεõ�ADC�������˲�֮���ֵ */
                    adc_value = adc_sensor();

                    /* <1> ��Դ ���� ���� ���Ƴ��� */
                    /* ��Դ�ƿ� */
                    LED_POWER_ON;
                    /* ���ϵƿ� */
                    LED_FAULT_ON;
                    /* �����ƿ� */
                    LED_ALARM_ON;

                    /* ������ �������������ֵ */
                    if (++delay_count > 1200)
                    {
                        delay_count = 0;
#ifdef _DEBUG_                        
                        uart_send(0x2f);
                        delay_1ms(5);
                        uart_send(0x2f);
                        delay_1ms(5);
                        uart_send(adc_value >> 8);
                        delay_1ms(5);
                        uart_send(adc_value);
                        delay_1ms(5);
#endif                        
                    }
                } while (adc_value < 410);

                delay_1ms(250);
                delay_1ms(250);

                /* ---- 4.�ȴ�����Ũ������ δ���вɼ� ---- */
                /* Wait 120s �˾��ڵȴ�����Ũ������ */
                for (i = 0; i <= 300; i++)
                {
                    /* <2> ��Դ�Ƴ��� �����������ŵ���˸ */
                    delay_1ms(500);

                    /* �����ƿ� */
                    LED_ALARM_ON;
                    /* ���ϵƿ� */
                    LED_FAULT_ON;

                    delay_1ms(1500);

                    /* �����ƹ� */
                    LED_ALARM_OFF;
                    /* ���ϵƹ� */
                    LED_FAULT_OFF;

#ifdef _DEBUG_
                    /* ��������ôβ�����ֵ */
                    uart_send(0x3f);
                    delay_1ms(5);
                    uart_send(0x3f);
                    delay_1ms(5);
                    uart_send(adc_value >> 8);
                    delay_1ms(5);
                    uart_send(adc_value);
                    delay_1ms(5);
#endif                    
                }

                /* �ݴ�õ�ADC�������˲�֮���ֵ */
                temp_data = adc_sensor();

                /* ---- 5.���б�����ֵ��¼ ---- */
                /* ������ AD������Χ ADC_result < 410 or  ADC_result > 950 */
                if ((temp_data < 410) || (temp_data > 950))
                {
                    delay_1ms(1000);

                    /* �ٴεõ�ADC�������˲�֮���ֵ */
                    temp_data = adc_sensor();
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
                /* ���� AD�ڷ�Χ�� 410 <= ADC_result <= 950 ��궨�ɹ� */
                else
                {
                    /* <3> ��Դ�Ƴ��� �����������Źر� */
                    /* ���ϵƹ� */
                    LED_FAULT_OFF;
                    /* �����ƹ� */
                    LED_ALARM_OFF;
                    /* �ݴ� �õ�ADC�������˲�֮���ֵ */
                    temp_data = adc_sensor();

                    /* ���ݴ��ADC����ֵ ����sensor_ch4_3500�� sensor_ch4_3500Ϊ������ 3500 ZZZ ��ֵ */
                    sensor_ch4_3500 = temp_data;

                    /* ������3500���ADC�������ݴ��������� */
                    sensor_demarcation_result[0] = sensor_ch4_0;
                    sensor_demarcation_result[1] = sensor_ch4_0 >> 8;
                    sensor_demarcation_result[2] = sensor_ch4_3500;
                    sensor_demarcation_result[3] = sensor_ch4_3500 >> 8;

                    /* ����������������FLASH�� */
                    flash_write_data(sensor_demarcation_result, 4, DEVICE_INFO_ADDR, OFFSET_OF_CH4_0);

                    /* ��I2Cʱ��оƬ�ж�ȡʱ��� */
                    i2c_get_time();

                    /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
                    EBO = 0;
                    /* ���ƴ�ʱ��оƬ��ȡ��ʱ��� */
                    format_to_device_time();
                    /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
                    EBO = 1;
                    
                    /* ��Flash�ж�ȡ sensor_ch4_0 �� sensor_ch4_3500 ���� �����бȶ� ʹ�� i2c_time_code �ݴ� */
                    flash_read_data(i2c_time_code, ADC_VALUE_OF_CH4_0_ADDR, 4);

                    if (i2c_time_code[0] != sensor_demarcation_result[0])
                        goto ERROR;
                    if (i2c_time_code[1] != sensor_demarcation_result[1])
                        goto ERROR;
                    if (i2c_time_code[2] != sensor_demarcation_result[2])
                        goto ERROR;
                    if (i2c_time_code[3] != sensor_demarcation_result[3])
                        goto ERROR;
#ifdef _DEBUG_                    
                    /* ���ͱ궨�ı��������� sensor_ch4_3500 */
                    uart_send(0x4f);
                    delay_1ms(5);
                    uart_send(0x4f);
                    delay_1ms(5);
                    uart_send(sensor_ch4_3500 >> 8);
                    delay_1ms(5);
                    uart_send(sensor_ch4_3500);
                    delay_1ms(5);
#endif                    
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

/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      main.c
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
#include "GlobalFormat8051Core.h"

#include "uart.h"
#include "delay.h"
#include "sensor.h"
#include "device.h"
#include "signal.h"
#include "timer.h"
#include "i2c.h"
#include "utlities.h"
#include "adc.h"
#include "flash.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
/*

/*
    P2.0 ������2
    P2.1 ������1
    P2.3 �̵���
    P2.4 ������
    P2.5 ���ϵ�
    P2.6 ������������

    P1.3  SDA
    P1.2  SCL
    P1.1  RXD
    P1.0  TXD
    P1.4  T_INT

    P0.0  ��ŷ�
    P0.1  ������AD����
    P0.2  ��ŷ����
    P0.3  ��Դ��
    P0.5  ��Դ��ѹ���
    P0.6  ���ԡ��궨
    P0.7  ���԰���
*/

/*  
    �±�0����ѯ�����¼����;
    �±�1��������¼ 200;
    �±�2�������ָ���¼ 200;
    �±�3�����ϼ�¼ 100;
    �±�4�����ϻָ���¼ 100;
    �±�5�������¼ 50;
    �±�6���ϵ��¼ 50;
    �±�7��������ʧЧ��¼ 1;
    �±�8���ڲ���ʱ����ǰʱ��
*/

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/
/* �豸״̬���� */
uint8_t device_status[2]={0};

#define STATUS1_WARMUP   device_status[1] &= 0x1f
#define STATUS1_NOMAL    device_status[1] =(device_status[1] & 0x1f)|0X20
#define STATUS1_ALARM    device_status[1] =(device_status[1] & 0x1f)|0X40
#define STATUS1_FAULT    device_status[1] =(device_status[1] & 0x1f)|0X60

#define NORMAL 0
#define Unknown_EROR 1
#define CHECKSUM_EROR 2
#define BYTELOSS_EROR 3
#define ILLEGAL_PARA_EROR 4
#define COMMAND_EROR 5

/* ��Ʒ���кŵ�ַ 0x3af0 �������� */
code uint8_t SERIAL_ADD[8] _at_ 0x3af0;

code uint8_t COMMAND_LEN_EASE_REC[2]={4,6};
code uint8_t COMMAND_LEN_WRITE_CLOCK[2]={10,6};
code uint8_t COMMAND_LEN_WR_DATEOFPRODUCTION[2]={9,6};
code uint8_t COMMAND_LEN_RE_DATEOFPRODUCTION[2]={4,11};
code uint8_t COMMAND_LEN_RE_SERIALNUM[2]={4,14};
code uint8_t COMMAND_LEN_RE_CLOCK[2]={4,12};
code uint8_t COMMAND_LEN_RE_MODEL[2]={4,12};
code uint8_t COMMAND_LEN_RE_CURTAD[2]={4,8};
code uint8_t COMMAND_LEN_RE_DEMA[2]={4,10};
code uint8_t COMMAND_LEN_CANCEL_WARMUP[2]={4,6};

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void main(void)
{
    uint8_t production_date[5] = {0};

    /* �������������ڼ���־ */
    uint8_t life_check[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t life_check_flag = false;
    
    /* д�������������ڼ�¼��־ */
    bit write_expired_record_flag = false;

    /* д�����������ָ���¼��־ */
    bit write_no_exceeded_record_flag = false;

    /* д���������ϻָ���¼��־ */
    bit write_no_fault_record_flag = false;

    /* �Լ찴�� */
    uint8_t selfcheck_key;

    /* ���Դ������� */
    uint8_t i;

    /* ����������ADֵ */
    uint16_t ch4_adc_value;

    /* ʱ����±�־ */
    bit device_time_renew_flag = false;
    /* ������ֵ�趨��־ */
    bit device_threshold_flag = false;

    /* ch4 �߹���ֵ�ı�����־ */
    bit exceeded_flag = false;

    /* ch4 �߹���ֵ�Ĵ��� */
    uint8_t exceeded_count = 0;
    /* ch4 �߹���ֵʱ���������� */
    uint8_t exceeded_valve_count = 0;

    /* ch4 �������ֵ�Ĵ��� ���ϱ�־ */
    bit fault_flag = false;

    /* ch4 �������ֵ�Ĵ��� ���ϼ��� */
    uint8_t fault_count = 0;

    /* �豸������� */
    device_power_down_count = 0;
    /* timer2 ����� */
    timer2_life_second_count = 0;
    /* Ԥ�ȱ�� */
    sensor_preheat_flag = false;

    /* �豸��ʼ�� */
    device_init();
    
    /* �豸�궨 */
    sersor_demarcation();

    /* �ϵ�״̬λΪfalse ����һ���ϵ��¼ */
    if (device_first_power_on == false)
    {
        device_first_power_on = true;
        /* �����ϵ��¼ */
        flash_write_record(POWER_ON_RECORD);
    }

    /* WARMUP ����ϵͳ״̬ device_status[1] &= 0x1f */
    STATUS1_WARMUP;

    /* �����������Լ�3s�� ȡ��Ԥ�� */
    /* ��ȡ���� P0.7 ��ֵ ����Լ칦���Ƿ񴥷� */
    selfcheck_key = P07 & 0x01;
    /* ���Լ����� */
    if (selfcheck_key == 0x00)
    {   
        /* ��ʱȥ�� */
        delay_1ms(50);
        /* ���԰�������ʱ������ */
        while ((P07 & 0x01) == 0x00)
        {   
            /* ���ݶ�ʱ�� �������´���һ�� */
            if (timer2_life_second_count > 1)
            {
                /* NOMAL ����ϵͳ״̬ device_status[1] = (device_status[1] & 0x1f) | 0X20 */
                STATUS1_NOMAL;
                sensor_preheat_flag = true;

                /* �����Ŀŵ���˸�ͷ��������� */
                device_alarm(Alarm_Selfcheck);
                break;
            }
        }
    }
    /* ��ʱ����������� */
    timer2_life_second_count = 0;

    /* ��Flash�洢��ַ SENSOR_EXPIRED_RECORD_ADDR �ж�ȡ����������Ϣ */
    flash_read_data(life_check, SENSOR_EXPIRED_RECORD_ADDR, 4);
    /* life_check_flag ��ʾ ����������ʱ�� */
    if (life_check[0] == 0xa5 && (life_check[1] == 0x36))
    {
        if (life_check[2] == 0x5a && (life_check[3] == 0xe7))
        {
                life_check_flag = true;
        }
    }

    /* ��Ԥ����� */
    if (sensor_preheat_flag == true)
    {
        /* ��Դ�ƿ� */
        LED_POWER_ON;

        /* û�д�FLASH�ж�ȡ�궨ֵ */
        if (!device_threshold_flag)
        {
            device_threshold_flag = true;
            /* ���Դ�FLASH�ж�ȡ�궨���� 10�� */
            for (i = 0; i < 10; i++)
            {
                /* ����I2C ������ʱ��д�� */
                i2c_start_rtc();
                /* ��FLASH�ж�ȡ�궨���� ch4_0 & ch4_3500 �������궨���ݵ�ַ������ 4 bytes */
                flash_read_data(sensor_demarcation_result, ADC_VALUE_OF_CH4_0_ADDR, 4);

                /* ���� sensor_ch4_0 */
                sensor_ch4_0 = sensor_demarcation_result[1];
                sensor_ch4_0 = sensor_ch4_0 << 8;
                sensor_ch4_0 |= sensor_demarcation_result[0];
                /* ���� sensor_ch4_3500 */
                sensor_ch4_3500 = sensor_demarcation_result[3];
                sensor_ch4_3500 = sensor_ch4_3500 << 8;
                sensor_ch4_3500 |= sensor_demarcation_result[2];

                /* ���δ�궨���߶�ȡ�ı궨�������� ������Ĭ�ϵı�����ֵ 
                    - sensor_ch4_0 = 150 
                    - sensor_ch4_3500 = 750 
                    - �ٴγ��Զ�ȡ
                    */
                if (sensor_ch4_3500 >= 1024 || (sensor_ch4_3500 <= sensor_ch4_0) || (sensor_ch4_0 <= 0))
                {
                    sensor_ch4_0 = 150;
                    sensor_ch4_3500 = 750;
                }
                /* �궨�������� ���� */
                else
                {
                    break;
                }
            }
        }
    }
    
    while (1)
    {
        /* Brown-Out Detector ��Դ��ѹ��� */
        check_BOD();

        /* ÿ����ִ��һ�� ��RTC�����豸ʱ�� */
        if (!(timer2_life_second_count % 2))
        {
            /* û�д�I2C�ж�ȡʱ�� */
            if (!device_time_renew_flag)
            {   
                /* �豸ʱ���Ѹ��� */
                device_time_renew_flag = true;
                /* Brown-Out Detector ��Դ��ѹ��� */
                check_BOD();
                /* ��ȡ��ǰʱ�� ����Time_Code */
                i2c_get_time();
                /* ZZZ */
                if (i2c_time_code[6] < 15 || (i2c_time_code[5] > 12) || (i2c_time_code[3] > 31) || (i2c_time_code[2] > 23) || (i2c_time_code[1] > 59) || (i2c_time_code[0] > 59))
                {
                    /* ZZZ �� i2c_time_code ת��Ϊ time_data */
                    format_to_RTC_time();
                }
                else
                {
                    /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
                    EBO = 0;
                    /* ZZZ ���ƴ�ʱ��оƬ��ȡ��ʱ��� �� time_data ת��Ϊ i2c_time_code */
                    format_to_device_time();
                    /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
                    EBO = 1;
                }
            }
        }
        else
        {
            device_time_renew_flag = 0;
        }

        /* �����������ѵ��� */
        if (life_check_flag == false)
        {
            /* �����������ƿ� */
            LED_LIFE_ON;
        }

        /* 1Сʱʱ�䵽�ͽ���һ�������Ƚ� */
        if (timer2_life_hour_flag == true)
        {
            timer2_life_hour_flag = false;

            /* Brown-Out Detector ��Դ��ѹ��� */
            check_BOD();
            /* ��ȡ��ǰʱ�� ����Time_Code */
            i2c_get_time();

            /* ��Flash�ж�ȡ�������� */
            flash_read_data(production_date, PRODUCTION_DATE_ADDR, 5);
            /* ��ȡ��������ʧ�� ������Ĭ����������Ϊ 15��12��31��23ʱ59�� */
            if (production_date[0] >= 255 || (production_date[1] >= 255) || (production_date[2] >= 255))
            {
                production_date[0] = 15;
                production_date[1] = 12;
                production_date[2] = 31;
                production_date[3] = 23;
                production_date[4] = 59;
            }

            /* ���ж�ȡ�ĵ�ǰʱ��ͼ�¼���������ڵıȽ� */
            /* ��ǰʱ����� ���� ���������е��� */
            if (i2c_time_code[6] >= production_date[0])
            {
                /* ������� */
                production_date[0] = i2c_time_code[6] - production_date[0];
                /* ��� �Ѿ��������� �� 2020 - 2015 = 5 */
                if (production_date[0] == SENSOR_LIFE)
                {
                    /* ��ǰʱ����� ���� ���������е��� 2020.6 > 2015.5 */
                    if (i2c_time_code[5] > production_date[1])
                    {
                        /* ��ʹ������ ��1 ��Ϊ6 */
                        production_date[0]++;
                    }
                    /* ��ǰʱ����� ���� ���������е��� 2020.5 = 2015.5 */
                    if (i2c_time_code[5] == production_date[1])
                    {
                        /* ��ǰʱ����� ���� ���������е��� 2020.5.6 > 2015.5.5 */
                        if (i2c_time_code[3] > production_date[2])
                        {
                            /* ��ʹ������ ��1 ��Ϊ6 */
                            production_date[0]++;
                        }
                        /* ��ǰʱ����� ���� ���������е��� 2020.5.5 = 2015.5.5 */
                        if (i2c_time_code[3] == production_date[2])
                        {
                            /* �Դ����ƱȽ�ʱ */
                            if (i2c_time_code[2] > production_date[3])
                            {
                                /* ��ʹ������ ��1 ��Ϊ6 */
                                production_date[0]++;
                            }
                            /* ʱ��� */
                            if (i2c_time_code[2] == production_date[3])
                            {
                                /* �Դ����ƱȽϷ� */
                                if (i2c_time_code[1] > production_date[4])
                                {
                                    /* ��ʹ������ ��1 ��Ϊ6 */
                                    production_date[0]++;
                                }
                            }
                        }
                    }
                }

                /* ʹ��������ʱ 5��ʱ�䵽 */
                if (production_date[0] > SENSOR_LIFE)
                {
                    /* �����������ƿ� */
                    LED_LIFE_ON;
                    
                    /* дʧЧ��¼��־ */
                    if (write_expired_record_flag == false)
                    {
                        write_expired_record_flag = true;

                        /* ��FLASH�ж�ȡ �������ڴ洢��ַ �ĵ�һ���ֽ� 1����ʧЧ 0����δʧЧ ʹ�� production_date �ݴ� */
                        flash_read_data(production_date, SENSOR_EXPIRED_RECORD_ADDR, 1);
                        /* ���ֽ�Ϊ0 ���� ����Ӧ��ĵ��ڼ�¼1�� */
                        if (production_date[0] == 0 || (production_date[0] > max_of_each_record[SENSOR_EXPIRED_RECORD]))
                        {
                            /* Brown-Out Detector ��Դ��ѹ��� */
                            check_BOD();
                            /* д�������ڼ�¼ */
                            flash_write_record(SENSOR_EXPIRED_RECORD);
                        }
                    }
                }
                else
                {
                    if (life_check_flag == true)
                    {
                        /* �����������ƹ� */
                        LED_LIFE_OFF;
                    }
                }
            }
        }

        /* ��ȡ���� P0.7 ��ֵ ����Լ칦���Ƿ񴥷� */
        selfcheck_key = P07 & 0x01;
        /* ���Լ����� */
        if (selfcheck_key == 0x00)
        {
            /* ��ʱȥ�� */
            delay_1ms(30);
            /* ȷ�ϲ��Լ����� */
            if (selfcheck_key == 0x00)
            {
                /* ���Լ�������־δ��λ */
                if (!key_long_press_flag)
                {
                    timer2_key_long_press_count = 0;
                    /* ��λ���Լ�������־ */
                    key_long_press_flag = true;
                }
                /* �������6 ͨ��ʵ�ʲ��Գ���ʱʱ��Ϊ11s ����涨���Լ����º� ��7~30s�� Ӧ�����̵��� �� */
                if (timer2_key_long_press_count >= 6)
                {
                    /* ����� �����ر�ʱ */
                    if (!device_valve_state)
                    {
                        device_valve_state = true;

                        /* ��ŷ��� */
                        VALVE_ON;
                        delay_1ms(30);

                        /* ��ŷ��� */
                        VALVE_OFF;
                        delay_1ms(30);
                    }
                    /* �̵����� */
                    DELAY_ON;
                    timer2_key_long_press_count = 8;
                }
                device_alarm(Alarm_Selfcheck);
            }
        }
        /* ���Լ�δ���� */
        else
        {
            /* ���Լ�������־����λ */
            if (key_long_press_flag)
            {
                /* ��ʱȥ�� */
                delay_1ms(30);
                /* ȷ�ϲ��Լ�δ���� */
                if (selfcheck_key != 0x00)
                {
                    /* ���Լ�������־��� */
                    key_long_press_flag = false;
                    /* ��״̬��־��� */
                    device_valve_state = false;
                    /* ������δԤ����� */
                    if (!sensor_preheat_flag)
                    {   
                        /* �̵����� */
                        DELAY_OFF;
                    }
                }
            }
        }

        /* �õ�ADC�������˲�֮���ֵ */
        ch4_adc_value = adc_sensor();

        /* �õ�ADC�������˲�֮���ֵ ���� sensor_ch4_3500 �ı�����ֵ ����״̬���� */
        if (ch4_adc_value >= sensor_ch4_3500)
        {
            /* Brown-Out Detector ��Դ��ѹ��� */
            check_BOD();
            /* Ԥ����� */
            if (sensor_preheat_flag)
            {
                /* ����Ϊ����״̬ */
                STATUS1_ALARM;
                /* ����P0.0Ϊ�������ģʽ */
                P0M1 = 0x00;
                P0M2 = 0x01;

                /* ÿ����ʱ����ʱһ�� */
                if (timer2_second_flag == true)
                {
                    timer2_second_flag = false;
                    /* ch4 �߹���ֵ�Ĵ��� */
                    exceeded_count++;

                    /* ch4 �߹���ֵ�Ĵ������� 3 */
                    if (exceeded_count == 3)
                    {
                        exceeded_flag = true;
                    }

                    /* ������������ 5 */
                    if (exceeded_count > 5)
                    {
                        exceeded_count = 5;
                        /* �̵����� */
                        DELAY_ON;
                        /* ������ִ�еĴ���Ϊ8�ı��� */
                        if (!(exceeded_valve_count % 8))
                        {
                            /* ��ŷ��� */
                            VALVE_ON;
                            delay_1ms(30);

                            /* ��ŷ��� */
                            VALVE_OFF;
                            delay_1ms(30);
                        }
                        exceeded_valve_count++;
                    }
                }

                /* ch4 �߹���ֵ�ı�����־��λ */
                if (exceeded_flag == true)
                {
                    exceeded_flag = false;
                    /* д�������ָ���¼��־Ϊ�� ����������ָ������д�ָ���¼ */
                    write_no_exceeded_record_flag = true;
                    /* Brown-Out Detector ��Դ��ѹ��� */
                    check_BOD();
                    /* ��FLASH�� д������¼ */
                    flash_write_record(ALARM_RECORD);
                }

                /* ������������3���� */
                if (exceeded_count >= 3)
                {
                    /* �������������ź� */
                    device_alarm(Alarm_CH4_Exceeded);
                }
            }
        }
        /* �õ�ADC�������˲�֮���ֵ С�� Short_Fault_L �Ĺ�����ֵ ����״̬���� */
        else if (ch4_adc_value <= Short_Fault_L)
        {
            /* Brown-Out Detector ��Դ��ѹ��� */
            check_BOD();

            /* Ԥ����� */
            if (sensor_preheat_flag)
            {
                /* ����ϵͳ״̬Ϊ����״̬ */
                STATUS1_FAULT;

                exceeded_count = 0;
                exceeded_valve_count = 0;

                /* �����ƹ� */
                LED_ALARM_OFF;
                /* �̵����� */
                DELAY_OFF;

                /* ���й��ϼ��� */
                fault_count++;
                if (fault_count == 1)
                {
                    fault_flag = true;
                }
                if (fault_count > 2)
                {
                    fault_count = 2;
                }

                /* ���������ź� */
                device_alarm(Alarm_Fault);

                /* ch4 ������Сֵ�Ĺ��ϱ�־��λ */
                if (fault_flag == true)
                {
                    fault_flag = false;
                    /* д���������ϻָ���¼��־Ϊ�� ����������ӹ��ϻָ������д�ָ���¼ */
                    write_no_fault_record_flag = 1;
                    /* Brown-Out Detector ��Դ��ѹ��� */
                    check_BOD();
                    /* ��FLASH�� д���ϼ�¼ */
                    flash_write_record(FAULT_RECORD);
                }
            }
        }
        /* �õ�ADC�������˲�֮���ֵ ���� Short_Fault_L �Ĺ�����ֵ С�� sensor_ch4_3500 �ı�����ֵ ����״̬���� */
        else
        {   
            /* ���Ԥ����� */
            if (sensor_preheat_flag)
            {
                STATUS1_NOMAL;
            }
            /* Brown-Out Detector ��Դ��ѹ��� */
            check_BOD();

            /* ch4 ������ֵ������ر�־�ͼ������� */
            exceeded_flag = false;
            exceeded_count = 0;
            exceeded_valve_count = 0;

            /* ch4 �������ֵ���ϱ�����ر�־�ͼ������� */
            fault_flag = 0;
            fault_count = 0;

            /* δд�����������ָ���¼ */
            if (write_no_exceeded_record_flag == true)
            {
                write_no_exceeded_record_flag = false;
                /* Brown-Out Detector ��Դ��ѹ��� */
                check_BOD();
                /* ��FLASH�� д�����ָ���¼ */
                flash_write_record(ALARM_RECOVERY_RECORD);
            }

            /* δд���������ϻָ���¼ */
            if (write_no_fault_record_flag == 1)
            {
                write_no_fault_record_flag = 0;
                fault_flag = 0;

                /* Brown-Out Detector ��Դ��ѹ��� */
                check_BOD();
                /* ��FLASH�� д���ϻָ���¼ */
                flash_write_record(FAULT_RECOVERY_RECORD);
            }

            /* �����ƹ� */
            LED_ALARM_OFF;
            /* ���ϵƹ� */
            LED_FAULT_OFF;

            /* ������1�� */
            SOUND1_OFF;
            /* ������2�� */
            SOUND2_OFF;
            /* ��ŷ��� */
            VALVE_OFF;

            /* ����״̬Ϊ�ر�ʱ */
            if (!device_valve_state)
            {
                /* �̵����� */
                DELAY_OFF;
            }
        }

        /* ---- UART ͨѶ����� ----*/
        /* UART������� */
        if (rx_finished)
        {   
            /* ZZZ ȡ��һ���ֽڵ����һλ ���Ϊ0B ��ʾ�ǹ�װ�巢�������� */
            // if (!(uart_buffer[0] & 0x80))
            // {
            //     if (rx_index >= 9)
            //     {
            //         if (get_crc(uart_buffer, rx_index) == uart_buffer[rx_index - 2])
            //         {
            //             for (i = 0; i < 5; i++)
            //             {
            //                 if (SERIAL_ADD[i + 3] != uart_buffer[i + 1])
            //                     break;
            //             }
            //             /* ��ʾ��ַ������ */
            //             if (i == 5)
            //             {
            //                 for (i = 6; i < rx_index; i++)
            //                 {
            //                     /* �ѵ�ַ��Ϣ�ӽ���֡��ȥ�� */
            //                     uart_buffer[i - 5] = uart_buffer[i];
            //                 }
            //                 /* �ѵ�ַ���ֽ�����ȥ */
            //                 rx_index -= 5;
            //                 /* ������ָ� */ 
            //                 uart_buffer[0] |= 0x80;
            //                 /* ���¼���crc */
            //                 uart_buffer[rx_index - 2] = get_crc(uart_buffer, rx_index);
            //             }
            //             else
            //             {
            //                 uart_buffer[0] = 0x00;
            //             }
            //         }
            //         else
            //         {
            //             uart_buffer[0] = 0x00;
            //         }
            //     }
            //     else
            //     {
            //         uart_buffer[0] = 0x00;
            //     }
            // }

            switch (uart_buffer[0])
            {
                /* 0xaa��ͷΪ ����涨��Э�� ������ */
                case 0xaa:
                {
                    /* ����ѯ�ļ�¼���� ӦС��0x09 */
                    if (uart_buffer[2] <= GB_COMMAND_MAX)
                    {
                        uart_send(0xAA);
                        delay_1ms(5);
                        uart_send(0xBB);
                        delay_1ms(5);
                        uart_send(0xCC);
                        delay_1ms(5);
                        uart_send(0xDD);
                        delay_1ms(5);
                        /* ����ѯ��������С�ڸ����͵ļ�¼���� */
                        if ((uart_buffer[1] <= max_of_each_record[uart_buffer[2]]))
                        {
                            /* �����У���ͨ�� */
                            if ((rx_index >= (GB_READ_FRAME_LEN - 1)) && uart_buffer[4] == get_crc(uart_buffer, GB_READ_FRAME_LEN))
                            {
                                /* ��ѯ��¼ */
                                flash_read_record(uart_buffer[2], uart_buffer[1]);
                            }
                        }
                    }
                    break;
                }
                /* 0xab��ͷΪд���� */
                case 0xab:
                {
                    switch (uart_buffer[1])
                    {
                        /* ��������EEP �������м�¼���豸��Ϣ */
                        case 0x00:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_EASE_REC[0]) == uart_buffer[COMMAND_LEN_EASE_REC[0] - 2])
                            {
                                for (i = 0; i < 28; i++)
                                {
                                    /* ��ҳ���� */
                                    flash_erase_page(RECORD_START_ADDR + i);
                                    /* Brown-Out Detector ��Դ��ѹ��� */
                                    check_BOD();
                                }

                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                            }
                            uart_buffer[2] = device_status[0];
                            uart_buffer[COMMAND_LEN_EASE_REC[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_EASE_REC[1]);
                            uart_buffer[COMMAND_LEN_EASE_REC[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_EASE_REC[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }

                            break;
                        }
                        /* дʱ��ָ�� */
                        case 0x01:
                        {
                            /* дʱ��ָ�� */
                            if ((rx_index >= COMMAND_LEN_WRITE_CLOCK[0]))
                            {
                                if (get_crc(uart_buffer, COMMAND_LEN_WRITE_CLOCK[0]) == uart_buffer[COMMAND_LEN_WRITE_CLOCK[0] - 2])
                                {
                                    if (uart_buffer[2] < 15)
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }
                                    if (uart_buffer[3] > 12 || (!uart_buffer[3]))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }
                                    if (uart_buffer[4] > 31 || (!uart_buffer[4]))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }

                                    if ((uart_buffer[5] > 23) || (uart_buffer[6] > 59) || (uart_buffer[7] > 59))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }

                                    for (i = 2; i < 8; i++)
                                    {
                                        /* ʮ������תΪBCD�� */
                                        i2c_time_code[8 - i] = hex2bcd(uart_buffer[i]);
                                    }
                                    i2c_time_code[0] = i2c_time_code[1];
                                    i2c_time_code[1] = i2c_time_code[2];
                                    i2c_time_code[2] = i2c_time_code[3];
                                    i2c_time_code[3] = i2c_time_code[4];
                                    i2c_time_code[4] = 0;

                                    /* д������ */
                                    i2c_set_time();

                                    delay_1ms(100);
                                    i2c_get_time();
                                    /* �£���ʾд��ʱ�䲻�ɹ� */
                                    if (i2c_time_code[5] != uart_buffer[3] && (i2c_time_code[5] != (uart_buffer[3] + 1)))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                        ;
                                    }
                                    else
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0);
                                    }
                                    /* �꣺��ʾд��ʱ�䲻�ɹ� */
                                    if (i2c_time_code[6] != uart_buffer[2] && (i2c_time_code[6] != (uart_buffer[2] + 1)))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    /* �գ���ʾд��ʱ�䲻�ɹ� */
                                    if (i2c_time_code[3] != uart_buffer[4] && (i2c_time_code[3] != (uart_buffer[4] + 1)))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    /* ʱ����ʾд��ʱ�䲻�ɹ� */
                                    if (i2c_time_code[2] != uart_buffer[5] && (i2c_time_code[2] != (uart_buffer[5] + 1)))
                                    {
                                        if (!((uart_buffer[5] == 23) && (i2c_time_code[2] == 0)))
                                            uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    /* �֣���ʾд��ʱ�䲻�ɹ� */
                                    if (i2c_time_code[1] != uart_buffer[6] && (i2c_time_code[1] != (uart_buffer[6] + 1)))
                                    {
                                        if (!((uart_buffer[6] == 59) && (i2c_time_code[1] == 0)))
                                            uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                }
                                else
                                {
                                    uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                                    ;
                                }
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | BYTELOSS_EROR;
                            }
                        WR_CLOCK_EROR:
                            uart_buffer[2] = device_status[0];
                            uart_buffer[COMMAND_LEN_WRITE_CLOCK[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_WRITE_CLOCK[1]);
                            uart_buffer[COMMAND_LEN_WRITE_CLOCK[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_WRITE_CLOCK[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* �������  ��ʱ�������ؼ����� */
                                delay_1ms(5);
                            }

                            /* ��ʾʱ��д��ɹ� û�з������� */
                            if ((uart_buffer[3] & (~0xe0)) == 0)
                            {
                                life_check[0] = 0xa5;
                                life_check[1] = 0x36;
                                life_check_flag = 1;

                                flash_write_data(life_check, 4, DEVICE_INFO_ADDR, OFFSET_OF_RTC);
                            }
                            break;
                        }
                        /* д�����������ĳ�ʱʱ�� ���������� */
                        case 0x02:
                        {
                            if ((rx_index >= COMMAND_LEN_WR_DATEOFPRODUCTION[0])) 
                            {
                                if (get_crc(uart_buffer, COMMAND_LEN_WR_DATEOFPRODUCTION[0]) == uart_buffer[COMMAND_LEN_WR_DATEOFPRODUCTION[0] - 2]) //
                                {
                                    flash_write_data(&uart_buffer[2], 5, PRODUCTION_DATE_ADDR, 0);
                                    flash_read_data(&uart_buffer[7], PRODUCTION_DATE_ADDR, 5);
                                    if (uart_buffer[8] != uart_buffer[3])
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    else
                                        uart_buffer[3] = (device_status[1] & 0xe0);
                                    if (uart_buffer[7] != uart_buffer[2])
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    if (uart_buffer[9] != uart_buffer[4])
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                }
                                else
                                {
                                    uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                                }
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | BYTELOSS_EROR;
                            }
                            uart_buffer[2] = device_status[0];
                            uart_buffer[COMMAND_LEN_WR_DATEOFPRODUCTION[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_WR_DATEOFPRODUCTION[1]);
                            uart_buffer[COMMAND_LEN_WR_DATEOFPRODUCTION[1] - 1] = 0x55;

                            for (i = 0; i < COMMAND_LEN_WR_DATEOFPRODUCTION[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            /* ��ʾ��������д��ɹ� */
                            if ((uart_buffer[3] & (~0xe0)) == 0)
                            {
                                life_check[2] = 0x5a;
                                life_check[3] = 0xe7;
                                life_check_flag = 1;

                                flash_write_data(life_check, 4, DEVICE_INFO_ADDR, OFFSET_OF_RTC);
                            }
                            break;
                        }
                        default:
                        {
                            uart_buffer[2] = device_status[0];
                            uart_buffer[3] = (device_status[1] & 0xe0) | COMMAND_EROR;
                            uart_buffer[4] = get_crc(uart_buffer, 6);
                            uart_buffer[5] = 0x55;
                            for (i = 0; i < 6; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }
                    }
                    break;
                }
                /* 0xac��ͷ Ϊ������ */
                case 0xac:
                {
                    switch (uart_buffer[1])
                    {
                        /* ��ʱ�� ���� */
                        case 0x01:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_RE_CLOCK[0]) == uart_buffer[COMMAND_LEN_RE_CLOCK[0] - 2])
                            {
                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }
                            uart_buffer[2] = device_status[0];
                            i2c_get_time();
                            uart_buffer[4] = i2c_time_code[6];
                            uart_buffer[5] = i2c_time_code[5];
                            uart_buffer[6] = i2c_time_code[3];
                            uart_buffer[7] = i2c_time_code[2];
                            uart_buffer[8] = i2c_time_code[1];
                            uart_buffer[9] = i2c_time_code[0];
                            uart_buffer[COMMAND_LEN_RE_CLOCK[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_CLOCK[1]);
                            uart_buffer[COMMAND_LEN_RE_CLOCK[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_CLOCK[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }

                        /* ��ģ���ͺŵ���Ϣ */
                        case 0x02:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_RE_MODEL[0]) == uart_buffer[COMMAND_LEN_RE_MODEL[0] - 2])
                            {
                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }
                            uart_buffer[2] = device_status[0];

                            uart_buffer[4] = 0x01;
                            uart_buffer[5] = 0;
                            uart_buffer[6] = 0;
                            uart_buffer[7] = 0;
                            uart_buffer[8] = 0;
                            uart_buffer[9] = 0;
                            uart_buffer[COMMAND_LEN_RE_MODEL[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_MODEL[1]);
                            uart_buffer[COMMAND_LEN_RE_MODEL[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_MODEL[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }
                        
                        /* ��AD */
                        case 0x03:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_RE_CURTAD[0]) == uart_buffer[COMMAND_LEN_RE_CURTAD[0] - 2])
                            {
                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }
                            uart_buffer[2] = device_status[0];

                            uart_buffer[4] = ch4_adc_value >> 8;
                            uart_buffer[5] = ch4_adc_value;
                            uart_buffer[COMMAND_LEN_RE_CURTAD[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_CURTAD[1]);
                            uart_buffer[COMMAND_LEN_RE_CURTAD[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_CURTAD[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }

                        /* ���궨�����Ϣ */
                        case 0x04:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_RE_DEMA[0]) == uart_buffer[COMMAND_LEN_RE_DEMA[0] - 2])
                            {
                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }
                            uart_buffer[2] = device_status[0];

                            /* ��FLASH�ж�ȡ�궨���� ch4_0 & ch4_3500 �������궨���ݵ�ַ������ 4 bytes */
                            flash_read_data(sensor_demarcation_result, ADC_VALUE_OF_CH4_0_ADDR, 4);

                            uart_buffer[8] = uart_buffer[4];
                            uart_buffer[4] = uart_buffer[5];
                            uart_buffer[5] = uart_buffer[8];
                            uart_buffer[8] = uart_buffer[6];
                            uart_buffer[6] = uart_buffer[7];
                            uart_buffer[7] = uart_buffer[8];
                            uart_buffer[COMMAND_LEN_RE_DEMA[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_DEMA[1]);
                            uart_buffer[COMMAND_LEN_RE_DEMA[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_DEMA[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }

                        /* �������������ĳ�ʼʱ�� ���������� */
                        case 0x05:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_RE_DATEOFPRODUCTION[0]) == uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[0] - 2])
                            {
                                uart_buffer[2] = device_status[0];
                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[2] = device_status[0];
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }
                            /* ��FLASH�ж�ȡ�������� */
                            flash_read_data(&uart_buffer[4], PRODUCTION_DATE_ADDR, 5);

                            uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_DATEOFPRODUCTION[1]);
                            uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_DATEOFPRODUCTION[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }

                        /* �����к� */
                        case 0x06:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_RE_SERIALNUM[0]) == uart_buffer[COMMAND_LEN_RE_SERIALNUM[0] - 2])
                            {
                                uart_buffer[2] = device_status[0];
                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[2] = device_status[0];
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }

                            for (i = 0; i < 8; i++)
                            {
                                uart_buffer[4 + i] = SERIAL_ADD[i];
                            }
                            uart_buffer[COMMAND_LEN_RE_SERIALNUM[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_SERIALNUM[1]);
                            uart_buffer[COMMAND_LEN_RE_SERIALNUM[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_SERIALNUM[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }
                        
                        default:
                        {

                            uart_buffer[2] = device_status[0];
                            uart_buffer[3] = (device_status[1] & 0xe0) | COMMAND_EROR;
                            uart_buffer[4] = get_crc(uart_buffer, 6);
                            uart_buffer[5] = 0x55;
                            for (i = 0; i < 6; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }
                    }
                    break;
                }
                /* 0xad��ͷ ΪXXX���� */
                case 0xad:
                {
                    switch (uart_buffer[1])
                    {
                        /* ȡ��Ԥ�� */
                        case 0x01:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_CANCEL_WARMUP[0]) == uart_buffer[COMMAND_LEN_CANCEL_WARMUP[0] - 2])
                            {
                                STATUS1_NOMAL;
                                uart_buffer[3] = device_status[1] & 0xe0;
                                sensor_preheat_flag = 1;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }
                            uart_buffer[2] = device_status[0];
                            uart_buffer[COMMAND_LEN_CANCEL_WARMUP[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_CANCEL_WARMUP[1]);
                            uart_buffer[COMMAND_LEN_CANCEL_WARMUP[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_CANCEL_WARMUP[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* ������� ��ʱ���� �ؼ����� */
                                delay_1ms(5);
                            }
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            rx_index = 0;
            rx_finished = 0;

            /* ʹ��UART0���� */
            UART0_RX_ENABLE;
        }
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

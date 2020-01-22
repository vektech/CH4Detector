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

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
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
/*******************************************************************************
 *                 Struct Define Section ('typedef')
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
void main(void)
{
    /* �������������ڼ���־ */
    uint8_t life_check[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t life_check_flag = 0;

    /* �豸��ʼ�� */
    device_init();
    
    /* �豸�궨 */
    sersor_demarcation();

    /* XXX �ϵ�״̬λΪ0 ����һ���ϵ��¼ */
    // if (device_first_power_on == false)
    // {
    //     device_first_power_on = true;
    //     /* �����ϵ��¼ */
    //     WriteRecordData(UPPOWER_RECORD);
    // }

    // /* �ر�ȫ���ж� */
    // EA = 0;
    // /* �ؼ�SFR��д���� */
    // TA = 0xAA;
    // TA = 0x55;
    // /* PMCR��Դ�����ƼĴ������� PMCR = 1100 0000B 
    //     - BODEN[7]       = 1B ʹ��Ƿѹ���
    //     - BOV[6]         = 1B ���CBOV ����Ƿѹ���Ϊ3.8V
    //     - Reserved3[5]   = 0B
    //     - BORST[4]       = 0B ��VDD�½���������VBOD ��ֹǷѹ��⸴λ ��VDD�½���VBOD���� оƬ����λBOF 
    //     - BOF[3]         = 0B Ƿѹ����־λ ��VDD�½���������VBOD��λ
    //     - Reserved2[2]   = 0B ����Ϊ0
    //     - Reserved1[1:0] = 00B 
    //     */
    // PMCR = 0xc0;

    // /* ����ȫ���ж� */
    // EA = 1;
    // /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
    // EBO = 1;

    // device_first_power_down = down_flag = 0;
    // timer2_life_second_count = bReadtime = sensor_preheat = 0;

    // /* WARMUP ����ϵͳ״̬ status[1] &= 0x1f */
    // STATUS1_WARMUP;

    // /* ��ȡ���� P0.7 ��ֵ ����Լ칦���Ƿ񴥷� */
    // Test_key = P07 & 0x01;
    // /* ���Լ����� */
    // if (Test_key == 0x00)
    // {   
    //     /* ��ʱȥ�� */
    //     Delay1ms(50);
    //     /* ���԰�������ʱ������ */
    //     while ((P07 & 0x01) == 0x00)
    //     {
    //         /* XXX */
    //         if (timer2_life_second_count > 1)
    //         {
    //             /* NOMAL ����ϵͳ״̬ status[1] = (status[1] & 0x1f) | 0X20 */
    //             STATUS1_NOMAL;
    //             sensor_preheat = 1;

    //             /* �����Ŀŵ���˸�ͷ��������� */
    //             Light_Flash();
    //             break;
    //         }
    //     }
    // }
    // /* XXX */
    // timer2_life_second_count = 0;

    // /* ---- ʱ�Ӽ�� ---- */
    // /* ��Flash�洢��ַ(RECORD_FIRST_ADDRESS[LIFE_RECORD] + 30) �ж�ȡ����������Ϣ */
    // ReadData(LifeCheck, RECORD_FIRST_ADDRESS[LIFE_RECORD] + 30, 4);
    // /* Life_Check_flag��ʾ ����������ʱ�� */
    // if (LifeCheck[0] == 0xa5 && (LifeCheck[1] == 0x36))
    // {
    //     if (LifeCheck[2] == 0x5a && (LifeCheck[3] == 0xe7))
    //         Life_Check_flag = 1;
    // }

    // while (1)
    // {
    //     /* XXX �������� */
    //     if (Life_Check_flag == 0)
    //         LED_LIFE_ON;

    //     /* XXX Ԥ����� */
    //     if (sensor_preheat)
    //     {
    //         /* ��Դ�ƿ� */
    //         LED_POWER_ON;

    //         /* XXX û�д�FLASH�ж�ȡ�궨ֵ */
    //         if (!bReaddemarcation_result)
    //         {
    //             bReaddemarcation_result = 1;
    //             i = 0;
    //             /* ���Դ�FLASH�ж�ȡ�궨���� 10�� */
    //             while (i < 10)
    //             {
    //                 /* XXX ����I2C ������ʱ��д�� */
    //                 startClock();
    //                 /* ��FLASH�ж�ȡ�궨���� */
    //                 ReadData(demarcation_result, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD] + Life_start_OFFSET_DEMA_CH4_0, 4);

    //                 /* ���� ch4_0 */
    //                 ch4_0 = demarcation_result[1];
    //                 ch4_0 = ch4_0 << 8;
    //                 ch4_0 |= demarcation_result[0];
    //                 /* ���� ch4_3500 */
    //                 ch4_3500 = demarcation_result[3];
    //                 ch4_3500 = ch4_3500 << 8;
    //                 ch4_3500 |= demarcation_result[2];

    //                 /* ���δ�궨���߶�ȡ�ı궨�������� ������Ĭ�ϵı�����ֵ ch4_0 = 150 ch4_3500 = 750 */
    //                 if (ch4_3500 >= 1024 || (ch4_3500 <= ch4_0) || (ch4_0 <= 0))
    //                 {
    //                     ch4_0 = 150;
    //                     ch4_3500 = 750;
    //                 }
    //                 /* �궨�������� ���� */
    //                 else
    //                 {
    //                     break;
    //                 }
    //                 i++;
    //             }
    //         }
    //     }

    //     /* XXX */
    //     if (!(timer2_life_second_count % 2))
    //     {
    //         /* XXX û�д�I2C�ж�ȡʱ�� */
    //         if (!bReadtime)
    //         {
    //             bReadtime = 1;
    //             /* Brown-Out Detector ��Դ��ѹ��� */
    //             check_BOD();
    //             /* ��ȡ��ǰʱ�� ����Time_Code */
    //             Master_Read_Data();
    //             /* XXX */
    //             if (i2c_time_code[6] < 15 || (i2c_time_code[5] > 12) || (i2c_time_code[3] > 31) || (i2c_time_code[2] > 23) || (i2c_time_code[1] > 59) || (i2c_time_code[0] > 59))
    //             {
    //                 /* XXX �� timeCOPY ת��Ϊ i2c_time_code */
    //                 timecopyTotimecode();
    //             }
    //             else
    //             {
    //                 /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
    //                 EBO = 0;
    //                 /* XXX ���ƴ�ʱ��оƬ��ȡ��ʱ��� �� i2c_time_code ת��Ϊ timeCOPY */
    //                 timecodeTotimecopy();
    //                 /* EBO ΪBOD��Դ��ѹ�����ж�ʹ��λ ���ж� sbit EBO = IE ^ 5 */
    //                 EBO = 1;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         bReadtime = 0;
    //     }

    //     /* Brown-Out Detector ��Դ��ѹ��� */
    //     check_BOD();

    //     /* ADC��ʼ�� */
    //     adc_init();

    //     /* �õ�ADC�������˲�֮���ֵ */
    //     CH4_Adc_Valu = adc_sensor();

    //     /* ---- UART ͨѶ����� ----*/
    //     /* UART������� */
    //     if (rxfinish)
    //     {   
    //         /* ȡ��һ���ֽڵ����һλ ���Ϊ0B ��ʾ�ǹ�װ�巢�������� */
    //         if (!(rxbuf[0] & 0x80))
    //         {
    //             if (rxidx >= 9)
    //             {
    //                 if (Get_crc(rxbuf, rxidx) == rxbuf[rxidx - 2])
    //                 {
    //                     for (i = 0; i < 5; i++)
    //                     {
    //                         if (SERIAL_ADD[i + 3] != rxbuf[i + 1])
    //                             break;
    //                     }
    //                     /* ��ʾ��ַ������ */
    //                     if (i == 5)
    //                     {
    //                         for (i = 6; i < rxidx; i++)
    //                         {
    //                             /* �ѵ�ַ��Ϣ�ӽ���֡��ȥ�� */
    //                             rxbuf[i - 5] = rxbuf[i];
    //                         }
    //                         /* �ѵ�ַ���ֽ�����ȥ */
    //                         rxidx -= 5;
    //                         /* ������ָ� */ 
    //                         rxbuf[0] |= 0x80;
    //                         /* ���¼���crc */
    //                         rxbuf[rxidx - 2] = Get_crc(rxbuf, rxidx);
    //                     }
    //                     else
    //                     {
    //                         rxbuf[0] = 0x00;
    //                     }
    //                 }
    //                 else
    //                 {
    //                     rxbuf[0] = 0x00;
    //                 }
    //             }
    //             else
    //             {
    //                 rxbuf[0] = 0x00;
    //             }
    //         }

    //         switch (rxbuf[0])
    //         {
    //         /* 0xab��ͷΪд���� */
    //         case 0xab:
    //         {
    //             switch (rxbuf[1])
    //             {
    //             /* ����EEP */
    //             case 0x00:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_EASE_REC[0]) == rxbuf[COMMAND_LEN_EASE_REC[0] - 2])
    //                 {
    //                     for (i = 1; i <= 8; i++)
    //                     {
    //                         ease_page(RECORD_FIRST_ADDRESS[i]);
    //                         /* Brown-Out Detector ��Դ��ѹ��� */
    //                         check_BOD();
    //                     }

    //                     rxbuf[3] = status[1] & 0xe0;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                 }
    //                 rxbuf[2] = status[0];
    //                 rxbuf[COMMAND_LEN_EASE_REC[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_EASE_REC[1]);
    //                 rxbuf[COMMAND_LEN_EASE_REC[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_EASE_REC[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }

    //                 break;
    //             }
    //             /* дʱ��ָ�� */
    //             case 0x01:
    //             {
    //                 /* дʱ��ָ�� */
    //                 if ((rxidx >= COMMAND_LEN_WRITE_CLOCK[0]))
    //                 {
    //                     if (Get_crc(rxbuf, COMMAND_LEN_WRITE_CLOCK[0]) == rxbuf[COMMAND_LEN_WRITE_CLOCK[0] - 2])
    //                     {
    //                         if (rxbuf[2] < 15)
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0) | ILLEGAL_PARA_EROR;
    //                             goto WR_CLOCK_EROR;
    //                         }
    //                         if (rxbuf[3] > 12 || (!rxbuf[3]))
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0) | ILLEGAL_PARA_EROR;
    //                             goto WR_CLOCK_EROR;
    //                         }
    //                         if (rxbuf[4] > 31 || (!rxbuf[4]))
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0) | ILLEGAL_PARA_EROR;
    //                             goto WR_CLOCK_EROR;
    //                         }

    //                         if ((rxbuf[5] > 23) || (rxbuf[6] > 59) || (rxbuf[7] > 59))
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0) | ILLEGAL_PARA_EROR;
    //                             goto WR_CLOCK_EROR;
    //                         }

    //                         for (i = 2; i < 8; i++)
    //                         {
    //                             /* ʮ������תΪBCD�� */
    //                             i2c_time_code[8 - i] = hex2bcd(rxbuf[i]);
    //                         }
    //                         i2c_time_code[0] = i2c_time_code[1];
    //                         i2c_time_code[1] = i2c_time_code[2];
    //                         i2c_time_code[2] = i2c_time_code[3];
    //                         i2c_time_code[3] = i2c_time_code[4];
    //                         i2c_time_code[4] = 0;

    //                         /* д������ */
    //                         Master_Write_Data();

    //                         Delay1ms(100);
    //                         Master_Read_Data();
    //                         /* �£���ʾд��ʱ�䲻�ɹ� */
    //                         if (i2c_time_code[5] != rxbuf[3] && (i2c_time_code[5] != (rxbuf[3] + 1)))
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                             ;
    //                         }
    //                         else
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0);
    //                         }
    //                         /* �꣺��ʾд��ʱ�䲻�ɹ� */
    //                         if (i2c_time_code[6] != rxbuf[2] && (i2c_time_code[6] != (rxbuf[2] + 1)))
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                         }
    //                         /* �գ���ʾд��ʱ�䲻�ɹ� */
    //                         if (i2c_time_code[3] != rxbuf[4] && (i2c_time_code[3] != (rxbuf[4] + 1)))
    //                         {
    //                             rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                         }
    //                         /* ʱ����ʾд��ʱ�䲻�ɹ� */
    //                         if (i2c_time_code[2] != rxbuf[5] && (i2c_time_code[2] != (rxbuf[5] + 1)))
    //                         {
    //                             if (!((rxbuf[5] == 23) && (i2c_time_code[2] == 0)))
    //                                 rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                         }
    //                         /* �֣���ʾд��ʱ�䲻�ɹ� */
    //                         if (i2c_time_code[1] != rxbuf[6] && (i2c_time_code[1] != (rxbuf[6] + 1)))
    //                         {
    //                             if (!((rxbuf[6] == 59) && (i2c_time_code[1] == 0)))
    //                                 rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                         }
    //                     }
    //                     else
    //                     {
    //                         rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                         ;
    //                     }
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | BYTELOSS_EROR;
    //                 }
    //             WR_CLOCK_EROR:
    //                 rxbuf[2] = status[0];
    //                 rxbuf[COMMAND_LEN_WRITE_CLOCK[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_WRITE_CLOCK[1]);
    //                 rxbuf[COMMAND_LEN_WRITE_CLOCK[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_WRITE_CLOCK[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* �������  ��ʱ�������ؼ����� */
    //                     Delay1ms(5);
    //                 }

    //                 /* ��ʾʱ��д��ɹ� û�з������� */
    //                 if ((rxbuf[3] & (~0xe0)) == 0)
    //                 {
    //                     LifeCheck[0] = 0xa5;
    //                     LifeCheck[1] = 0x36;
    //                     Life_Check_flag = 1;
    //                     WriteData(LifeCheck, 4, RECORD_FIRST_ADDRESS[LIFE_RECORD], 30);
    //                 }
    //                 break;
    //             }
    //             /* д�����������ĳ�ʼʱ�� ���������� */
    //             case 0x02:
    //             {
    //                 if ((rxidx >= COMMAND_LEN_WR_DATEOFPRODUCTION[0])) 
    //                 {
    //                     if (Get_crc(rxbuf, COMMAND_LEN_WR_DATEOFPRODUCTION[0]) == rxbuf[COMMAND_LEN_WR_DATEOFPRODUCTION[0] - 2]) //
    //                     {
    //                         WriteData(&rxbuf[2], 5, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 0);
    //                         ReadData(&rxbuf[7], RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 5);
    //                         if (rxbuf[8] != rxbuf[3])
    //                             rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                         else
    //                             rxbuf[3] = (status[1] & 0xe0);
    //                         if (rxbuf[7] != rxbuf[2])
    //                             rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                         if (rxbuf[9] != rxbuf[4])
    //                             rxbuf[3] = (status[1] & 0xe0) | Unknown_EROR;
    //                     }
    //                     else
    //                     {
    //                         rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                     }
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | BYTELOSS_EROR;
    //                 }
    //                 rxbuf[2] = status[0];
    //                 rxbuf[COMMAND_LEN_WR_DATEOFPRODUCTION[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_WR_DATEOFPRODUCTION[1]);
    //                 rxbuf[COMMAND_LEN_WR_DATEOFPRODUCTION[1] - 1] = 0x55;

    //                 for (i = 0; i < COMMAND_LEN_WR_DATEOFPRODUCTION[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 /* ��ʾ��������д��ɹ� */
    //                 if ((rxbuf[3] & (~0xe0)) == 0)
    //                 {
    //                     LifeCheck[2] = 0x5a;
    //                     LifeCheck[3] = 0xe7;
    //                     Life_Check_flag = 1;
    //                     WriteData(LifeCheck, 4, RECORD_FIRST_ADDRESS[LIFE_RECORD], 30);
    //                 }
    //                 break;
    //             }
    //             default:
    //             {
    //                 rxbuf[2] = status[0];
    //                 rxbuf[3] = (status[1] & 0xe0) | COMMAND_EROR;
    //                 rxbuf[4] = Get_crc(rxbuf, 6);
    //                 rxbuf[5] = 0x55;
    //                 for (i = 0; i < 6; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             }
    //             break;
    //         }
    //         /* 0xaa��ͷΪ ����涨��Э�� ������ */
    //         case 0xaa:
    //         {
    //             /* ����ѯ�ļ�¼���� ӦС��0x09 */
    //             if (rxbuf[2] <= GBCOMMANDMAX)
    //             {
    //                 /* ����ѯ��������С�ڸ����͵ļ�¼���� */
    //                 if ((rxbuf[1] <= RecordLEN[rxbuf[2]]))
    //                 {
    //                     /* �����У���ͨ�� */
    //                     if ((rxidx >= (READFrameLEN - 1)) && rxbuf[4] == Get_crc(rxbuf, READFrameLEN))
    //                     {
    //                         ReadRecordData(rxbuf[2], rxbuf[1]);
    //                     }
    //                 }
    //             }
    //             break;
    //         }
    //         /* 0xac��ͷ Ϊ������ */
    //         case 0xac:
    //         {
    //             switch (rxbuf[1])
    //             {
    //             /* ��ʱ�� ���� */
    //             case 0x01:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_RE_CLOCK[0]) == rxbuf[COMMAND_LEN_RE_CLOCK[0] - 2])
    //                 {
    //                     rxbuf[3] = status[1] & 0xe0;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                 }
    //                 rxbuf[2] = status[0];
    //                 Master_Read_Data();
    //                 rxbuf[4] = i2c_time_code[6];
    //                 rxbuf[5] = i2c_time_code[5];
    //                 rxbuf[6] = i2c_time_code[3];
    //                 rxbuf[7] = i2c_time_code[2];
    //                 rxbuf[8] = i2c_time_code[1];
    //                 rxbuf[9] = i2c_time_code[0];
    //                 rxbuf[COMMAND_LEN_RE_CLOCK[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_RE_CLOCK[1]);
    //                 rxbuf[COMMAND_LEN_RE_CLOCK[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_RE_CLOCK[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }

    //             /* ��ģ���ͺŵ���Ϣ */
    //             case 0x02:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_RE_MODEL[0]) == rxbuf[COMMAND_LEN_RE_MODEL[0] - 2])
    //                 {
    //                     rxbuf[3] = status[1] & 0xe0;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                 }
    //                 rxbuf[2] = status[0];

    //                 rxbuf[4] = 0x01;
    //                 rxbuf[5] = 0;
    //                 rxbuf[6] = 0;
    //                 rxbuf[7] = 0;
    //                 rxbuf[8] = 0;
    //                 rxbuf[9] = 0;
    //                 rxbuf[COMMAND_LEN_RE_MODEL[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_RE_MODEL[1]);
    //                 rxbuf[COMMAND_LEN_RE_MODEL[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_RE_MODEL[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             /* ��AD */
    //             case 0x03:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_RE_CURTAD[0]) == rxbuf[COMMAND_LEN_RE_CURTAD[0] - 2])
    //                 {
    //                     rxbuf[3] = status[1] & 0xe0;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                 }
    //                 rxbuf[2] = status[0];

    //                 rxbuf[4] = CH4_Adc_Valu >> 8;
    //                 rxbuf[5] = CH4_Adc_Valu;
    //                 rxbuf[COMMAND_LEN_RE_CURTAD[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_RE_CURTAD[1]);
    //                 rxbuf[COMMAND_LEN_RE_CURTAD[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_RE_CURTAD[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             /* ���궨�����Ϣ */
    //             case 0x04:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_RE_DEMA[0]) == rxbuf[COMMAND_LEN_RE_DEMA[0] - 2])
    //                 {
    //                     rxbuf[3] = status[1] & 0xe0;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                 }
    //                 rxbuf[2] = status[0];
    //                 ReadData(&rxbuf[4], RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD] + Life_start_OFFSET_DEMA_CH4_0, 4);
    //                 rxbuf[8] = rxbuf[4];
    //                 rxbuf[4] = rxbuf[5];
    //                 rxbuf[5] = rxbuf[8];
    //                 rxbuf[8] = rxbuf[6];
    //                 rxbuf[6] = rxbuf[7];
    //                 rxbuf[7] = rxbuf[8];
    //                 rxbuf[COMMAND_LEN_RE_DEMA[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_RE_DEMA[1]);
    //                 rxbuf[COMMAND_LEN_RE_DEMA[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_RE_DEMA[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             /* �������������ĳ�ʼʱ�� ���������� */
    //             case 0x05:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_RE_DATEOFPRODUCTION[0]) == rxbuf[COMMAND_LEN_RE_DATEOFPRODUCTION[0] - 2])
    //                 {
    //                     rxbuf[2] = status[0];
    //                     rxbuf[3] = status[1] & 0xe0;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[2] = status[0];
    //                     rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                 }
    //                 ReadData(&rxbuf[4], RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 5);
    //                 rxbuf[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_RE_DATEOFPRODUCTION[1]);
    //                 rxbuf[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_RE_DATEOFPRODUCTION[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             /* �����к� */
    //             case 0x06:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_RE_SERIALNUM[0]) == rxbuf[COMMAND_LEN_RE_SERIALNUM[0] - 2])
    //                 {
    //                     rxbuf[2] = status[0];
    //                     rxbuf[3] = status[1] & 0xe0;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[2] = status[0];
    //                     rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                 }

    //                 for (i = 0; i < 8; i++)
    //                 {
    //                     rxbuf[4 + i] = SERIAL_ADD[i];
    //                 }
    //                 rxbuf[COMMAND_LEN_RE_SERIALNUM[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_RE_SERIALNUM[1]);
    //                 rxbuf[COMMAND_LEN_RE_SERIALNUM[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_RE_SERIALNUM[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             default:
    //             {

    //                 rxbuf[2] = status[0];
    //                 rxbuf[3] = (status[1] & 0xe0) | COMMAND_EROR;
    //                 rxbuf[4] = Get_crc(rxbuf, 6);
    //                 rxbuf[5] = 0x55;
    //                 for (i = 0; i < 6; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             }
    //             break;
    //         }
    //         case 0xad:
    //         {
    //             switch (rxbuf[1])
    //             {
    //             /* ȡ��Ԥ�� */
    //             case 0x01:
    //             {
    //                 if (Get_crc(rxbuf, COMMAND_LEN_CANCEL_WARMUP[0]) == rxbuf[COMMAND_LEN_CANCEL_WARMUP[0] - 2])
    //                 {
    //                     STATUS1_NOMAL;
    //                     rxbuf[3] = status[1] & 0xe0;
    //                     sensor_preheat = 1;
    //                 }
    //                 else
    //                 {
    //                     rxbuf[3] = (status[1] & 0xe0) | CHECKSUM_EROR;
    //                 }
    //                 rxbuf[2] = status[0];
    //                 rxbuf[COMMAND_LEN_CANCEL_WARMUP[1] - 2] = Get_crc(rxbuf, COMMAND_LEN_CANCEL_WARMUP[1]);
    //                 rxbuf[COMMAND_LEN_CANCEL_WARMUP[1] - 1] = 0x55;
    //                 for (i = 0; i < COMMAND_LEN_CANCEL_WARMUP[1]; i++)
    //                 {
    //                     uart_send(rxbuf[i]);
    //                     /* ������� ��ʱ���� �ؼ����� */
    //                     Delay1ms(5);
    //                 }
    //                 break;
    //             }
    //             default:
    //             {
    //                 break;
    //             }
    //             }
    //             break;
    //         }
    //         default:
    //         {
    //             break;
    //         }
    //         }
    //         rxidx = 0;
    //         rxfinish = 0;

    //         /* ʹ��UART0���� */
    //         UART0_RX_ENABLE;
    //     }

    //     /* 1Сʱʱ�䵽�ͽ���һ�������Ƚ� */
    //     if (timer2_life_hour_flag == 1)
    //     {
    //         timer2_life_hour_flag = 0;

    //         /* Brown-Out Detector ��Դ��ѹ��� */
    //         check_BOD();
    //         /* ��ȡ��ǰʱ�� ����Time_Code */
    //         Master_Read_Data();

    //         /* XXX ��Flash�ж�ȡ�������� */
    //         ReadData(dateofmanufacture, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 5);
    //         /* ��ȡ��������ʧ�� ������Ĭ����������Ϊ 15��12��31��23ʱ59�� */
    //         if (dateofmanufacture[0] >= 255 || (dateofmanufacture[1] >= 255) || (dateofmanufacture[2] >= 255))
    //         {
    //             dateofmanufacture[0] = 15;
    //             dateofmanufacture[1] = 12;
    //             dateofmanufacture[2] = 31;
    //             dateofmanufacture[3] = 23;
    //             dateofmanufacture[4] = 59;
    //         }

    //         /* ���ж�ȡ�ĵ�ǰʱ��ͼ�¼���������ڵıȽ� */
    //         /* ��ǰʱ����� ���� ���������е��� */
    //         if (i2c_time_code[6] >= dateofmanufacture[0])
    //         {
    //             /* ������� */
    //             dateofmanufacture[0] = i2c_time_code[6] - dateofmanufacture[0];
    //             /* ��� �Ѿ��������� �� 2020 - 2015 = 5 */
    //             if (dateofmanufacture[0] == SENSOR_LIFE)
    //             {
    //                 /* ��ǰʱ����� ���� ���������е��� 2020.6 > 2015.5 */
    //                 if (i2c_time_code[5] > dateofmanufacture[1])
    //                 {
    //                     /* ��ʹ������ ��1 ��Ϊ6 */
    //                     dateofmanufacture[0]++;
    //                 }
    //                 /* ��ǰʱ����� ���� ���������е��� 2020.5 = 2015.5 */
    //                 if (i2c_time_code[5] == dateofmanufacture[1])
    //                 {
    //                     /* ��ǰʱ����� ���� ���������е��� 2020.5.6 > 2015.5.5 */
    //                     if (i2c_time_code[3] > dateofmanufacture[2])
    //                     {
    //                         /* ��ʹ������ ��1 ��Ϊ6 */
    //                         dateofmanufacture[0]++;
    //                     }
    //                     /* ��ǰʱ����� ���� ���������е��� 2020.5.5 = 2015.5.5 */
    //                     if (i2c_time_code[3] == dateofmanufacture[2])
    //                     {
    //                         /* �Դ����ƱȽ�ʱ */
    //                         if (i2c_time_code[2] > dateofmanufacture[3])
    //                         {
    //                             /* ��ʹ������ ��1 ��Ϊ6 */
    //                             dateofmanufacture[0]++;
    //                         }
    //                         /* ʱ��� */
    //                         if (i2c_time_code[2] == dateofmanufacture[3])
    //                         {
    //                             /* �Դ����ƱȽϷ� */
    //                             if (i2c_time_code[1] > dateofmanufacture[4])
    //                             {
    //                                 /* ��ʹ������ ��1 ��Ϊ6 */
    //                                 dateofmanufacture[0]++;
    //                             }
    //                         }
    //                     }
    //                 }
    //             }

    //             /* ʹ��������ʱ 5��ʱ�䵽 */
    //             if (dateofmanufacture[0] > SENSOR_LIFE)
    //             {
    //                 /* �����������ƿ� */
    //                 LED_LIFE_ON;
    //                 /* XXX */
    //                 if (Life_Write_Flag == 0)
    //                 {
    //                     Life_Write_Flag = 1;

    //                     /* XXX ��FLASH�ж�ȡ �������ڴ洢��ַ �ĵ�һ���ֽ� 1����ʧЧ��0����δʧЧ */
    //                     ReadData(dateofmanufacture, RECORD_FIRST_ADDRESS[LIFE_RECORD], 1);
    //                     /* XXX ���ֽ�Ϊ0 ���� ����Ӧ��ĵ��ڼ�¼1�� */
    //                     if (dateofmanufacture[0] == 0 || (dateofmanufacture[0] > RecordLEN[LIFE_RECORD]))
    //                     {
    //                         /* Brown-Out Detector ��Դ��ѹ��� */
    //                         check_BOD();
    //                         /* д�������ڼ�¼ */
    //                         WriteRecordData(LIFE_RECORD);
    //                     }
    //                 }
    //             }
    //             else
    //             {
    //                 if (Life_Check_flag == 1)
    //                 {
    //                     /* �����������ƹ� */
    //                     LED_LIFE_OFF;
    //                 }
    //             }
    //         }
    //     }

    //     /* ��ȡ���� P0.7 ��ֵ ����Լ칦���Ƿ񴥷� */
    //     Test_key = P07 & 0x01;
    //     /* ���Լ����� */
    //     if (Test_key == 0x00)
    //     {
    //         /* ��ʱȥ�� */
    //         Delay1ms(30);
    //         /* ȷ�ϲ��Լ����� */
    //         if (Test_key == 0x00)
    //         {
    //             /* XXX */
    //             if (!key_long_press_flag)
    //             {
    //                 timer2_key_long_press_count = 0;
    //                 key_long_press_flag = 1;
    //             }
    //             /* �������6 ͨ��ʵ�ʲ��Գ���ʱʱ��Ϊ11s ����涨���Լ����º� ��7~30s�� Ӧ�����̵��� �� */
    //             if (timer2_key_long_press_count >= 6)
    //             {
    //                 /* XXX �� */
    //                 if (!VALVE_flag)
    //                 {
    //                     VALVE_flag = 1;

    //                     /* ��ŷ��� */
    //                     VALVE_ON;
    //                     Delay1ms(30);

    //                     /* ��ŷ��� */
    //                     VALVE_OFF;
    //                     Delay1ms(30);
    //                 }
    //                 /* �̵����� */
    //                 DELAY_ON;
    //                 timer2_key_long_press_count = 8;
    //             }
    //             Light_Flash();
    //         }
    //     }
    //     /* ���Լ�δ���� */
    //     else
    //     {
    //         /* XXX */
    //         if (key_long_press_flag)
    //         {
    //             /* ��ʱȥ�� */
    //             Delay1ms(30);
    //             /* ȷ�ϲ��Լ�δ���� */
    //             if (Test_key != 0x00)
    //             {
    //                 key_long_press_flag = 0;
    //                 VALVE_flag = 0;
    //                 if (!sensor_preheat)
    //                 {   
    //                     /* �̵����� */
    //                     DELAY_OFF;
    //                 }
    //             }
    //         }
    //     }

    //     /* �õ�ADC�������˲�֮���ֵ ���� ch4_3500 �ı�����ֵ ����״̬���� */
    //     if (CH4_Adc_Valu >= ch4_3500)
    //     {
    //         /* Brown-Out Detector ��Դ��ѹ��� */
    //         check_BOD();
    //         /* Ԥ����� */
    //         if (sensor_preheat)
    //         {
    //             /* ����Ϊ����״̬ */
    //             STATUS1_ALARM;
    //             /* ����P0.0Ϊ�������ģʽ */
    //             P0M1 = 0x00;
    //             P0M2 = 0x01;

    //             /* XXX */
    //             if (Delay1s_flag == 1)
    //             {
    //                 Delay1s_flag = 0;
    //                 /* XXX */
    //                 Alarm_Count++;

    //                 /* ������������ 3 */
    //                 if (Alarm_Count == 3)
    //                 {
    //                     Alarm_flag = 1;
    //                 }

    //                 /* ������������ 5 */
    //                 if (Alarm_Count > 5)
    //                 {
    //                     Alarm_Count = 5;
    //                     /* �̵����� */
    //                     DELAY_ON;
    //                     /* XXX Alarm_Value Ϊ8�ı��� */
    //                     if (!(Alarm_Value % 8))
    //                     {
    //                         /* ��ŷ��� */
    //                         VALVE_ON;
    //                         Delay1ms(30);

    //                         /* ��ŷ��� */
    //                         VALVE_OFF;
    //                         Delay1ms(30);
    //                     }
    //                     Alarm_Value++;
    //                 }
    //             }

    //             /* ������������ 3 Alarm_flag = 1 */
    //             if (Alarm_flag == 1)
    //             {
    //                 {
    //                     Alarm_flag = 0;
    //                     Alarm_Back_flag = 1;
    //                     /* Brown-Out Detector ��Դ��ѹ��� */
    //                     check_BOD();
    //                     /* ��FLASH�� д������¼ */
    //                     WriteRecordData(ALARM_RECORD);
    //                 }
    //             }

    //             /* ������������3���� */
    //             if (Alarm_Count >= 3)
    //             {
    //                 /* �������������ź� */
    //                 Light_Alarm_Flash();
    //             }
    //         }
    //     }
    //     /* �õ�ADC�������˲�֮���ֵ С�� Short_Fault_L �Ĺ�����ֵ ����״̬���� */
    //     else if (CH4_Adc_Valu <= Short_Fault_L)
    //     {
    //         /* Brown-Out Detector ��Դ��ѹ��� */
    //         check_BOD();

    //         /* Ԥ����� */
    //         if (sensor_preheat)
    //         {
    //             /* ����ϵͳ״̬Ϊ����״̬ */
    //             STATUS1_FAULT;

    //             Alarm_Count = 0;
    //             Alarm_Value = 0;

    //             /* �����ƹ� */
    //             LED_ALARM_OFF;
    //             /* �̵����� */
    //             DELAY_OFF;

    //             /* XXX */
    //             Fault_Count++;
    //             if (Fault_Count == 1)
    //             {
    //                 Fault_flag = 1;
    //             }
    //             if (Fault_Count > 2)
    //             {
    //                 Fault_Count = 2;
    //             }

    //             /* ���������ź� */
    //             Fault_Flash();

    //             /* XXX */
    //             if (Fault_flag == 1)
    //             {
    //                 Fault_flag = 0;
    //                 Fault_Back_flag = 1;
    //                 /* Brown-Out Detector ��Դ��ѹ��� */
    //                 check_BOD();
    //                 /* ��FLASH�� д���ϼ�¼ */
    //                 WriteRecordData(FAULT_RECORD);
    //             }
    //         }
    //     }
    //     /* �õ�ADC�������˲�֮���ֵ ���� Short_Fault_L �Ĺ�����ֵ С�� ch4_3500 �ı�����ֵ ����״̬���� */
    //     else
    //     {
    //         if (Delay_Time_Flag)
    //         {
    //             STATUS1_NOMAL;
    //         }
    //         /* Brown-Out Detector ��Դ��ѹ��� */
    //         check_BOD();

    //         /* XXX */
    //         Alarm_flag = 0;
    //         Alarm_Count = 0;
    //         Alarm_Value = 0;
    //         Fault_flag = 0;
    //         Fault_Count = 0;

    //         /* XXX */
    //         if (Alarm_Back_flag == 1)
    //         {
    //             Alarm_Back_flag = 0;
    //             /* Brown-Out Detector ��Դ��ѹ��� */
    //             check_BOD();
    //             /* ��FLASH�� д�����ָ���¼ */
    //             WriteRecordData(ALARM_BACK_RECORD);
    //         }

    //         /* XXX */
    //         if (Fault_Back_flag == 1)
    //         {
    //             Fault_Back_flag = 0;
    //             Fault_flag = 0;

    //             /* Brown-Out Detector ��Դ��ѹ��� */
    //             check_BOD();
    //             /* ��FLASH�� д���ϻָ���¼ */
    //             WriteRecordData(FAULT_BACK_RECORD);
    //         }

    //         /* �����ƹ� */
    //         LED_ALARM_OFF;
    //         /* ���ϵƹ� */
    //         LED_FAULT_OFF;

    //         /* ������1�� */
    //         SOUND1_OFF;
    //         /* ������2�� */
    //         SOUND2_OFF;
    //         /* ��ŷ��� */
    //         VALVE_OFF;

    //         /* XXX */
    //         if (!VALVE_flag)
    //         {
    //             /* �̵����� */
    //             DELAY_OFF;
    //         }
    //     }
    // }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

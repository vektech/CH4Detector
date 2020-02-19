/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      flash.c
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
#include "flash.h"
#include "device.h"
#include "delay.h"
#include "i2c.h"
#include "uart.h"
#include "utlities.h"

#include <intrins.h>

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define ERASE_ALL_DATA 0x22
#define FLASH_READ_DATA 0x00
#define BYTE_PROGRAM_DATA 0x21

#define ERASE_ALL_CONFIG 0xE2
#define FLASH_READ_CONFIG 0xC0
#define BYTE_PROGRAM_CONFIG 0xE1

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

/* ÿһ���¼���ܼ�¼�� */
code uint8_t max_of_each_record[9] = {0xff,200,200,100,100,50,50,1,0xff};
/* �������򳤶� */
code uint8_t length_of_data_zone[9] = {0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x06};

code uint16_t RECORD_FIRST_ADDRESS[11] = 
{
    0xffff,                     /*!< 0.��ѯ�����¼���� */
    ALARM_RECORD_ADDR,          /*!< 1.������¼ 200 */
    ALARM_RECOVERY_RECORD_ADDR, /*!< 2.�����ָ���¼ 200 */
    FAULT_RECORD_ADDR,          /*!< 3.���ϼ�¼ 100 */
    FAULT_RECOVERY_RECORD_ADDR, /*!< 4.���ϻָ���¼ 100 */
    POWER_DOWN_RECORD_ADDR,     /*!< 5.�����¼ 50 */
    POWER_ON_RECORD_ADDR,       /*!< 6.�ϵ��¼ 50 */
    SENSOR_EXPIRED_RECORD_ADDR, /*!< 7.������ʧЧ��¼ 1 */
    0xffff,                     /*!< 8.�ڲ���ʱ����ǰʱ�� */
    TEMP_PAGE_ADDR,             /*!< 9.���ڱ��ݴ洢��ʱ���ݵ�FLASH��ַ */
    DEVICE_INFO_ADDR            /*!< 10.�豸��Ϣ�洢��ַ */
};

code uint16_t RECORD_LAST_ADDRESS[11] = 
{
    0xffff,                               /*!< 0.��ѯ�����¼���� */
    ALARM_RECORD_ADDR          + 200 * 4, /*!< 1.������¼ 200 */
    ALARM_RECOVERY_RECORD_ADDR + 200 * 4, /*!< 2.�����ָ���¼ 200 */
    FAULT_RECORD_ADDR          + 100 * 4, /*!< 3.���ϼ�¼ 100 */
    FAULT_RECOVERY_RECORD_ADDR + 100 * 4, /*!< 4.���ϻָ���¼ 100 */
    POWER_DOWN_RECORD_ADDR     + 50 * 4,  /*!< 5.�����¼ 50 */
    POWER_ON_RECORD_ADDR       + 50 * 4,  /*!< 6.�ϵ��¼ 50 */
    SENSOR_EXPIRED_RECORD_ADDR,           /*!< 7.������ʧЧ��¼ 1 */
    0xffff,                               /*!< 8.�ڲ���ʱ����ǰʱ�� */
    TEMP_PAGE_ADDR,                       /*!< 9.���ڱ��ݴ洢��ʱ���ݵ�FLASH��ַ */
    DEVICE_INFO_ADDR                      /*!< 10.�豸��Ϣ�洢��ַ */
};

/* �洢ѹ��֮���ʱ���� */
uint8_t zipped_time[4] = {0};
/* �洢��ѹ֮���ʱ���� */
uint8_t unzipped_time[5] = {0};

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
/* ��ISPģʽ */
void Enable_ISP_Mode(void)
{
    /* Record EA Status */
    EA_save_bit = EA;
    /* �ر�ȫ���ж� */
    EA = 0;

    /* �ؼ�SFR��д���� */
    TA = 0xAA;
    TA = 0x55;
    /* ��ISPEN��1 ����ISP���� 
        - ʹ��ISP���ܽ����ڲ���22.1184MHz RC ����Ϊʱ��
        - ��ISPENӦ����ISP����������һ��ָ�� ��������ֹͣ�ڲ�RC�Լ��ٹ���
        */
    CHPCON |= 0x01;
    /* Resume EA */
    EA = EA_save_bit;
}

/* �ر�ISPģʽ */
void Disable_ISP_Mode(void)
{
    /* Disable ISP */
    /* Record EA Status */
    EA_save_bit = EA;
    /* �ر�ȫ���ж� */
    EA = 0;

    /* ISP ���ܵĹؼ�SFR��д���� */
    TA = 0xAA;
    TA = 0x55;
    /* ��ISPEN��0 �ر�ISP���� 
        - ʹ��ISP���ܽ����ڲ���22.1184MHz RC ����Ϊʱ��
        - ��ISPENӦ����ISP����������һ��ָ�� ��������ֹͣ�ڲ�RC�Լ��ٹ���
        */
    CHPCON &= 0xFE;
    /* Resume EA */
    EA = EA_save_bit;
}

/* ����ISP���� ��ִ��ISPָ�� */
void Trigger_ISP(void)
{
    /* Record EA Status */
    EA_save_bit = EA;
    /* �ر�ȫ���ж� */
    EA = 0;

    /* ISP ���ܵĹؼ�SFR��д���� */
    TA = 0xAA;
    TA = 0x55;
    /* ISPGO -> ISPTRG[0] ��λ ִ��ISP
        - ���ø�λΪ1��ʼִ��ISP ��ָ��� CPU���ֳ��������PC ISPӲ���Զ�������Ƹù���
        - ISP��ɺ� �������������ִ�� ISPGOλ�Զ����� ����Ϊ0
        */
    ISPTRG |= 0x01;
    /* Resume EA */
    EA = EA_save_bit;
}

/* Flash ҳ���� */
void flash_erase_page(uint16_t start_addr)
{
    Enable_ISP_Mode();

    /* ISPCN = 0010 0010B ISP���� ѡ�����APROM��������FLASH ����FLASHҳ����
        - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
        - [5] FOEN      = 1B ѡ�� ҳ���� ���� ���; 0B ѡ�� ������;
        - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
        - [3:0] FCTRL   = 0010B ҳ����; 0001B ���; 0000B ������;
        */
    ISPCN = ERASE_ALL_DATA;

    /* ISPAL ISP��ַ���ֽ� */
    ISPAL = LOBYTE(start_addr);
    /* ISPAH ISP��ַ���ֽ�*/
    ISPAH = HIBYTE(start_addr);

    Trigger_ISP();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();

    Disable_ISP_Mode();
}

/* ��Flash��д���� 
    - p          Դ���ݵ�ַ
    - len        Դ���ݳ���
    - start_addr ����д�����ʼ��ַ
    - offset     �������ʼ��ַ��ƫ����
    */
void flash_write_data(uint8_t *p, uint8_t len, uint16_t start_addr, uint8_t offset)
{
    uint8_t j;

    if (!len)
        return;
    if (offset >= 128)
        return;
    /* offset Ϊ��ʼд����ʼ��ַƫ�� ��ע��˴�ӦΪ (offset + len - 1) */
    if ((offset + len - 1) >= 128)
        return;

    /* ���� TEMP_PAGE_ADDR ���ڱ��� */
    flash_erase_page(TEMP_PAGE_ADDR);

    Enable_ISP_Mode();

    /* ����Ҫ���µ�ҳ������ TEMP_PAGE �� */
    for (j = 0; j < 128; j++)
    {
        /* ISPCN = 0000 0000B ISP���� ѡ�����APROM��������FLASH ����FLASH������
            - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
            - [5] FOEN      = 0B ѡ�� ������; 1B ѡ�� ҳ���� ���� ���;
            - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
            - [3:0] FCTRL   = 0000B ������; 0010B ҳ����; 0001B ���; 
            */
        ISPCN = FLASH_READ_DATA;
        /* ISPAL ISP��ַ���ֽ� */
        ISPAL = LOBYTE(start_addr + j);
        /* ISPAH ISP��ַ���ֽ� */
        ISPAH = HIBYTE(start_addr + j);

        Trigger_ISP();        

        /* ISPCN = 0010 0001B ISP���� ѡ����� APROM��������FLASH ����FLASH���
            - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
            - [5] FOEN      = 1B ѡ�� ҳ���� ���� ���; 0B ѡ�� ������;
            - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
            - [3:0] FCTRL   = 0001B ���; 0000B ������; 0010B ҳ����; 
            */
        ISPCN = BYTE_PROGRAM_DATA;
        /* ISPAL ISP��ַ���ֽ� */
        ISPAL = LOBYTE(TEMP_PAGE_ADDR + j);
        /* ISPAH ISP��ַ���ֽ� */
        ISPAH = HIBYTE(TEMP_PAGE_ADDR + j);
        
        Trigger_ISP();
    }

    Disable_ISP_Mode();

    /* ������Ҫ���µ�ҳ */
    flash_erase_page(start_addr);

    Enable_ISP_Mode();

    for (j = 0; j < 128; j++)
    {
        /* 1. ��Ҫ���µĵ�ַ���� �����ַƫ��֮�� ������������� ����Ϊlen */
        if (j >= offset && (j < (len + offset)))
        {
            /* ISPCN = 0010 0001B ISP���� ѡ����� APROM��������FLASH ����FLASH���
                - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
                - [5] FOEN      = 1B ѡ�� ҳ���� ���� ���; 0B ѡ�� ������;
                - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
                - [3:0] FCTRL   = 0001B ���; 0000B ������; 0010B ҳ����; 
                */
            ISPCN = BYTE_PROGRAM_DATA;
            /* ISPAL ISP��ַ���ֽ� */
            ISPAL = LOBYTE(start_addr + j);
            /* ISPAH ISP��ַ���ֽ� */
            ISPAH = HIBYTE(start_addr + j);
            /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
                - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
                - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
            */
            ISPFD = *p++;
        }
        else
        {
            /* 2. ��������Ҫ���µĵ�ַ���� ��ȡ�ݴ�ҳ�е����� */
            /* ISPCN = 0000 0000B ISP���� ѡ�����APROM��������FLASH ����FLASH������
                - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
                - [5] FOEN      = 0B ѡ�� ������; 1B ѡ�� ҳ���� ���� ���;
                - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
                - [3:0] FCTRL   = 0000B ������; 0010B ҳ����; 0001B ���; 
                */
            ISPCN = FLASH_READ_DATA;
            /* ISPAL ISP��ַ���ֽ� */
            ISPAL = LOBYTE(TEMP_PAGE_ADDR + j);
            /* ISPAH ISP��ַ���ֽ� */
            ISPAH = HIBYTE(TEMP_PAGE_ADDR + j);

            Trigger_ISP();

            /* 3. ��������Ҫ���µĵ�ַ���� ����ȡ���ݴ�ҳ�е����� д����Ҫ���µ�ҳ�� */
            /* ISPCN = 0010 0001B ISP���� ѡ����� APROM��������FLASH ����FLASH���
                - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
                - [5] FOEN      = 1B ѡ�� ҳ���� ���� ���; 0B ѡ�� ������;
                - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
                - [3:0] FCTRL   = 0001B ���; 0000B ������; 0010B ҳ����; 
                */
            ISPCN = BYTE_PROGRAM_DATA;
            /* ISPAL ISP��ַ���ֽ� */
            ISPAL = LOBYTE(start_addr + j);
            /* ISPAH ISP��ַ���ֽ� */
            ISPAH = HIBYTE(start_addr + j);
        }

        Trigger_ISP();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
    }
    
    Disable_ISP_Mode();
}

/* ��Flash�ж�ȡ����
    - p          ����������ݵĵ�ַ
    - start_addr ��ȡ���ݵ���ʼ��ַ
    - len        �����ݵĳ���
    */
void flash_read_data(uint8_t *p, uint16_t start_addr, uint8_t len)
{
    if (!len)
        return;

    Enable_ISP_Mode();

    /* ISPCN = 0000 0000B ISP���� ѡ�����APROM��������FLASH ����FLASH������
        - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
        - [5] FOEN      = 0B ѡ�� ������; 1B ѡ�� ҳ���� ���� ���;
        - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
        - [3:0] FCTRL   = 0000B ������; 0010B ҳ����; 0001B ���; 
        */
    ISPCN = FLASH_READ_DATA;
    /* ISPAL ISP��ַ���ֽ� */
    ISPAL = LOBYTE(start_addr);
    /* ISPAH ISP��ַ���ֽ� */
    ISPAH = HIBYTE(start_addr);

    while (len--)
    {
        Trigger_ISP();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();

        /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
            - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
            - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
        */
        *p++ = ISPFD;
        /* ISPAL ISP��ַ���ֽڼ�1 */
        ISPAL++;
    }

    Disable_ISP_Mode();
}

/* ѹ����ǰʱ�� ��ÿ�δ��¼ʱ��Ҫ��ȡ��ǰ��ʱ�� */
void zip_current_time(uint8_t *p)
{
    uint8_t i;

    /* ��ʱ�洢ѹ��֮���ʱ�� */
    uint8_t temp_zipped_time[4] = {0};
    /* ��ʱ�洢��ѹ֮���ʱ�� */
    uint8_t temp_unzipped_time[5] = {0};

    /* ����һ�ζ�ʱ�亯�� Լ��ʱ500uS */
    i2c_get_time();

    /* second */
    /* i2c_time_code[0]; */
    /* minute */
    temp_unzipped_time[4] = i2c_time_code[1];
    /* hour */
    temp_unzipped_time[3] = i2c_time_code[2];
    /* day */
    temp_unzipped_time[2] = i2c_time_code[3];
    /* week */
    /* i2c_time_code[4]; */
    /* month */
    temp_unzipped_time[1] = i2c_time_code[5];
    /* year */
    temp_unzipped_time[0] = i2c_time_code[6];

    /* ͨ����λѹ�� */

    /* flag & year 0000 00|11 */
    temp_zipped_time[0] = (temp_unzipped_time[0] >> 4);
    /* year & month 1111|1111 */
    temp_zipped_time[1] = temp_unzipped_time[1];
    temp_zipped_time[1] |= (temp_unzipped_time[0] << 4);
    /* day & hour 1111 1|111 */
    temp_zipped_time[2] = temp_unzipped_time[2];
    temp_zipped_time[2] <<= 3;
    temp_zipped_time[2] |= (temp_unzipped_time[3] >> 2);
    /* hour & minute 11|11 1111 */
    temp_zipped_time[3] = temp_unzipped_time[4];
    temp_zipped_time[3] |= (temp_unzipped_time[3] << 6);

    /* ������Ŀ�������� */
    for ( i = 0; i < 4; i++)
    {
        *(p + i) = temp_zipped_time[i];
    }    
}

/* ��ѹ��ʱ�� p �洢ѹ������ʱ�� q �洢��ѹ���ʱ�� */
void unzip_time(uint8_t *p, uint8_t *q)
{
    uint8_t i;

    /* ��ʱ�洢ѹ��֮���ʱ�� */
    uint8_t temp_zipped_time[4] = {0};
    /* ��ʱ�洢��ѹ֮���ʱ�� */
    uint8_t temp_unzipped_time[5] = {0};

    /* ������Ŀ�������� */
    for ( i = 0; i < 4; i++)
    {
        temp_zipped_time[i] = *(p + i);
    }

    /* flag & year 0000 00|11 */
    temp_unzipped_time[0] = ((temp_zipped_time[0] << 4) | (temp_zipped_time[1] >> 4));
    /* year & month 1111|1111 */
    temp_unzipped_time[1] = (temp_zipped_time[1] & 0x0F);
    /* day & hour 1111 1|111 */
    temp_unzipped_time[2] = ((temp_zipped_time[2] & 0xF8) >> 3);
    /* hour & minute 11|11 1111 */
    temp_unzipped_time[3] = (((temp_zipped_time[2] & 0x07) << 2) | ((temp_zipped_time[3] & 0xC0) >> 6));
    temp_unzipped_time[4] = (temp_zipped_time[3] & 0x3F);

    /* ������Ŀ�������� */
    for ( i = 0; i < 5; i++)
    {
        *(q + i) = temp_unzipped_time[i];
    }
}

void flash_write_record(uint8_t record_type)
{
    uint16_t page_addr;
    uint8_t page_offset;
    uint8_t record_first_byte;

    uint16_t record_total;

    uint16_t start_addr;
    uint16_t record_addr;
    uint16_t newest_addr;

    uint8_t temp_record_total[2];
    uint8_t temp_newest_addr[2];

    /* �ҵ������ͼ�¼���׵�ַ */
    start_addr = RECORD_FIRST_ADDRESS[record_type];
#ifdef _DEBUG_
    uart_send((uint8_t)(start_addr >> 8));
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)start_addr);
    delay_1ms_without_BOD(10);
#endif

    /* ��ȡ��¼���� */
    flash_read_data(temp_record_total, start_addr, 2);
    /* ��¼�����ĸ�λ */
    record_total = temp_record_total[0];
    /* ��¼�����ĵ�λ */
    record_total = (record_total << 8) + temp_record_total[1];
#ifdef _DEBUG_
    uart_send(temp_record_total[0]);
    delay_1ms_without_BOD(10);
    uart_send(temp_record_total[1]);
    delay_1ms_without_BOD(10);
#endif

    /* ��ȡ���¼�¼�ĵ�ַ */
    flash_read_data(temp_newest_addr, start_addr + 2, 2);
    /* ���¼�¼��ַ�ĸ�λ */
    newest_addr = temp_newest_addr[0];
    /* ���¼�¼��ַ�ĵ�λ */
    newest_addr = (newest_addr << 8) + temp_newest_addr[1];
#ifdef _DEBUG_
    uart_send(temp_newest_addr[0]);
    delay_1ms_without_BOD(10);
    uart_send(temp_newest_addr[1]);
    delay_1ms_without_BOD(10);
#endif

    /* �ü�¼����δ�洢����¼ */
    if ((record_total == 0xFFFF) && (newest_addr == 0xFFFF))
    {
        /* ���¼�¼���� */
        record_total = 0x0001;

        /* ����д�¼�¼�ĵ�ַ */
        record_addr = start_addr + 4;
    }
    /* ���Ѵ洢�ļ�¼ */
    else
    {
        /* �������涨������¼���� */
        if (record_total < max_of_each_record[record_type])
        {
            record_total++;
        }

        /* ��ȡǰ���µļ�¼�����ֽ� */
        flash_read_data(&record_first_byte, newest_addr, 1);
        /* ����ǰ���µļ�¼��־��Ϊ�ɵļ�¼��־ 0000 00 */
        record_first_byte &= 0x03;
        /* �����±�־��ļ�¼���ֽ�д�� */
        page_addr = start_addr + ((newest_addr - start_addr) / 128) * 128;
        page_offset = (newest_addr - start_addr) % 128;
        flash_write_data(&record_first_byte, 1, page_addr, page_offset);

        /* ������¼�¼�ĵ�ַλ�ڼ�¼�����ĩβ */
        if (newest_addr == RECORD_LAST_ADDRESS[record_type])
        {
            /* ����д�¼�¼�ĵ�ַ ������¼�����ײ� +4 byte ����ʼ�ĵ�һ����¼ */
            record_addr = start_addr + 4;
        }
        else
        {
            /* ����д�¼�¼�ĵ�ַ */
            record_addr = newest_addr + 4;
        }
    }

    /* �������¼�¼�ĵ�ַ ҳ��ַ��ҳ��ƫ�� */
    newest_addr = record_addr;
    page_addr = start_addr + ((newest_addr - start_addr) / 128) * 128;
    page_offset = (newest_addr - start_addr) % 128;
#ifdef _DEBUG_
    uart_send((uint8_t)(newest_addr >> 8));
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)newest_addr);
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)(page_addr >> 8));
    delay_1ms_without_BOD(10);
    uart_send((uint8_t)page_addr);
    delay_1ms_without_BOD(10);
    uart_send(page_offset);
    delay_1ms_without_BOD(10);
#endif

    /* ѹ����ǰʱ���� zipped_time */
    zip_current_time(zipped_time);
    /* �����������ݱ�־ 1000 00 */
    zipped_time[0] &= 0x03;
    zipped_time[0] |= 0x80;
#ifdef _DEBUG_
    uart_send(zipped_time[0]);
    delay_1ms_without_BOD(10);
    uart_send(zipped_time[1]);
    delay_1ms_without_BOD(10);
    uart_send(zipped_time[2]);
    delay_1ms_without_BOD(10);
    uart_send(zipped_time[3]);
    delay_1ms_without_BOD(10);
#endif

    /* д��¼ ��Ҫ����ҳ�׵�ַ */
    flash_write_data(zipped_time, 4, page_addr, page_offset);

    /* д��¼���� */
    temp_record_total[0] = (uint8_t)((record_total >> 8) & 0xFF);
    temp_record_total[1] = (uint8_t)(record_total & 0xFF);
    flash_write_data(temp_record_total, 2, start_addr, 0);
    /* д���¼�¼��ַ */
    temp_newest_addr[0] = (uint8_t)((newest_addr >> 8) & 0xFF);
    temp_newest_addr[1] = (uint8_t)(newest_addr & 0xFF);
    flash_write_data(temp_newest_addr, 2, start_addr, 2);
}

void flash_read_record(uint8_t record_type, uint8_t record_number)
{
    uint8_t i;

    uint16_t record_total;
    uint16_t storage_length;

    uint16_t start_addr;
    uint16_t record_addr;
    uint16_t newest_addr;

    uint8_t temp_record_total[2];
    uint8_t temp_newest_addr[2];

    switch (record_type)
    {
        /* ��ѯ�豸�����¼������ */
        case 0x00:
        {
            /* SENSOR_EXPIRED_RECORD �������� */
            for (i = ALARM_RECORD; i < SENSOR_EXPIRED_RECORD; i++)
            {
                /* �ҵ������ͼ�¼���׵�ַ */
                start_addr = RECORD_FIRST_ADDRESS[i];
#ifdef _DEBUG_
                uart_send((uint8_t)(start_addr >> 8));
                delay_1ms(10);
                uart_send((uint8_t)start_addr);
                delay_1ms(10);
#endif
                /* ��ȡ��¼���� �˴����õ�8λ temp_record_total[1] ���� */
                flash_read_data(temp_record_total, start_addr, 2);
#ifdef _DEBUG_
                uart_send(temp_record_total[0]);
                delay_1ms(10);
                uart_send(temp_record_total[1]);
                delay_1ms(10);
#endif                

                /* �ü�¼����δ�洢����¼ �˴����õ�8λ temp_record_total[1] ���� */
                if (temp_record_total[1] == 0xFF)
                {
                    uart_buffer[i + 3] = 0x00;
                }
                else
                {
                    uart_buffer[i + 3] = temp_record_total[1];
                }
#ifdef _DEBUG_                
                uart_send(uart_buffer[i + 3]);
                delay_1ms(10);
#endif                
            }

            /* ��ȡ���崫����ʧЧ������ uart_buffer[10] */
            flash_read_data(&uart_buffer[10], SENSOR_EXPIRED_RECORD_ADDR, 1);

            /* ����Ƿ����崫�����Ѿ����� */
            if (uart_buffer[10] != 0x01)
            {
                /* δ���ڵ�����·��� 0x00 */
                uart_buffer[10] = 0x00;
            }

            /* ��ʼ�� */
            uart_buffer[0] = 0xaa;
            /* ��¼��� */
            uart_buffer[1] = uart_buffer[1];
            /* ��¼���� */
            uart_buffer[2] = uart_buffer[2];
            /* �����򳤶� �����һ���豸ʱ�������Ϊ 0x07 */
            uart_buffer[3] = 0x07;

            /* uart_buffer[4] �� uart_buffer[11] �Ѵ��� */

            /* У��� */
            uart_buffer[11] = get_crc(uart_buffer, (0x07 + 0x06));
            /* ������ */
            uart_buffer[12] = 0x55;

            for (i = 0; i < (0x07 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* ���������ʱ���� �ؼ����� ������ */
                delay_1ms(10);
            }

            break;
        }
        /* ��ѯ���崫����ʧЧ��¼ */
        case 0x07:
        {
            /* ��ʼ�� */
            uart_buffer[0] = 0xaa;
            /* ��¼��� */
            uart_buffer[1] = uart_buffer[1];
            /* ��¼���� */
            uart_buffer[2] = uart_buffer[2];
            /* �����򳤶� �����һ���豸ʱ�������Ϊ 0x07 */
            uart_buffer[3] = 0x07;

            /* ��ȡ���崫����ʧЧ���� ��־[5] ��[6] ��[7] ��[8] ʱ[9] ��[10] */
            flash_read_data(&uart_buffer[5], SENSOR_EXPIRED_RECORD_ADDR, 6);
            /* ���崫�����Ѿ�ʧЧ */
            if (uart_buffer[5] == 0x01)
            {
                /* ʹ�� uart_buffer[11] ��ʱ�洢�� ����ת��Ϊ�����ȫ��ʽ ���õ�����뷽ʽ */
                uart_buffer[11] = uart_buffer[6];

                /* n1 ���崫����ʧЧ��־ */
                uart_buffer[4] = uart_buffer[5];
                /* n2 ����ֽ� */
                uart_buffer[5] = (2000 + uart_buffer[11]) / 256;
                /* n3 ����ֽ� */
                uart_buffer[6] = (2000 + uart_buffer[11]) % 256;
                /* n4 �� */
                uart_buffer[7] = uart_buffer[7];
                /* n5 �� */
                uart_buffer[8] = uart_buffer[8];
                /* n6 ʱ */
                uart_buffer[9] = uart_buffer[9];
                /* n7 �� */
                uart_buffer[10] = uart_buffer[10];
            }
            /* ���崫����δʧЧ */
            else
            {
                for (i = 4; i < 11; i++)
                {
                    uart_buffer[i] = 0x00;
                }
                
            }

            /* У��� */
            uart_buffer[11] = get_crc(uart_buffer, (0x07 + 0x06));
            /* ������ */
            uart_buffer[12] = 0x55;

            for (i = 0; i < (0x07 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* ���������ʱ���� �ؼ����� ������ */
                delay_1ms(10);
            }
            break;
        }
        /* ��ѯ�豸��ǰʱ�� */
        case 0x08:
        {
            /* ��ȡ��ǰ���豸ʱ�� */
            i2c_get_time();

            /* ��ʼ�� */
            uart_buffer[0] = 0xaa;
            /* ��¼��� */
            uart_buffer[1] = uart_buffer[1];
            /* ��¼���� */
            uart_buffer[2] = uart_buffer[2];
            /* �����򳤶� 0x06 */
            uart_buffer[3] = 0x06;

            /* n1 ����ֽ� */
            uart_buffer[4] = (2000 + i2c_time_code[6]) / 256;
            /* n2 ����ֽ� */
            uart_buffer[5] = (2000 + i2c_time_code[6]) % 256;
            /* n3 �� */
            uart_buffer[6] = i2c_time_code[5];
            /* n4 �� */
            uart_buffer[7] = i2c_time_code[3];
            /* n5 ʱ */
            uart_buffer[8] = i2c_time_code[2];
            /* n6 �� */
            uart_buffer[9] = i2c_time_code[1];
            /* У��� */
            uart_buffer[10] = get_crc(uart_buffer, (0x06 + 0x06));
            /* ������ */
            uart_buffer[11] = 0x55;

            for (i = 0; i < (0x06 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* ���������ʱ���� �ؼ����� ������ */
                delay_1ms(10);
            }
            break;
        }
        /* ���������ѯ */
        default:
        {
            /* �ҵ������ͼ�¼���׵�ַ */
            start_addr = RECORD_FIRST_ADDRESS[record_type];

            /* ��ȡ��¼���� */
            flash_read_data(temp_record_total, start_addr, 2);
            /* ��¼�����ĸ�λ */
            record_total = temp_record_total[0];
            /* ��¼�����ĵ�λ */
            record_total = (record_total << 8) + temp_record_total[1];
#ifdef _DEBUG_            
            uart_send(temp_record_total[0]);
            delay_1ms(10);
            uart_send(temp_record_total[1]);
            delay_1ms(10);
#endif

            /* ��ȡ���¼�¼�ĵ�ַ */
            flash_read_data(temp_newest_addr, start_addr + 2, 2);
            /* ���¼�¼��ַ�ĸ�λ */
            newest_addr = temp_newest_addr[0];
            /* ���¼�¼��ַ�ĵ�λ */
            newest_addr = (newest_addr << 8) + temp_newest_addr[1];
#ifdef _DEBUG_
            uart_send(temp_newest_addr[0]);
            delay_1ms(10);
            uart_send(temp_newest_addr[1]);
            delay_1ms(10);
#endif

            /* �ü�¼����δ�洢����¼ �� �����ѯ�ļ�¼�Ŵ�������ļ�¼���� */
            if (((record_total == 0xFFFF) && (newest_addr == 0xFFFF)) || (record_number > temp_record_total[1]))
            {
                /* Option 1: all data set to 0x00; Option 2: just return */
                for (i = 4; i < 11; i++)
                {
                    uart_buffer[i] = 0x00;
                }
            }
            else
            {
                /* ת����¼��� ���Խ���¼Խ�� ���ԽС��¼Խ�� 01�ż�¼Ϊ��ɼ�¼ */
                record_number = temp_record_total[1] + 1 - record_number;
                /* ����Ҫ��ȡ�ļ�¼��ַ */
                storage_length = max_of_each_record[record_type] * 4;
                /* ZZZ �˼�������в�ͬ�ļ�����ʽ */
                record_addr = (start_addr + 4) + ((storage_length + newest_addr - (record_number - 1) * 4 - (start_addr + 4)) % storage_length);
#ifdef _DEBUG_
                uart_send((uint8_t)((start_addr + 4) >> 8));
                delay_1ms(10);
                uart_send((uint8_t)(start_addr + 4));
                delay_1ms(10);
                uart_send((uint8_t)(storage_length >> 8));
                delay_1ms(10);
                uart_send((uint8_t)storage_length);
                delay_1ms(10);
                uart_send((uint8_t)(newest_addr >> 8));
                delay_1ms(10);
                uart_send((uint8_t)newest_addr);
                delay_1ms(10);
                uart_send((uint8_t)((record_number - 1) >> 8));
                delay_1ms(10);
                uart_send((uint8_t)(record_number - 1));
                delay_1ms(10);
                uart_send((uint8_t)(record_addr >> 8));
                delay_1ms(10);
                uart_send((uint8_t)record_addr);
                delay_1ms(10);
#endif

                flash_read_data(zipped_time, record_addr, 4);
#ifdef _DEBUG_                
                uart_send(zipped_time[0]);
                delay_1ms(10);
                uart_send(zipped_time[1]);
                delay_1ms(10);
                uart_send(zipped_time[2]);
                delay_1ms(10);
                uart_send(zipped_time[3]);
                delay_1ms(10);
#endif

                unzip_time(zipped_time, unzipped_time);

                /* n1 ��¼��� */
                uart_buffer[4] = record_number;
                /* n2 ����ֽ� */
                uart_buffer[5] = (2000 + unzipped_time[0]) / 256;
                /* n3 ����ֽ� */
                uart_buffer[6] = (2000 + unzipped_time[0]) % 256;
                /* n4 �� */
                uart_buffer[7] = unzipped_time[1];
                /* n5 �� */
                uart_buffer[8] = unzipped_time[2];
                /* n6 ʱ */
                uart_buffer[9] = unzipped_time[3];
                /* n7 �� */
                uart_buffer[10] = unzipped_time[4];
            }

            /* ��ʼ�� */
            uart_buffer[0] = 0xaa;
            /* ��¼��� */
            uart_buffer[1] = uart_buffer[1];
            /* ��¼���� */
            uart_buffer[2] = uart_buffer[2];
            /* �����򳤶� �����һ���豸ʱ�������Ϊ 0x07 */
            uart_buffer[3] = 0x07;

            /* uart_buffer[4] �� uart_buffer[11] �Ѵ��� */

            /* У��� */
            uart_buffer[11] = get_crc(uart_buffer, (0x07 + 0x06));
            /* ������ */
            uart_buffer[12] = 0x55;

            for (i = 0; i < (0x07 + 0x06); i++)
            {
                uart_send(uart_buffer[i]);
                /* ���������ʱ���� �ؼ����� ������ */
                delay_1ms(10);
            }
            break;
        }
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

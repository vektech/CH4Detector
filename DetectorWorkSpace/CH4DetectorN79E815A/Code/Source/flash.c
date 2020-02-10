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
    if ((len + offset) >= 128)
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
        delay_1ms(0);

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
        delay_1ms(0);
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
    // i2c_time_code[0];
    /* minute */
    temp_unzipped_time[4] = i2c_time_code[1];
    /* hour */
    temp_unzipped_time[3] = i2c_time_code[2];
    /* day */
    temp_unzipped_time[2] = i2c_time_code[3];
    /* week */
    // i2c_time_code[4];
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

/*******************************************************************************  
* �� �� ��: uint8_t ReadRecordTotal(uint8_t FirstAddr_index)
* ��������: 
* ����˵��: ��
* ���ú���: ��
* ȫ�ֱ���: ��
* ��    ��: FirstAddr_index:�����������RECORD_FIRST_ADDRESS���±꣩��

* ��    ��:  
* �� �� �ߣ� �����                  ���ڣ�2015.08.18
* �� �� �ߣ�                         ���ڣ�
* ��    ����V1.00
*ʹ��ע�����
1����Ȼ���±�ֵ�����ܴ���������±�����������ú���ʲôҲ������
********************************************************************************/
// uint8_t ReadRecordTotal(uint8_t FirstAddr_index)
// {
//     uint8_t i, k1, k2;

//     /* ��ȡ�������ͳ���Χ XXX Ӧ�����������ͼ���8�� ����0 */
//     if (FirstAddr_index <= 0 || (FirstAddr_index >= (sizeof(RECORD_FIRST_ADDRESS) - 1)))
//         return 0;

//     Enable_ISP_Mode();
//     /* ISPCN = 0000 0000B ISP���� ѡ�����APROM��������FLASH ����FLASH������
//         - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
//         - [5] FOEN      = 0B ѡ�� ������; 1B ѡ�� ҳ���� ���� ���;
//         - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
//         - [3:0] FCTRL   = 0000B ������; 0010B ҳ����; 0001B ���; 
//         */
//     ISPCN = FLASH_READ_DATA;
//     /* ISPAL ISP��ַ���ֽ� */
//     ISPAL = LOBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);
//     /* ISPAH ISP��ַ���ֽ� */
//     ISPAH = HIBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);

//     k1 = 0;
//     /* XXX  ����ĳ���¼�������� */
//     for (i = 0; i < RecordLEN[FirstAddr_index]; i++)
//     {
//         Trigger_ISP();

//         /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//             - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//             - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//         */
//         k2 = ISPFD;
//         /* �����ȡ�ĵ�һ����¼�ļ�¼�� ���ڴ����͵�����¼�� ����С�ڵ���k1(��һ���ļ�¼��) ���ߵ���0 */
//         if (k2 > RecordLEN[FirstAddr_index] || (k2 <= k1) || (k2 == 0))
//         {
//             /* ��ʾ�洢�ļ�¼��һ�黹û���� �� 1 0xff 0xff (��3����¼�ļ�¼��) */
//             Disable_ISP_Mode();
//             return i;
//         }

//         /* ISPAL ISP��ַ���ֽ� ���ϴ����͵�����¼��ռ���ֽ��� ����һ����¼����ʼ��ַ */
//         ISPAL += NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];

//         /* ʹ��k1 �洢������¼�ļ�¼�� */
//         k1 = k2;
//         /* Brown-Out Detector ��Դ��ѹ��� */
//         check_BOD();
//     }
//     Disable_ISP_Mode();

//     /* ��ʾ��¼�Ѵ��� ���׵�ַ���������¼�ļ�¼�������Ǵ�1��ʼ ������RecordLEN[FirstAddr_index] �磺1 2 3 (��3����¼�ļ�¼��) */
//     if (i == RecordLEN[FirstAddr_index])
//         return RecordLEN[FirstAddr_index];

//     return 0;
// }

/*******************************************************************************  
* �� �� ��: void ReadRecordData(uint8_t FirstAddr_index, uint8_t recordnumber)
* ��������: ����һ���׵�ַ���±�ֵ,�ͼ�¼�ţ�����һ����¼��
* ����˵��: ��
* ���ú���: ��
* ȫ�ֱ���: ��
* ��    ��: FirstAddr_index:�����������RECORD_FIRST_ADDRESS���±꣩��
            recordnumber:��¼���ǲ��ܴ���RecordLEN[FirstAddr_index]�ġ�
* ��    ��:  
* �� �� �ߣ� �����                  ���ڣ�2015.08.18
* �� �� �ߣ�                         ���ڣ�
* ��    ����V1.00
*ʹ��ע�����
1����Ȼ���±�ֵ�����ܴ���������±�����������ú���ʲôҲ������
********************************************************************************/
// void ReadRecordData(uint8_t FirstAddr_index, uint8_t recordnumber)
// {
//     uint8_t i, j = FirstAddr_index & 0x80;

//     FirstAddr_index = FirstAddr_index & 0x7f;
//     if ((FirstAddr_index >= (sizeof(RECORD_FIRST_ADDRESS))))
//         return;
//     if (recordnumber > RecordLEN[FirstAddr_index])
//         return;

//     demaAD[0] = demaAD[1] = demaAD[2] = demaAD[3] = 0;

//     switch (FirstAddr_index)
//     {
//         /* Check ��ѯ�����¼���� */
//         case 0:
//         {
//             rxbuf[0] = 0xaa;
//             rxbuf[1] = 0;
//             rxbuf[2] = 0;
//             rxbuf[3] = 8;
//             /* XXX �ɰ水��8�����ݼ��� �°����Ϊ7�� */
//             for (i = 1; i <= 8; i++)
//             {   
//                 /* ����FLASH���ܼ�¼���� */
//                 rxbuf[i + 3] = ReadRecordTotal(i);
//             }
//             rxbuf[12] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//             rxbuf[13] = 0x55;

//             for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//             {
//                 UART_SEND(rxbuf[i]);
//                 /* ���������ʱ���� �ؼ����� ������ */
//                 delay_1ms(5);
//             }
//             break;
//         }
//         /* Check ��ѯ̽�����ĵ�ǰʱ�� */
//         case 9:
//         {
//             rxbuf[0] = 0xaa;
//             rxbuf[1] = 0;
//             rxbuf[2] = 9;
//             rxbuf[3] = 6;

//             /* ����һ�ζ�ʱ�亯�� Լ��ʱ500uS */
//             i2c_get_time();

//             rxbuf[4] = (2000 + i2c_time_code[6]) / 256;
//             rxbuf[5] = (2000 + i2c_time_code[6]) % 256;
//             rxbuf[6] = i2c_time_code[5];
//             rxbuf[7] = i2c_time_code[3];
//             rxbuf[8] = i2c_time_code[2];
//             rxbuf[9] = i2c_time_code[1];
//             rxbuf[10] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//             rxbuf[11] = 0x55;
//             for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//             {
//                 UART_SEND(rxbuf[i]);
//                 /* ���������ʱ���� �ؼ����� ������ */
//                 delay_1ms(5);
//             }
//             break;
//         }
//         /* ��ѯ�����¼��Ϊrecordnumber�ļ�¼ */
//         default:
//         {
//             /* �����ݼ�¼��ѯ */
//             if (recordnumber == 0)
//                 return;

//             Enable_ISP_Mode();

//             /* ISPCN = 0000 0000B ISP���� ѡ�����APROM��������FLASH ����FLASH������
//                 - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
//                 - [5] FOEN      = 0B ѡ�� ������; 1B ѡ�� ҳ���� ���� ���;
//                 - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
//                 - [3:0] FCTRL   = 0000B ������; 0010B ҳ����; 0001B ���; 
//                 */
//             ISPCN = FLASH_READ_DATA;
//             /* ISPAL ISP��ַ���ֽ� �������ͼ��������׵�ַ */
//             ISPAL = LOBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);
//             /* ISPAH ISP��ַ���ֽ� �������ͼ��������׵�ַ */
//             ISPAH = HIBYTE(RECORD_FIRST_ADDRESS[FirstAddr_index]);

//             /* ��ѯ�ļ�¼��ҪС�ڵ��ڸü�¼���涨�������� */
//             for (i = 0; i < RecordLEN[FirstAddr_index]; i++)
//             {
//                 Trigger_ISP();
//                 delay_1ms(0);

//                 /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                     - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                     - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//                 */
//                 rxbuf[1] = ISPFD;
//                 /* ��ѯ������ļ�¼�����˳� */
//                 if (rxbuf[1] == recordnumber)
//                 {
//                     break;
//                 }
//                 /* ISPAL ISP��ַ���ֽ� ���ϴ����͵�����¼��ռ���ֽ��� ����һ����¼����ʼ��ַ */
//                 ISPAL += NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//             }

//             /* ����ѯ���ļ�¼��С�ڵ��ڸü�¼���涨�������� */
//             if (i < RecordLEN[FirstAddr_index])
//             {
//                 /* ����ȡ�ļ�¼����Ϊ�궨ֵ */
//                 if (FirstAddr_index == DEM_RECORD)
//                 {
//                     for (i = 0; i < 4; i++)
//                     {
//                         /* ISPAL ISP��ַ���ֽ� ��1 */
//                         ISPAL++;
//                         /* ��ȡ�궨ֵ */
//                         Trigger_ISP();
//                         _nop_();
//                         _nop_();
//                         _nop_();
//                         _nop_();
//                         _nop_();
//                         delay_1ms(0);
//                         /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                             - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                             - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//                         */
//                         /* ���궨ֵ���������� */
//                         demaAD[i] = ISPFD;
//                     }
//                 }

//                 /* �˴����� �� �� �� ʱ �� ��5���ֽڵ����� */
//                 for (i = 10; i > 5; i--)
//                 {
//                     /* ISPAL ISP��ַ���ֽ� */
//                     ISPAL++;
//                     Trigger_ISP();
//                     _nop_();
//                     _nop_();
//                     _nop_();
//                     _nop_();
//                     _nop_();
//                     delay_1ms(0);
//                     /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                         - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                         - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//                     */
//                     rxbuf[i] = ISPFD;
//                 }
//                 Disable_ISP_Mode();

//                 /* ������¼�ļ�¼��n1 */
//                 /* ����ֽ� */
//                 rxbuf[5] = (rxbuf[6] + 2000) / 256;
//                 /* ����ֽ� */
//                 rxbuf[6] = (rxbuf[6] + 2000) % 256;
//                 rxbuf[0] = 0xaa;
//                 rxbuf[2] = rxbuf[2];
//                 rxbuf[3] = FRAME_DATA_LEN[FirstAddr_index];
//                 rxbuf[4] = rxbuf[1];
//                 rxbuf[11] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//                 rxbuf[12] = 0x55;
//                 if (!j)
//                 {
//                     for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//                     {
//                         UART_SEND(rxbuf[i]);
//                         /* ���������ʱ���� �ؼ����� ������ */
//                         delay_1ms(5);
//                     }
//                 }
//             }
//             /* ����ѯ���ļ�¼�Ŵ��ڸü�¼���涨�������� */
//             else
//             {
//                 /* ��Ҫ��ȡ���Ǵ��������� XXX ������޸�*/
//                 if (FirstAddr_index == LIFE_RECORD)
//                 {
//                     /* �˴�Ӧ���䴫������ʧЧ���� n1��ʾ�Ƿ�ʧЧ 
//                         - n1 = 0 δʧЧ ʧЧ���ھ�Ϊ0 
//                         - n1 = 1 ��ʧЧ ʧЧ����Ϊn2 - n7 */
//                     rxbuf[0] = 0xaa;
//                     rxbuf[1] = 1;
//                     rxbuf[3] = FRAME_DATA_LEN[FirstAddr_index];
//                     rxbuf[4] = 0;
//                     rxbuf[5] = 0;
//                     rxbuf[6] = 0;
//                     rxbuf[7] = 0;
//                     rxbuf[8] = 0;
//                     rxbuf[9] = 0;
//                     rxbuf[10] = 0;
//                     rxbuf[11] = Get_crc(rxbuf, FRAME_TOTAL_LEN[FirstAddr_index]);
//                     rxbuf[12] = 0x55;
//                     for (i = 0; i < FRAME_TOTAL_LEN[FirstAddr_index]; i++)
//                     {
//                         UART_SEND(rxbuf[i]);
//                         /* ���������ʱ���� �ؼ����� ������ */
//                         delay_1ms(5);
//                     }
//                 }
//                 Disable_ISP_Mode();
//                 return;
//             }
//             break;
//         }
//     }
// }

/*******************************************************************************  
* �� �� ��: void WriteRecordData(uint8_t FirstAddr_index)
* ��������: ����һ���׵�ַ���±�ֵ,��дһ����¼��
* ����˵��: ��
* ���ú���: ��
* ȫ�ֱ���: ��
* ��    ��: FirstAddr_index:�����������RECORD_FIRST_ADDRESS���±꣩��

* ��    ��:  
* �� �� �ߣ� �����                  ���ڣ�2015.08.18
* �� �� �ߣ�                         ���ڣ�
* ��    ����V1.00
*ʹ��ע�����
1����Ȼ���±�ֵ�����ܴ���������±�����������ú���ʲôҲ������
********************************************************************************/
// void WriteRecordData(uint8_t FirstAddr_index)
// {
//     uint8_t i, j, k, cnt;
//     uint16_t writeaddres, readaddres;

//     if (FirstAddr_index <= 0 || (FirstAddr_index >= (sizeof(RECORD_FIRST_ADDRESS) - 1)))
//         return;

//     /* Erase copy_Adress ����д��ʱ������ */
//     flash_erase_page(TEMP_PAGE_ADDR);
    
//     /* copy to copy_Adress */
//     i = 0;
//     cnt = 0;
//     for (j = 0; j < RecordLEN[FirstAddr_index]; j++)
//     {
//         /* ��������ͼ�¼��ÿ����¼���׵�ַ */
//         writeaddres = RECORD_FIRST_ADDRESS[FirstAddr_index] + j * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//     REREAD:
//         Enable_ISP_Mode();
//         /* ISPCN = 0000 0000B ISP���� ѡ�����APROM��������FLASH ����FLASH������
//             - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
//             - [5] FOEN      = 0B ѡ�� ������; 1B ѡ�� ҳ���� ���� ���;
//             - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
//             - [3:0] FCTRL   = 0000B ������; 0010B ҳ����; 0001B ���; 
//             */
//         ISPCN = FLASH_READ_DATA;
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP��ַ���ֽ� */
//         ISPAH = HIBYTE(writeaddres);

//         /* ��һ����¼ */
//         for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//         {
//             Trigger_ISP();
//             /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                 - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                 - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//             */
//             copyeep[k] = ISPFD;
//             /* ISPAL ISP��ַ���ֽ� */
//             ISPAL++;
//         }
//         Disable_ISP_Mode();

//         /* copyeep[0] Ϊ������¼�ļ�¼��� */
//         if ((copyeep[0] != (j + 1)) || copyeep[0] > RecordLEN[FirstAddr_index] || (copyeep[0] <= i) || (copyeep[0] == 0))
//         {
//             /* ��ʾ�洢�ļ�¼��һ�黹û�������� 1 0xff 0xff �繲3����¼�ļ�¼�� */
//             if (copyeep[0] != 0xff)
//             {
//                 if (++cnt <= 2)
//                 {
//                     Delay_1ms(100);
//                     goto REREAD;
//                 }
//             }

//             break;
//         }
//         i = copyeep[0];

//         /* ����д��ĵ�ַ */
//         writeaddres = TEMP_PAGE_ADDR + j * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         Enable_ISP_Mode();

//         /* ISPCN = 0010 0001B ISP���� ѡ����� APROM��������FLASH ����FLASH���
//             - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
//             - [5] FOEN      = 1B ѡ�� ҳ���� ���� ���; 0B ѡ�� ������;
//             - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
//             - [3:0] FCTRL   = 0001B ���; 0000B ������; 0010B ҳ����; 
//             */
//         ISPCN = BYTE_PROGRAM_DATA;
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP��ַ���ֽ� */
//         ISPAH = HIBYTE(writeaddres);

//         /* д��һ����¼�� copy_Adress */
//         for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//         {
//             /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                 - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                 - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//             */
//             ISPFD = copyeep[k];
//             Trigger_ISP();
//             _nop_();
//             _nop_();
//             _nop_();
//             _nop_();
//             _nop_();
//             /* ISPAL ISP��ַ���ֽ� */
//             ISPAL++;
//         }
//         Disable_ISP_Mode();
//     }

//     /* XXX �����ü�¼�������ڵ�ҳ */
//     flash_erase_page(RECORD_FIRST_ADDRESS[FirstAddr_index]);

//     /* XXX */
//     if (j < RecordLEN[FirstAddr_index])
//         readaddres = TEMP_PAGE_ADDR;
//     else
//     {
//         /* XXX */
//         readaddres = TEMP_PAGE_ADDR + NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         /* XXX ����¼ */
//         j = RecordLEN[FirstAddr_index] - 1;
//     }

//     /* �ڱ���ҳ��ѭ������¼ �� д��¼�ڸ�����������Flash��ַ�� */
//     for (i = 0; i < j; i++)
//     {
//         /* д��ַ = ����ַ + ��ǰ��¼��i * ÿ����¼�ĳ��� */
//         writeaddres = readaddres + i * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         Enable_ISP_Mode();
//         /* ISPCN = 0000 0000B ISP���� ѡ�����APROM��������FLASH ����FLASH������
//             - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
//             - [5] FOEN      = 0B ѡ�� ������; 1B ѡ�� ҳ���� ���� ���;
//             - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
//             - [3:0] FCTRL   = 0000B ������; 0010B ҳ����; 0001B ���; 
//             */
//         ISPCN = FLASH_READ_DATA;
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP��ַ���ֽ� */
//         ISPAH = HIBYTE(writeaddres);

//         /* �ӱ���ҳ XXX ��д��ַ �ж�ȡһ����¼ ����copyeep[]�� */
//         for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//         {
//             Trigger_ISP();
//             /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                 - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                 - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//             */
//             copyeep[k] = ISPFD;
//             /* ISPAL ISP��ַ���ֽ� */
//             ISPAL++;
//         }
//         Disable_ISP_Mode();

//         /* д��ַ = �ü�¼�����׵�ַ + ��ǰ��¼��i * ÿ����¼�ĳ��� */
//         writeaddres = RECORD_FIRST_ADDRESS[FirstAddr_index] + i * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//         Enable_ISP_Mode();

//         /* ISPCN = 0010 0001B ISP���� ѡ����� APROM��������FLASH ����FLASH���
//             - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
//             - [5] FOEN      = 1B ѡ�� ҳ���� ���� ���; 0B ѡ�� ������;
//             - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
//             - [3:0] FCTRL   = 0001B ���; 0000B ������; 0010B ҳ����; 
//             */
//         ISPCN = BYTE_PROGRAM_DATA;
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL = LOBYTE(writeaddres);
//         /* ISPAH ISP��ַ���ֽ� */
//         ISPAH = HIBYTE(writeaddres);

//         /* д ��һ�� ��¼���ü�¼���͵�FLASH�� */
//         /* ����ַ���ڱ���ҳ���׵�ַ */
//         if (readaddres > TEMP_PAGE_ADDR)
//         {
//             /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                 - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                 - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//             */
//             /* �µļ�¼��� */
//             ISPFD = i + 1;
//             Trigger_ISP();

//             /* ISPAL ISP��ַ���ֽ� */
//             ISPAL++;
//             /* дһ����¼ */
//             for (k = 1; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//             {
//                 /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                     - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                     - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//                 */
//                 ISPFD = copyeep[k];
//                 Trigger_ISP();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 /* ISPAL ISP��ַ���ֽ� */
//                 ISPAL++;
//             }
//         }
//         /* ����ַС�ڵ��ڱ���ҳ���׵�ַ */
//         else
//         {
//             /* дһ����¼ */
//             for (k = 0; k < NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index]; k++)
//             {
//                 /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//                     - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//                     - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//                 */
//                 ISPFD = copyeep[k];
//                 Trigger_ISP();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 _nop_();
//                 /* ISPAL ISP��ַ���ֽ� */
//                 ISPAL++;
//             }
//         }
//         Disable_ISP_Mode();
//     }

//     /* д�����¼�������͵�FLASH�� �¼�¼�� = �ɼ�¼�� + 1 */
//     writeaddres = RECORD_FIRST_ADDRESS[FirstAddr_index] + j * NUMBER_OF_BYTES_PER_RECORD[FirstAddr_index];
//     Enable_ISP_Mode();

//     /* ISPCN = 0010 0001B ISP���� ѡ����� APROM��������FLASH ����FLASH���
//         - [7:6] A17:A16 = 00B 00B ѡ�����APROM��������FLASH; 01B ѡ��LDROM; 11B CONFIG���⹦��;
//         - [5] FOEN      = 1B ѡ�� ҳ���� ���� ���; 0B ѡ�� ������;
//         - [4] FCEN      = 0B ���������� ֻ��Ϊ0;
//         - [3:0] FCTRL   = 0001B ���; 0000B ������; 0010B ҳ����; 
//         */
//     ISPCN = BYTE_PROGRAM_DATA;
//     /* ISPAL ISP��ַ���ֽ� */
//     ISPAL = LOBYTE(writeaddres);
//     /* ISPAH ISP��ַ���ֽ� */
//     ISPAH = HIBYTE(writeaddres);

//     /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ 
//         - ���ģʽ��    �û���Ҫ�ڴ���ISP֮ǰд���ݵ�ISPFD�� 
//         - ��/У��ģʽ�� ��ISP��ɺ��ISPFD��������
//     */
//     /* ���µļ�¼�� */
//     ISPFD = j + 1;
//     Trigger_ISP();
//     _nop_();
//     _nop_();
//     _nop_();
//     _nop_();
//     _nop_();
//     /* ISPAL ISP��ַ���ֽ� */
//     ISPAL++;

//     /* ��ʾд���Ǳ궨��¼ �������ļ�¼���˸�0��AD �ͱ궨��AD */
//     if (FirstAddr_index == DEM_RECORD)
//     {
//         /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ */
//         ISPFD = ch4_0;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL++;

//         /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ */
//         ISPFD = ch4_0 >> 8;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL++;

//         /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ */
//         ISPFD = ch4_3500;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL++;

//         /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ */
//         ISPFD = ch4_3500 >> 8;
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL++;
//     }

//     /* ��¼����: �� �� �� ʱ �� */
//     for (i = 1; i < 6; i++)
//     {
//         /* ISPFD ISP�ڴ����� ���ֽڰ�����Ҫ����д���ڴ�ռ������ */
//         ISPFD = timeCOPY[i];
//         Trigger_ISP();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         _nop_();
//         /* ISPAL ISP��ַ���ֽ� */
//         ISPAL++;
//     }
//     Disable_ISP_Mode();
// }

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

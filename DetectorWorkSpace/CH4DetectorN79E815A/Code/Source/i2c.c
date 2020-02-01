/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      i2c.c
 * @version   V1.0.0
 * @brief     I2C Function(Master)
 *            SCL => P1.2
 *            SDA => P1.3
 *            Output => P1.4 toggle when I2C function pass
 *            I2CEN = 1 enable I2C function
 *            I2C Frequency == Fosc/(I2CLK+1)
 *            Use external OSC Fosc = 11.0592MHz
 *  
 *            <o0.0..7> I2CLK<2-255>
 *            <o1.0..7> I2C Slave Address<160-175>
 *            <h> UART pin Select
 *            <o2.6> Uart pin
 *            <0=> Select P1.0, P1.1 as UART pin(default)
 *            <1=> Select P2.6, P2.7 as UART pin(28 pin only)
 *            </h>
 * @warning   
 *******************************************************************************
 * @remarks
 * 1. Version                Date                 Author
 *    v1.0.0                 2020��1��1��          
 *    Modification: �����ĵ�
 *******************************************************************************
 */
/* NEED FIX */
/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "N79E81x.h"
#include "i2c.h"
#include "delay.h"
#include "utlities.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define I2C_CLOCK 255

/* 1:��ʾʱ��оƬΪPCF8563 0����ʾʱ��оƬΪRX8010SJ */
#define PCF8563  0

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/
static void i2c_start(void);
static void i2c_8563_address(uint8_t sub);
static void i2c_reapeat_start(void);
static void i2c_read_data(uint8_t *temp, uint8_t u8_data);
static void i2c_deal_time_code(void);
static void i2c_stop(void);
// static void i2c_write_data(uint8_t *time, uint8_t length);


/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/
/* ��--����--Сʱ--��--����--��--�� */
uint8_t i2c_time_code[7] = {0x00};

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void i2c_init(void)
{
    /* I2C ����ģʽ ʱ������ f[I2C] = f[PHERI] / (1 + I2CLK) �ӻ�ģʽ���ֽ���Ч ���Զ�ͬ�� 400kps ���µ����� */
    I2CLK = I2C_CLOCK;
    /* ʹ�� I2C���� */
    I2CEN = 1;
}

/* ����һ�ζ����� Լ��ʱ500uS */
void i2c_get_time(void)
{
    uint8_t i = 0;

    i2c_start();
#if PCF8563 == 1
    /* 8563 ����д��ַ */
    i2c_8563_address(0xA2);
    i2c_8563_address(0x02);

    /* I2C ֹͣ������ */
    i2c_reapeat_start();
    
    /* 8563 ����д��ַ */
    i2c_8563_address(0xA3);
    i2c_read_data(i2c_time_code, 7);
    i2c_deal_time_code();
#else
    /* 8563 ����д��ַ */
    i2c_8563_address(0x64);
    i2c_8563_address(0x10);
    
    /* I2C ֹͣ������ */
    i2c_reapeat_start();

    /* 8563 ����д��ַ */
    i2c_8563_address(0x65);
    i2c_read_data(i2c_time_code, 7);
    i2c_time_code[3] += i2c_time_code[4];
    i2c_time_code[4] = i2c_time_code[3] - i2c_time_code[4];
    i2c_time_code[3] = i2c_time_code[3] - i2c_time_code[4];
    i2c_deal_time_code();
#endif

    for (i = 0; i < 7; i++)
    {
        i2c_time_code[i] = bcd2hex(i2c_time_code[i]);
    }
    i2c_stop();
}

// void i2c_set_time(void)
// {
//     uint8_t i = 0;
    
//     i2c_start();
// #if PCF8563 == 1
//     /* 8563 ����д��ַ */
//     i2c_8563_address(0xA2);
//     i2c_8563_address(0x02);

//     /* 8563 д��ʱ������ */
//     i2c_write_data(i2c_time_code, 7);
// #else
//     /* 8563 ����д��ַ */
//     i2c_8563_address(0x64);
//     i2c_8563_address(0x10);

//     i = i2c_time_code[3];
//     i2c_time_code[3] = i2c_time_code[4];
//     i2c_time_code[4] = i;
//     /* 8563 д��ʱ������ */
//     i2c_write_data(i2c_time_code, 7);
//     i = i2c_time_code[3];
//     i2c_time_code[3] = i2c_time_code[4];
//     i2c_time_code[4] = i;
// #endif
//     i2c_stop();
// }

// void i2c_start_rtc(void)
// {
//     i2c_start();
// #if PCF8563 == 1
//     /* 8563 ����д��ַ */
//     i2c_8563_address(0xA2);
//     i2c_8563_address(0x00);

//     i2c_time_code[0] = 0;
//     i2c_write_data(i2c_time_code, 1);
// #else
//     /* 8563 ����д��ַ */
//     i2c_8563_address(0x64);
//     i2c_8563_address(0x1f);

//     i2c_time_code[0] = 0;
//     i2c_write_data(i2c_time_code, 1);
// #endif
//     i2c_stop();
// }

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/
static void i2c_start(void)
{
    uint8_t delay_count = 0;

    /* I2C Start��־ �����߿��������I2C��ʼ�ź� */
    STA = 1;

    /* ��⴮���жϱ�־SI�Ƿ���λ ��λ������ѭ�� ͨ�����Ե�֪ �ȴ�ʱ��Ϊcnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }

    /* I2C Start��־ �ڼ�⵽��ʼ�źź�Ӧ�ֶ����� */
    STA = 0;
}

static void i2c_reapeat_start(void)
{
    uint8_t delay_count = 0;

    /* I2C Start��־ �����߿��������I2C��ʼ�ź� */
    STA = 1;
    /* STO��1 �������������ֹͣ�ź� */
    STO = 1;
    /* �����жϱ�־SI���� ���д�����ͣ */
    SI = 0;

    /* ��⴮���жϱ�־SI�Ƿ���λ ��λ������ѭ�� ͨ�����Ե�֪ �ȴ�ʱ��Ϊcnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }

    /* I2C Start��־ �ڼ�⵽��ʼ�źź�Ӧ�ֶ����� */
    STA = 0;
}

static void i2c_stop(void)
{
    uint8_t delay_count = 0;

    /* STO��1 �������������ֹͣ�ź� */
    STO = 1;
    /* �����жϱ�־SI���� ���д�����ͣ */
    SI = 0;

    /* ���STOֹͣ��־ ͨ�����Ե�֪ �ȴ�ʱ��Ϊcnnt=0x0e */
    while ((I2CON & 0x10) == 0x10)
    {
        if (++delay_count > 250)
        {
            break;
        }
    }
}

/* д�Ĵ�����ַ */
static void i2c_8563_address(uint8_t sub)
{
    uint8_t delay_count = 0;

    /* I2C ���ݼĴ��� ֻҪSIΪ�߼�1 I2DAT�е����ݱ��ֲ��� ��I2C���ͽ��չ����� ����дI2DAT�Ľ�����ǲ�ȷ���� */
    /* XXX Address high for I2C EEPROM */
    I2DAT = sub;
    /* �����жϱ�־SI���� ���д�����ͣ */
    SI = 0;

    /* ��⴮���жϱ�־SI�Ƿ���λ ��λ������ѭ�� ͨ�����Ե�֪ �ȴ�ʱ��Ϊcnnt=0x0e */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }
}

// static void i2c_write_data(uint8_t *time, uint8_t length)
// {
//     uint8_t *ptemp = time;
//     uint8_t i = 0;
//     uint8_t delay_count;

//     for (i = 0; i < length; i++)
//     {
//         /* I2C ���ݼĴ��� ֻҪSIΪ�߼�1 I2DAT�е����ݱ��ֲ��� ��I2C���ͽ��չ����� ����дI2DAT�Ľ�����ǲ�ȷ���� */
//         I2DAT = *ptemp;
//         /* �����жϱ�־SI���� ���д�����ͣ */
//         SI = 0;

//         delay_count = 0;
//         /* ͨ�����Ե�֪���ȴ�ʱ��Ϊcnnt=0x69��������֪��������乲7��ָ���ָ����=735.=33uS@22M */
//         /* ��⴮���жϱ�־SI�Ƿ���λ ��λ������ѭ�� ͨ�����Ե�֪ �ȴ�ʱ��Ϊcnnt=0x0e */
//         while (!(I2CON & SET_BIT3))
//         {
//             if (++delay_count > 250)
//             {
//                 break;
//             }
//         }

//         ptemp++;
//     }
// }

/* ��I2C�ж�ȡ�̶��������ݷ��뻺������ */
static void i2c_read_data(uint8_t *temp, uint8_t length)
{
    uint8_t i = 0;
    uint8_t delay_count;

    for (i = 0; i < length; i++)
    {
        /* I2CON[2] -> AA
            - ���AA��־��λ ��I2C�豸Ϊ������ʱ ��Ӧ��ʱ�������ڼ佫����ACK��
            - ���AA������ ��I2C�豸�ǽ�����ʱ ��Ӧ��ʱ�������ڼ佫����NACK (SDA��Ϊ�ߵ�ƽ)
            */
        AA = 1;

        /* �����жϱ�־SI���� ���д�����ͣ */
        SI = 0;

        delay_count = 0;
        /* ��⴮���жϱ�־SI�Ƿ���λ ��λ������ѭ�� ͨ�����Ե�֪ �ȴ�ʱ��Ϊcnnt=0x69 */
        while (!(I2CON & SET_BIT3))
        {
            if (++delay_count > 250)
            {
                break;
            }
        }
        *temp = I2DAT;
        temp++;
    }

    /* I2CON[2] -> AA
        - ���AA��־��λ ��I2C�豸Ϊ������ʱ ��Ӧ��ʱ�������ڼ佫����ACK��
        - ���AA������ ��I2C�豸�ǽ�����ʱ ��Ӧ��ʱ�������ڼ佫����NACK (SDA��Ϊ�ߵ�ƽ)
        */
    AA = 0;

    /* �����жϱ�־SI���� ���д�����ͣ */
    SI = 0;

    delay_count = 0;
    /* ��⴮���жϱ�־SI�Ƿ���λ ��λ������ѭ�� ͨ�����Ե�֪ �ȴ�ʱ��Ϊcnnt=0x69 */
    while (!(I2CON & SET_BIT3))
    {
        if (++delay_count > 250)
        {
            break;
        }
    }
}

/* ��������ת�� */
static void i2c_deal_time_code(void)
{
    /* second */
    i2c_time_code[0] = i2c_time_code[0] & 0x7f;
    /* minuter */
    i2c_time_code[1] = i2c_time_code[1] & 0x7f;
    /* hour */
    i2c_time_code[2] = i2c_time_code[2] & 0x3f;
    /* day */
    i2c_time_code[3] = i2c_time_code[3] & 0x3f;
    /* week */
    i2c_time_code[4] = i2c_time_code[4] & 0x07;
    /* month */
    i2c_time_code[5] = i2c_time_code[5] & 0x1f;
    /* year */
    i2c_time_code[6] = i2c_time_code[6] & 0xff;
}

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

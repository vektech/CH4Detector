/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      signal.c
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

#include "signal.h"
#include "delay.h"
#include "uart.h"
#include "device.h"

extern uint8_t rx_index;

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
#define CH4_EXCEEDED_LED_ON     LED_ALARM_ON;      /* �����ƿ� */
#define CH4_EXCEEDED_LED_OFF    LED_ALARM_OFF;     /* �����ƿ� */

#define SELFCHECK_LED_ON        do                              \
                                {                               \
                                    LED_POWER_ON;  /* ��Դ�ƿ� */ \
                                    LED_FAULT_ON;  /* ���ϵƿ� */ \
                                    LED_ALARM_ON;  /* �����ƿ� */ \
                                    LED_LIFE_ON;   /* �����ƿ� */ \
                                } while(0)
#define SELFCHECK_LED_OFF       do                               \
                                {                                \
                                    LED_POWER_OFF; /* ��Դ�ƹ� */ \
                                    LED_FAULT_OFF; /* ���ϵƹ� */ \
                                    LED_ALARM_OFF; /* �����ƹ� */ \
                                    LED_LIFE_OFF;  /* �����ƹ� */ \
                                } while(0)

#define DEMARCATION_LED_ON      do                               \
                                {                                \
                                    LED_POWER_ON;  /* ��Դ�ƿ� */ \
                                    LED_FAULT_ON;  /* ���ϵƿ� */ \
                                    LED_ALARM_ON;  /* �����ƿ� */ \
                                    LED_LIFE_ON;   /* �����ƿ� */ \
                                } while(0)
#define DEMARCATION_LED_OFF     do                               \
                                {                                \
                                    LED_POWER_OFF; /* ��Դ�ƹ� */ \
                                    LED_FAULT_OFF; /* ���ϵƹ� */ \
                                    LED_ALARM_OFF; /* �����ƹ� */ \
                                    LED_LIFE_OFF;  /* �����ƹ� */ \
                                } while(0)

#define FAULT_LED_ON            LED_FAULT_ON;      /* ���ϵƿ� */
#define FAULT_LED_OFF           LED_FAULT_OFF;     /* ���ϵƹ� */

#define EXPIRED_LED_ON          LED_LIFE_ON;       /* �����ƿ� */
#define EXPIRED_LED_OFF         LED_LIFE_OFF;      /* �����ƹ� */

/* ������״̬1 */
#define SOUND_STATE0            do                                \
                                {                                 \
                                    SOUND1_ON;     /* ������1�� */ \
                                    SOUND2_OFF;    /* ������2�� */ \
                                } while(0)
/* ������״̬2 */
#define SOUND_STATE1            do                                \
                                {                                 \
                                    SOUND1_OFF;    /* ������1�� */ \
                                    SOUND2_ON;     /* ������2�� */ \
                                } while(0)
/* �������ر� */
#define SOUND_OFF               do                                \
                                {                                 \
                                    SOUND1_OFF;    /* ������1�� */ \
                                    SOUND2_OFF;    /* ������2�� */ \
                                } while(0)

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Prototype Declare Section ('static function')
 ******************************************************************************/
static void signal_beeper(uint8_t interval, uint16_t repeat_time);

/*******************************************************************************
 *                 Global Variable Declare Section ('variable')
 ******************************************************************************/

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void device_alarm(enum alarm_type type)
{
    uint16_t i = 0, k = 0;
    uint8_t j = 0;

    /* check rx_index */
    if (!rx_index)
    {
        /* rx_index = 0 ��ֹ��ʱ��2 ���λ����ֹ��ʱ��2������ǰ����������TH2��TL2 */
        TR2 = 0;
    }
    else
    {
        /* rx_index = 1 �򿪶�ʱ��2 */
        TR2 = 1;
    }

    switch (type)
    {
        case Alarm_CH4_Exceeded:
        {
            /* �����ƿ� */
            CH4_EXCEEDED_LED_ON;

            /* ���������� */
            signal_beeper(7, 3000);

            /* �����ƹ� */
            CH4_EXCEEDED_LED_OFF;

            /* Brown-Out Detector ��Դ��ѹ��� */
            check_BOD();

            /* �򿪶�ʱ��2 */
            TR2 = 1;
            
            break;
        }
        case Alarm_Selfcheck:
        {
            /* �����Ŀŵ���˸�ͷ��������� ����Ϊ�ظ����� */
            for (j = 0; j < 3; j++)
            {
                /* �Լ���صƿ� */
                SELFCHECK_LED_ON;

                /* ���������� */
                signal_beeper(7, 1000);

                /* �Լ���صƹ� */
                SELFCHECK_LED_OFF;

                /* ��ʱ 1600 * 70 = 112000us */
                for (k = 0; k < 1600; k++)
                {
                    delay_10us(7);
                    /* Brown-Out Detector ��Դ��ѹ��� */
                    check_BOD();
                }
            }
            /* �򿪶�ʱ��2 */
            TR2 = 1;
            break;
        }
        case Alarm_Demarcation:
        {
            /* �����Ŀŵ���˸�ͷ��������� ����Ϊ�ظ����� �����������Ƶ�ʲ�ͬ */
            for (j = 0; j < 3; j++)
            {
                /* �궨��صƿ� */
                DEMARCATION_LED_ON;

                /* ���������� */
                signal_beeper(50, 150);

                /* �궨��صƹ� */
                DEMARCATION_LED_OFF;

                /* �ȴ� 200 * 70 = 14000us */
                for (k = 0; k < 200; k++)
                {
                    delay_10us(7);
                    /* Brown-Out Detector ��Դ��ѹ��� */
                    check_BOD();
                }
            }
            /* �򿪶�ʱ��2 */
            TR2 = 1;
            break;
        }   
        case Alarm_Fault:
        {
            /* ���ϵƿ� */
            FAULT_LED_ON;

            /* ���������� */
            signal_beeper(10, 1000);

            /* ���ϵƹ� */
            FAULT_LED_OFF;

            /* Brown-Out Detector ��Դ��ѹ��� */
            check_BOD();
            
            /* �򿪶�ʱ��2 */
            TR2 = 1;

            for (k = 0; k < 1800; k++)
            {
                delay_10us(10);
                /* Brown-Out Detector ��Դ��ѹ��� */
                check_BOD();
            }
            break;
        }
        case Alarm_Expired:
        {
            /* ���ϵƿ� */
            EXPIRED_LED_ON;

            /* �򿪶�ʱ��2 */
            TR2 = 1;

            for (k = 0; k < 1800; k++)
            {
                delay_10us(10);
                /* Brown-Out Detector ��Դ��ѹ��� */
                check_BOD();
            }

            /* ���ϵƹ� */
            EXPIRED_LED_OFF;
            
            /* �򿪶�ʱ��2 */
            TR2 = 1;

            for (k = 0; k < 1800; k++)
            {
                delay_10us(10);
                /* Brown-Out Detector ��Դ��ѹ��� */
                check_BOD();
            }
            break;
        }
        default:
            break;
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/
static void signal_beeper(uint8_t interval, uint16_t repeat_time)
{
    uint16_t i = 0;

    do
    {
        /*  Brown-Out Detector ��Դ��ѹ��� */
        check_BOD();
        /* ������״̬0 */
        SOUND_STATE0;
        delay_10us(interval);
        i++;

        /*  Brown-Out Detector ��Դ��ѹ��� */
        check_BOD();
        /* ������״̬1 */
        SOUND_STATE1;
        delay_10us(interval);
        i++;
    } while (i < repeat_time);

    /* �������ر� */
    SOUND_OFF;
}

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

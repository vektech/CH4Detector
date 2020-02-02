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
 *    v1.0.0                 2020年1月1日          Unknown
 *    Modification: 创建文档
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
#define CH4_EXCEEDED_LED_ON     LED_ALARM_ON;      /* 报警灯开 */
#define CH4_EXCEEDED_LED_OFF    LED_ALARM_OFF;     /* 报警灯开 */

#define SELFCHECK_LED_ON        do                              \
                                {                               \
                                    LED_POWER_ON;  /* 电源灯开 */ \
                                    LED_FAULT_ON;  /* 故障灯开 */ \
                                    LED_ALARM_ON;  /* 报警灯开 */ \
                                    LED_LIFE_ON;   /* 寿命灯开 */ \
                                } while(0)
#define SELFCHECK_LED_OFF       do                               \
                                {                                \
                                    LED_POWER_OFF; /* 电源灯关 */ \
                                    LED_FAULT_OFF; /* 故障灯关 */ \
                                    LED_ALARM_OFF; /* 报警灯关 */ \
                                    LED_LIFE_OFF;  /* 寿命灯关 */ \
                                } while(0)

#define DEMARCATION_LED_ON      do                               \
                                {                                \
                                    LED_POWER_ON;  /* 电源灯开 */ \
                                    LED_FAULT_ON;  /* 故障灯开 */ \
                                    LED_ALARM_ON;  /* 报警灯开 */ \
                                    LED_LIFE_ON;   /* 寿命灯开 */ \
                                } while(0)
#define DEMARCATION_LED_OFF     do                               \
                                {                                \
                                    LED_POWER_OFF; /* 电源灯关 */ \
                                    LED_FAULT_OFF; /* 故障灯关 */ \
                                    LED_ALARM_OFF; /* 报警灯关 */ \
                                    LED_LIFE_OFF;  /* 寿命灯关 */ \
                                } while(0)

#define FAULT_LED_ON            LED_FAULT_ON;      /* 故障灯开 */
#define FAULT_LED_OFF           LED_FAULT_OFF;     /* 故障灯关 */

#define EXPIRED_LED_ON          LED_LIFE_ON;       /* 寿命灯开 */
#define EXPIRED_LED_OFF         LED_LIFE_OFF;      /* 寿命灯关 */

/* 蜂鸣器状态1 */
#define SOUND_STATE0            do                                \
                                {                                 \
                                    SOUND1_ON;     /* 蜂鸣器1开 */ \
                                    SOUND2_OFF;    /* 蜂鸣器2关 */ \
                                } while(0)
/* 蜂鸣器状态2 */
#define SOUND_STATE1            do                                \
                                {                                 \
                                    SOUND1_OFF;    /* 蜂鸣器1关 */ \
                                    SOUND2_ON;     /* 蜂鸣器2开 */ \
                                } while(0)
/* 蜂鸣器关闭 */
#define SOUND_OFF               do                                \
                                {                                 \
                                    SOUND1_OFF;    /* 蜂鸣器1关 */ \
                                    SOUND2_OFF;    /* 蜂鸣器2关 */ \
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
        /* rx_index = 0 中止定时器2 清该位将中止定时器2并将当前计数保存在TH2和TL2 */
        TR2 = 0;
    }
    else
    {
        /* rx_index = 1 打开定时器2 */
        TR2 = 1;
    }

    switch (type)
    {
        case Alarm_CH4_Exceeded:
        {
            /* 报警灯开 */
            CH4_EXCEEDED_LED_ON;

            /* 蜂鸣器报警 */
            signal_beeper(7, 3000);

            /* 报警灯关 */
            CH4_EXCEEDED_LED_OFF;

            /* Brown-Out Detector 电源电压检测 */
            check_BOD();

            /* 打开定时器2 */
            TR2 = 1;
            
            break;
        }
        case Alarm_Selfcheck:
        {
            /* 控制四颗灯闪烁和蜂鸣器鸣响 该行为重复三次 */
            for (j = 0; j < 3; j++)
            {
                /* 自检相关灯开 */
                SELFCHECK_LED_ON;

                /* 蜂鸣器报警 */
                signal_beeper(7, 1000);

                /* 自检相关灯关 */
                SELFCHECK_LED_OFF;

                /* 延时 1600 * 70 = 112000us */
                for (k = 0; k < 1600; k++)
                {
                    delay_10us(7);
                    /* Brown-Out Detector 电源电压检测 */
                    check_BOD();
                }
            }
            /* 打开定时器2 */
            TR2 = 1;
            break;
        }
        case Alarm_Demarcation:
        {
            /* 控制四颗灯闪烁和蜂鸣器鸣响 该行为重复三次 蜂鸣器鸣响的频率不同 */
            for (j = 0; j < 3; j++)
            {
                /* 标定相关灯开 */
                DEMARCATION_LED_ON;

                /* 蜂鸣器报警 */
                signal_beeper(50, 150);

                /* 标定相关灯关 */
                DEMARCATION_LED_OFF;

                /* 等待 200 * 70 = 14000us */
                for (k = 0; k < 200; k++)
                {
                    delay_10us(7);
                    /* Brown-Out Detector 电源电压检测 */
                    check_BOD();
                }
            }
            /* 打开定时器2 */
            TR2 = 1;
            break;
        }   
        case Alarm_Fault:
        {
            /* 故障灯开 */
            FAULT_LED_ON;

            /* 蜂鸣器报警 */
            signal_beeper(10, 1000);

            /* 故障灯关 */
            FAULT_LED_OFF;

            /* Brown-Out Detector 电源电压检测 */
            check_BOD();
            
            /* 打开定时器2 */
            TR2 = 1;

            for (k = 0; k < 1800; k++)
            {
                delay_10us(10);
                /* Brown-Out Detector 电源电压检测 */
                check_BOD();
            }
            break;
        }
        case Alarm_Expired:
        {
            /* 故障灯开 */
            EXPIRED_LED_ON;

            /* 打开定时器2 */
            TR2 = 1;

            for (k = 0; k < 1800; k++)
            {
                delay_10us(10);
                /* Brown-Out Detector 电源电压检测 */
                check_BOD();
            }

            /* 故障灯关 */
            EXPIRED_LED_OFF;
            
            /* 打开定时器2 */
            TR2 = 1;

            for (k = 0; k < 1800; k++)
            {
                delay_10us(10);
                /* Brown-Out Detector 电源电压检测 */
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
        /*  Brown-Out Detector 电源电压检测 */
        check_BOD();
        /* 蜂鸣器状态0 */
        SOUND_STATE0;
        delay_10us(interval);
        i++;

        /*  Brown-Out Detector 电源电压检测 */
        check_BOD();
        /* 蜂鸣器状态1 */
        SOUND_STATE1;
        delay_10us(interval);
        i++;
    } while (i < repeat_time);

    /* 蜂鸣器关闭 */
    SOUND_OFF;
}

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

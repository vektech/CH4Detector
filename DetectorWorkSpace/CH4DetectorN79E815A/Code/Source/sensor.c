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
 *    v1.0.0                 2020年1月1日          Unknown
 *    Modification: 创建文档
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
/* 预热标记 */
bit sensor_preheat_flag = false;

/* 预热时间计数 */
uint16_t sensor_preheat_time_count = 0;

/* 甲烷 零点AD值 */
uint16_t sensor_ch4_0 = 0;
/* 甲烷 3500点AD值 */
uint16_t sensor_ch4_3500 = 712;
/* 标定AD值存储数组 */
uint8_t sensor_demarcation_result[4] = 0;

/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
/* 标定函数 */
/*  1.按键按下 消抖后
    2.电源灯声音闪亮3声,声音与报警和故障声不同，随后进入3分钟预热
    3.开始读取AD，if ((adc_value < 41) || (adc_value > 260))，则认为是故障，故障灯闪亮，标定失败
    4.if (adc_value < 410)，则电源，故障，报警灯全亮，等待。若>=410，则故障、报警灯闪亮100s左右以指示
    5.if ((temp_data < 410)||(temp_data > 950))，则反应值不合法，故障灯亮，标定失败
    6.标定成功，报警灯常亮。进入死循环等待重启
    */
void sersor_demarcation(void)
{
    uint8_t demarcate_key;
    uint16_t i = 0;
    uint16_t adc_value = 0;
    uint16_t temp_data = 0;
    uint16_t delay_count = 0;

    /* 读取 P0.6 的值 sbit P06 = P0 ^ 6 (sbit 和 ^ 操作 指为寄存器某位取的别称 为C51特殊类型 而非异或) */
    demarcate_key = P06 & 0x01;

    /* 调试键按下 */
    if (demarcate_key == 0x00)
    {
        /* 延时去抖 */
        delay_1ms(50);
        /* 中止定时器2->关闭电源灯闪烁 清该位将中止定时器2并将当前计数保存在TH2和TL2 */
        TR2 = 0;
        /* 再次获取P0.6 的值 */
        demarcate_key = P06 & 0x01;
        /* 确认调试键按下 进入标定状态 */
        if (demarcate_key == 0x00)
        {
            /* 标定状态的声光提示 闪亮，响一声 */
            device_alarm(Alarm_Demarcation);
            
            /* 选择ADC0 即P0.1作为ADC采样端口 打开ADC中断使能 */
            /* ZZZ 是否可以只使用 adc_init_interrupt() 而不需要 adc_init() */
            adc_init_interrupt();

            /* 设备预热标记为已预热 */
            sensor_preheat_flag = true;
            
            /* ---- 1.传感器预热 ---- */
            /* 等待 255 * 2 = 510s */
            for (i = 0; i < 225; i++)
            {
                /* 电源灯开 */
                LED_POWER_ON;
                delay_1ms(1500);

                /* 电源灯关 */
                LED_POWER_OFF;
                delay_1ms(500);

                /* 通过按键 测试电磁阀和继电器 */
                /* 获取P0.7 的值 */
                demarcate_key = P07 & 0x01;
                /* P0.7 按键按下 */
                if (demarcate_key == 0x00)
                {
                    /* 延时去抖 */
                    delay_1ms(30);
                    /* 再次获取P0.7 的值 */
                    demarcate_key = P07 & 0x01;
                    /* 确认P0.7 按键按下 */
                    if (demarcate_key == 0x00)
                    {
                        /* 电磁阀开 */
                        VALVE_ON;

                        delay_1ms(1500);

                        /* 继电器开 */
                        DELAY_ON;
                    }
                }
                else
                {
                    /* 继电器关  */
                    DELAY_OFF;
                    /* 电磁阀关 */
                    VALVE_OFF;
                }
            }

            /* 得到ADC采样并滤波之后的值 */
            adc_value = adc_sensor();
            /* 将该采样值存储到sensor_ch4_0中 sensor_ch4_0为0传感器0点的值 */
            sensor_ch4_0 = adc_value;

            /* 串口输出该次采样的值 */
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

            /* 再次得到ADC采样并滤波之后的值 */
            adc_value = adc_sensor();

            /* ZZZ 此处程序结构可以改善 不需要循环全部内容 */
            while (1)
            { 
                /* 得到ADC采样并滤波之后的值 */
                adc_value = adc_sensor();
                
                /* ---- 2.传感器基线判断 ---- */
                /* 传感器基线合理性判断 AD超出范围 0.2V-1.27V 则故障灯亮 */
                if ((adc_value < 41) || (adc_value > 260))
                {
                    delay_1ms(1000);
                    if ((adc_value < 41) || (adc_value > 260))
                    {
                        while (1)
                        {   
                            /* 故障灯开 */
                            LED_FAULT_ON;
                            delay_1ms(500);

                            /* 故障灯关 */
                            LED_FAULT_OFF;
                            delay_1ms(500);
                        }
                    }
                }

                /* ---- 3.根据采集的AD值 等待气体浓度上升 ---- */
                /* 仅当ADC采样值小于410时执行 当采样值大于410时 退出该循环 此举是在等待气体浓度上升至410以上 */
                do
                {
                    /* 再次得到ADC采样并滤波之后的值 */
                    adc_value = adc_sensor();

                    /* <1> 电源 故障 报警 三灯常亮 */
                    /* 电源灯开 */
                    LED_POWER_ON;
                    /* 故障灯开 */
                    LED_FAULT_ON;
                    /* 报警灯开 */
                    LED_ALARM_ON;

                    /* 周期性 串口输出采样的值 */
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

                /* ---- 4.等待气体浓度上升 未进行采集 ---- */
                /* Wait 120s 此举在等待气体浓度上升 */
                for (i = 0; i <= 300; i++)
                {
                    /* <2> 电源灯常亮 报警故障两颗灯闪烁 */
                    delay_1ms(500);

                    /* 报警灯开 */
                    LED_ALARM_ON;
                    /* 故障灯开 */
                    LED_FAULT_ON;

                    delay_1ms(1500);

                    /* 报警灯关 */
                    LED_ALARM_OFF;
                    /* 故障灯关 */
                    LED_FAULT_OFF;

#ifdef _DEBUG_
                    /* 串口输出该次采样的值 */
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

                /* 暂存得到ADC采样并滤波之后的值 */
                temp_data = adc_sensor();

                /* ---- 5.进行报警阈值记录 ---- */
                /* 不合理 AD超出范围 ADC_result < 410 or  ADC_result > 950 */
                if ((temp_data < 410) || (temp_data > 950))
                {
                    delay_1ms(1000);

                    /* 再次得到ADC采样并滤波之后的值 */
                    temp_data = adc_sensor();
                    /* 再次确认 采样值 合理性判断 AD超出范围 ADC_result < 410 or  ADC_result > 950 */
                    if (temp_data < 410 || temp_data > 950)
                    {
                    /* <5> 故障状态 故障灯常亮 */
                    ERROR:
                        while (1)
                        {
                            LED_FAULT_ON;
                        }
                    }
                }
                /* 合理 AD在范围内 410 <= ADC_result <= 950 则标定成功 */
                else
                {
                    /* <3> 电源灯常亮 报警故障两颗关闭 */
                    /* 故障灯关 */
                    LED_FAULT_OFF;
                    /* 报警灯关 */
                    LED_ALARM_OFF;
                    /* 暂存 得到ADC采样并滤波之后的值 */
                    temp_data = adc_sensor();

                    /* 将暂存的ADC采样值 存入sensor_ch4_3500中 sensor_ch4_3500为传感器 3500 ZZZ 的值 */
                    sensor_ch4_3500 = temp_data;

                    /* 将零点和3500点的ADC采样数据存入数组中 */
                    sensor_demarcation_result[0] = sensor_ch4_0;
                    sensor_demarcation_result[1] = sensor_ch4_0 >> 8;
                    sensor_demarcation_result[2] = sensor_ch4_3500;
                    sensor_demarcation_result[3] = sensor_ch4_3500 >> 8;

                    /* 将采样结果数组存入FLASH中 */
                    flash_write_data(sensor_demarcation_result, 4, DEVICE_INFO_ADDR, OFFSET_OF_CH4_0);

                    /* 从I2C时钟芯片中读取时间戳 */
                    i2c_get_time();

                    /* EBO 为BOD电源电压检测的中断使能位 关中断 sbit EBO = IE ^ 5 */
                    EBO = 0;
                    /* 复制从时钟芯片获取的时间戳 */
                    format_to_device_time();
                    /* EBO 为BOD电源电压检测的中断使能位 开中断 sbit EBO = IE ^ 5 */
                    EBO = 1;
                    
                    /* 从Flash中读取 sensor_ch4_0 与 sensor_ch4_3500 数据 并进行比对 使用 i2c_time_code 暂存 */
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
                    /* 发送标定的报警点数据 sensor_ch4_3500 */
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

                /* <4> 标定完成 电源 报警灯常亮 故障灯关闭 */
                while (1)
                {
                    /* 报警灯开 */
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

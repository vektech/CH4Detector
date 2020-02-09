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

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
/*
/*
P2.0 蜂鸣器2
P2.1 蜂鸣器1
P2.3 继电器
P2.4 报警灯
P2.5 故障灯
P2.6 传感器寿命灯

P1.3  SDA
P1.2  SCL
P1.1  RXD
P1.0  TXD
P1.4  T_INT

P0.0  电磁阀
P0.1  传感器AD采样
P0.2  电磁阀检测
P0.3  电源灯
P0.5  电源电压检测
P0.6  调试、标定
P0.7  测试按键
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
/* 设备状态字组 */
uint8_t device_status[2]={0};

#define STATUS1_WARMUP   device_status[1] &= 0x1f
#define STATUS1_NOMAL    device_status[1] =(device_status[1] & 0x1f)|0X20
#define STATUS1_ALARM    device_status[1] =(device_status[1] & 0x1f)|0X40
#define STATUS1_FAULT    device_status[1] =(device_status[1] & 0x1f)|0X60
/*******************************************************************************
 *                 File Static Variable Define Section ('static variable')
 ******************************************************************************/

/*******************************************************************************
 *                 Normal Function Define Section ('function')
 ******************************************************************************/
void main(void)
{
    uint8_t production_date[5] = {0};

    /* 传感器寿命到期检测标志 */
    uint8_t life_check[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t life_check_flag = false;
    
    /* 写传感器寿命到期记录标志 */
    bit write_expired_record_flag = false;

    /* 写传感器报警恢复记录标志 */
    bit write_no_exceeded_record_flag = false;

    /* 写传感器故障恢复记录标志 */
    bit write_no_fault_record_flag = false;

    /* 自检按键 */
    uint8_t selfcheck_key;

    /* 尝试次数计数 */
    uint8_t i;

    /* 传感器采样AD值 */
    uint16_t ch4_adc_value;

    /* 时间更新标志 */
    bit device_time_renew_flag = false;
    /* 报警阈值设定标志 */
    bit device_threshold_flag = false;

    /* ch4 高过阈值的报警标志 */
    bit exceeded_flag = false;

    /* ch4 高过阈值的次数 */
    uint8_t exceeded_count = 0;
    /* ch4 高过阈值时阀操作计数 */
    uint8_t exceeded_valve_count = 0;

    /* ch4 低于最低值的次数 故障标志 */
    bit fault_flag = false;

    /* ch4 低于最低值的次数 故障计数 */
    uint8_t fault_count = 0;

    /* 设备掉电计数 */
    device_power_down_count = 0;
    /* timer2 秒计数 */
    timer2_life_second_count = 0;
    /* 预热标记 */
    sensor_preheat_flag = false;

    /* 设备初始化 */
    device_init();
    
    /* 设备标定 */
    sersor_demarcation();

    /* 上电状态位为false 进行一次上电记录 */
    if (device_first_power_on == false)
    {
        device_first_power_on = true;
        /* YYY 进行上电记录 */
        // WriteRecordData(UPPOWER_RECORD);
    }

    /* WARMUP 设置系统状态 device_status[1] &= 0x1f */
    STATUS1_WARMUP;

    /* 开机长按测试键3s后 取消预热 */
    /* 读取按键 P0.7 的值 检查自检功能是否触发 */
    selfcheck_key = P07 & 0x01;
    /* 测试键按下 */
    if (selfcheck_key == 0x00)
    {   
        /* 延时去抖 */
        delay_1ms(50);
        /* 测试按键按下时长计算 */
        while ((P07 & 0x01) == 0x00)
        {   
            /* 根据定时器 按键按下大于一秒 */
            if (timer2_life_second_count > 1)
            {
                /* NOMAL 设置系统状态 device_status[1] = (device_status[1] & 0x1f) | 0X20 */
                STATUS1_NOMAL;
                sensor_preheat_flag = true;

                /* 控制四颗灯闪烁和蜂鸣器鸣响 */
                device_alarm(Alarm_Selfcheck);
                break;
            }
        }
    }
    /* 定时器秒计数归零 */
    timer2_life_second_count = 0;

    /* ---- 时钟检测 ---- */
    /* YYY 从Flash存储地址(RECORD_FIRST_ADDRESS[LIFE_RECORD] + 30) 中读取寿命到期信息 */
    // ReadData(LifeCheck, RECORD_FIRST_ADDRESS[LIFE_RECORD] + 30, 4);
    /* YYY life_check_flag表示 线上设置了时钟 */
    // if (LifeCheck[0] == 0xa5 && (LifeCheck[1] == 0x36))
    // {
    //     if (LifeCheck[2] == 0x5a && (LifeCheck[3] == 0xe7))
    //     {
                /* YYY */
                life_check_flag = true;
    //     }
    // }

    /* 已预热完成 */
    if (sensor_preheat_flag == true)
    {
        /* 电源灯开 */
        LED_POWER_ON;

        /* 没有从FLASH中读取标定值 */
        if (!device_threshold_flag)
        {
            device_threshold_flag = true;
            /* 尝试从FLASH中读取标定数据 10次 */
            for (i = 0; i < 10; i++)
            {
                /* 开启I2C 并进行时间写入 */
                i2c_start_rtc();
                /* YYY 从FLASH中读取标定数据 */
                // ReadData(sensor_demarcation_result, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD] + Life_start_OFFSET_DEMA_sensor_ch4_0, 4);

                /* 设置 sensor_ch4_0 */
                sensor_ch4_0 = sensor_demarcation_result[1];
                sensor_ch4_0 = sensor_ch4_0 << 8;
                sensor_ch4_0 |= sensor_demarcation_result[0];
                /* 设置 sensor_ch4_3500 */
                sensor_ch4_3500 = sensor_demarcation_result[3];
                sensor_ch4_3500 = sensor_ch4_3500 << 8;
                sensor_ch4_3500 |= sensor_demarcation_result[2];

                /* 如果未标定或者读取的标定数据有误 则设置默认的报警阈值 
                    - sensor_ch4_0 = 150 
                    - sensor_ch4_3500 = 750 
                    - 再次尝试读取
                    */
                if (sensor_ch4_3500 >= 1024 || (sensor_ch4_3500 <= sensor_ch4_0) || (sensor_ch4_0 <= 0))
                {
                    sensor_ch4_0 = 150;
                    sensor_ch4_3500 = 750;
                }
                /* 标定数据无误 跳出 */
                else
                {
                    break;
                }
            }
        }
    }
    
    while (1)
    {
        /* 传感器寿命已到期 */
        if (life_check_flag == false)
        {
            /* 传感器寿命灯开 */
            LED_LIFE_ON;
        }
        
        /* 每两秒执行一次 从RTC更新设备时间 */
        if (!(timer2_life_second_count % 2))
        {
            /* 没有从I2C中读取时间 */
            if (!device_time_renew_flag)
            {   
                /* 设备时间已更新 */
                device_time_renew_flag = true;
                /* Brown-Out Detector 电源电压检测 */
                check_BOD();
                /* 读取当前时间 存入Time_Code */
                i2c_get_time();
                /* XXX */
                if (i2c_time_code[6] < 15 || (i2c_time_code[5] > 12) || (i2c_time_code[3] > 31) || (i2c_time_code[2] > 23) || (i2c_time_code[1] > 59) || (i2c_time_code[0] > 59))
                {
                    /* XXX 将 i2c_time_code 转换为 time_data */
                    format_to_RTC_time();
                }
                else
                {
                    /* EBO 为BOD电源电压检测的中断使能位 关中断 sbit EBO = IE ^ 5 */
                    EBO = 0;
                    /* XXX 复制从时钟芯片获取的时间戳 将 time_data 转换为 i2c_time_code */
                    format_to_device_time();
                    /* EBO 为BOD电源电压检测的中断使能位 开中断 sbit EBO = IE ^ 5 */
                    EBO = 1;
                }
            }
        }
        else
        {
            device_time_renew_flag = 0;
        }

        /* 读取按键 P0.7 的值 检查自检功能是否触发 */
        selfcheck_key = P07 & 0x01;
        /* 测试键按下 */
        if (selfcheck_key == 0x00)
        {
            /* 延时去抖 */
            delay_1ms(30);
            /* 确认测试键按下 */
            if (selfcheck_key == 0x00)
            {
                /* 测试键长按标志未置位 */
                if (!key_long_press_flag)
                {
                    timer2_key_long_press_count = 0;
                    /* 置位测试键长按标志 */
                    key_long_press_flag = true;
                }
                /* 这里等于6 通过实际测试出延时时间为11s 国标规定测试键按下后 在7~30s内 应触发继电器 阀 */
                if (timer2_key_long_press_count >= 6)
                {
                    /* 阀检测 当阀关闭时 */
                    if (!device_valve_state)
                    {
                        device_valve_state = true;

                        /* 电磁阀开 */
                        VALVE_ON;
                        delay_1ms(30);

                        /* 电磁阀关 */
                        VALVE_OFF;
                        delay_1ms(30);
                    }
                    /* 继电器开 */
                    DELAY_ON;
                    timer2_key_long_press_count = 8;
                }
                device_alarm(Alarm_Selfcheck);
            }
        }
        /* 测试键未按下 */
        else
        {
            /* 测试键长按标志已置位 */
            if (key_long_press_flag)
            {
                /* 延时去抖 */
                delay_1ms(30);
                /* 确认测试键未按下 */
                if (selfcheck_key != 0x00)
                {
                    /* 测试键长按标志清除 */
                    key_long_press_flag = false;
                    /* 阀状态标志清除 */
                    device_valve_state = false;
                    /* 传感器未预热完成 */
                    if (!sensor_preheat_flag)
                    {   
                        /* 继电器关 */
                        DELAY_OFF;
                    }
                }
            }
        }

        /* 1小时时间到就进行一次寿命比较 */
        if (timer2_life_hour_flag == true)
        {
            timer2_life_hour_flag = false;

            /* Brown-Out Detector 电源电压检测 */
            check_BOD();
            /* 读取当前时间 存入Time_Code */
            i2c_get_time();

            /* YYY 从Flash中读取生产日期 */
            // ReadData(production_date, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 5);
            /* 读取生产日期失败 则设置默认生产日期为 15年12月31日23时59分 */
            if (production_date[0] >= 255 || (production_date[1] >= 255) || (production_date[2] >= 255))
            {
                production_date[0] = 15;
                production_date[1] = 12;
                production_date[2] = 31;
                production_date[3] = 23;
                production_date[4] = 59;
            }

            /* 进行读取的当前时间和记录的生产日期的比较 */
            /* 当前时间的年 大于 生产日期中的年 */
            if (i2c_time_code[6] >= production_date[0])
            {
                /* 计算年差 */
                production_date[0] = i2c_time_code[6] - production_date[0];
                /* 年差 已经到达五年 如 2020 - 2015 = 5 */
                if (production_date[0] == SENSOR_LIFE)
                {
                    /* 当前时间的月 大于 生产日期中的月 2020.6 > 2015.5 */
                    if (i2c_time_code[5] > production_date[1])
                    {
                        /* 已使用年数 加1 变为6 */
                        production_date[0]++;
                    }
                    /* 当前时间的月 等于 生产日期中的月 2020.5 = 2015.5 */
                    if (i2c_time_code[5] == production_date[1])
                    {
                        /* 当前时间的日 大于 生产日期中的日 2020.5.6 > 2015.5.5 */
                        if (i2c_time_code[3] > production_date[2])
                        {
                            /* 已使用年数 加1 变为6 */
                            production_date[0]++;
                        }
                        /* 当前时间的日 等于 生产日期中的日 2020.5.5 = 2015.5.5 */
                        if (i2c_time_code[3] == production_date[2])
                        {
                            /* 以此类推比较时 */
                            if (i2c_time_code[2] > production_date[3])
                            {
                                /* 已使用年数 加1 变为6 */
                                production_date[0]++;
                            }
                            /* 时相等 */
                            if (i2c_time_code[2] == production_date[3])
                            {
                                /* 以此类推比较分 */
                                if (i2c_time_code[1] > production_date[4])
                                {
                                    /* 已使用年数 加1 变为6 */
                                    production_date[0]++;
                                }
                            }
                        }
                    }
                }

                /* 使用寿命超时 5年时间到 */
                if (production_date[0] > SENSOR_LIFE)
                {
                    /* 传感器寿命灯开 */
                    LED_LIFE_ON;
                    
                    /* 写失效记录标志 */
                    if (write_expired_record_flag == false)
                    {
                        write_expired_record_flag = true;

                        // /* YYY 从FLASH中读取 寿命到期存储地址 的第一个字节 1代表失效，0代表未失效 */
                        // ReadData(production_date, RECORD_FIRST_ADDRESS[LIFE_RECORD], 1);
                        // /* YYY 该字节为0 或者 大于应存的到期记录1条 */
                        // if (production_date[0] == 0 || (production_date[0] > RecordLEN[LIFE_RECORD]))
                        // {
                        //     /* Brown-Out Detector 电源电压检测 */
                        //     check_BOD();
                        //     /* 写寿命到期记录 */
                        //     WriteRecordData(LIFE_RECORD);
                        // }
                    }
                }
                else
                {
                    if (life_check_flag == true)
                    {
                        /* 传感器寿命灯关 */
                        LED_LIFE_OFF;
                    }
                }
            }
        }

        /* 得到ADC采样并滤波之后的值 */
        ch4_adc_value = adc_sensor();

        /* 得到ADC采样并滤波之后的值 超过 sensor_ch4_3500 的报警阈值 报警状态处理 */
        if (ch4_adc_value >= sensor_ch4_3500)
        {
            /* Brown-Out Detector 电源电压检测 */
            check_BOD();
            /* 预热完成 */
            if (sensor_preheat_flag)
            {
                /* 设置为报警状态 */
                STATUS1_ALARM;
                /* 设置P0.0为推挽输出模式 */
                P0M1 = 0x00;
                P0M2 = 0x01;

                /* 每当定时器计时一秒 */
                if (timer2_second_flag == true)
                {
                    timer2_second_flag = false;
                    /* ch4 高过阈值的次数 */
                    exceeded_count++;

                    /* ch4 高过阈值的次数等于 3 */
                    if (exceeded_count == 3)
                    {
                        exceeded_flag = true;
                    }

                    /* 报警次数大于 5 */
                    if (exceeded_count > 5)
                    {
                        exceeded_count = 5;
                        /* 继电器开 */
                        DELAY_ON;
                        /* 阀操作执行的次数为8的倍数 */
                        if (!(exceeded_valve_count % 8))
                        {
                            /* 电磁阀开 */
                            VALVE_ON;
                            delay_1ms(30);

                            /* 电磁阀关 */
                            VALVE_OFF;
                            delay_1ms(30);
                        }
                        exceeded_valve_count++;
                    }
                }

                /* ch4 高过阈值的报警标志置位 */
                if (exceeded_flag == true)
                {
                    exceeded_flag = false;
                    /* 写传感器恢复记录标志为真 如果传感器恢复则可以写恢复记录 */
                    write_no_exceeded_record_flag = true;
                    /* Brown-Out Detector 电源电压检测 */
                    check_BOD();
                    /* YYY 向FLASH中 写报警记录 */
                    // WriteRecordData(ALARM_RECORD);
                }

                /* 报警次数大于3等于 */
                if (exceeded_count >= 3)
                {
                    /* 发出报警声光信号 */
                    device_alarm(Alarm_CH4_Exceeded);
                }
            }
        }
        /* 得到ADC采样并滤波之后的值 小于 Short_Fault_L 的故障阈值 故障状态处理 */
        else if (ch4_adc_value <= Short_Fault_L)
        {
            /* Brown-Out Detector 电源电压检测 */
            check_BOD();

            /* 预热完成 */
            if (sensor_preheat_flag)
            {
                /* 设置系统状态为故障状态 */
                STATUS1_FAULT;

                exceeded_count = 0;
                exceeded_valve_count = 0;

                /* 报警灯关 */
                LED_ALARM_OFF;
                /* 继电器关 */
                DELAY_OFF;

                /* 进行故障计数 */
                fault_count++;
                if (fault_count == 1)
                {
                    fault_flag = true;
                }
                if (fault_count > 2)
                {
                    fault_count = 2;
                }

                /* 故障声光信号 */
                device_alarm(Alarm_Fault);

                /* ch4 低于最小值的故障标志置位 */
                if (fault_flag == true)
                {
                    fault_flag = false;
                    /* 写传感器故障恢复记录标志为真 如果传感器从故障恢复则可以写恢复记录 */
                    write_no_fault_record_flag = 1;
                    /* Brown-Out Detector 电源电压检测 */
                    check_BOD();
                    /* YYY 向FLASH中 写故障记录 */
                    // WriteRecordData(FAULT_RECORD);
                }
            }
        }
        /* 得到ADC采样并滤波之后的值 大于 Short_Fault_L 的故障阈值 小于 sensor_ch4_3500 的报警阈值 正常状态处理 */
        else
        {   
            /* 如果预热完成 */
            if (sensor_preheat_flag)
            {
                STATUS1_NOMAL;
            }
            /* Brown-Out Detector 电源电压检测 */
            check_BOD();

            /* ch4 超过阈值报警相关标志和计数清零 */
            exceeded_flag = false;
            exceeded_count = 0;
            exceeded_valve_count = 0;

            /* ch4 低于最低值故障报警相关标志和计数清零 */
            fault_flag = 0;
            fault_count = 0;

            /* 未写传感器报警恢复记录 */
            if (write_no_exceeded_record_flag == true)
            {
                write_no_exceeded_record_flag = false;
                /* Brown-Out Detector 电源电压检测 */
                check_BOD();
                /* YYY 向FLASH中 写报警恢复记录 */
                // WriteRecordData(ALARM_BACK_RECORD);
            }

            /* 未写传感器故障恢复记录 */
            if (write_no_fault_record_flag == 1)
            {
                write_no_fault_record_flag = 0;
                fault_flag = 0;

                /* Brown-Out Detector 电源电压检测 */
                check_BOD();
                /* YYY向FLASH中 写故障恢复记录 */
                // WriteRecordData(FAULT_BACK_RECORD);
            }

            /* 报警灯关 */
            LED_ALARM_OFF;
            /* 故障灯关 */
            LED_FAULT_OFF;

            /* 蜂鸣器1关 */
            SOUND1_OFF;
            /* 蜂鸣器2关 */
            SOUND2_OFF;
            /* 电磁阀关 */
            VALVE_OFF;

            /* 当阀状态为关闭时 */
            if (!device_valve_state)
            {
                /* 继电器关 */
                DELAY_OFF;
            }
        }

        // while (1)
        // {
        //     device_alarm(Alarm_Selfcheck);
        //     uart_send(0xAA);
        //     delay_1ms(3000);
        // }

        // /* Brown-Out Detector 电源电压检测 */
        // check_BOD();

        // /* YYY 已在设备初始化中进行了 ADC初始化 所以此处注释掉 */
        // // adc_init();

        /* ---- UART 通讯程序段 ----*/
        /* UART接收完成 */
        // if (rx_finished)
        // {   
        //     /* 取第一个字节的最高一位 如果为0B 表示是工装板发来的命令 */
        //     if (!(uart_buffer[0] & 0x80))
        //     {
        //         if (rx_index >= 9)
        //         {
        //             if (Get_crc(uart_buffer, rx_index) == uart_buffer[rx_index - 2])
        //             {
        //                 for (i = 0; i < 5; i++)
        //                 {
        //                     if (SERIAL_ADD[i + 3] != uart_buffer[i + 1])
        //                         break;
        //                 }
        //                 /* 表示地址对上了 */
        //                 if (i == 5)
        //                 {
        //                     for (i = 6; i < rx_index; i++)
        //                     {
        //                         /* 把地址信息从接收帧中去掉 */
        //                         uart_buffer[i - 5] = uart_buffer[i];
        //                     }
        //                     /* 把地址的字节数减去 */
        //                     rx_index -= 5;
        //                     /* 把命令恢复 */ 
        //                     uart_buffer[0] |= 0x80;
        //                     /* 重新计算crc */
        //                     uart_buffer[rx_index - 2] = Get_crc(uart_buffer, rx_index);
        //                 }
        //                 else
        //                 {
        //                     uart_buffer[0] = 0x00;
        //                 }
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

        //     switch (uart_buffer[0])
        //     {
        //     /* 0xab开头为写命令 */
        //     case 0xab:
        //     {
        //         switch (uart_buffer[1])
        //         {
        //         /* 擦除EEP */
        //         case 0x00:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_EASE_REC[0]) == uart_buffer[COMMAND_LEN_EASE_REC[0] - 2])
        //             {
        //                 for (i = 1; i <= 8; i++)
        //                 {
        //                     ease_page(RECORD_FIRST_ADDRESS[i]);
        //                     /* Brown-Out Detector 电源电压检测 */
        //                     check_BOD();
        //                 }

        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //             }
        //             uart_buffer[2] = device_status[0];
        //             uart_buffer[COMMAND_LEN_EASE_REC[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_EASE_REC[1]);
        //             uart_buffer[COMMAND_LEN_EASE_REC[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_EASE_REC[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }

        //             break;
        //         }
        //         /* 写时钟指令 */
        //         case 0x01:
        //         {
        //             /* 写时钟指令 */
        //             if ((rx_index >= COMMAND_LEN_WRITE_CLOCK[0]))
        //             {
        //                 if (Get_crc(uart_buffer, COMMAND_LEN_WRITE_CLOCK[0]) == uart_buffer[COMMAND_LEN_WRITE_CLOCK[0] - 2])
        //                 {
        //                     if (uart_buffer[2] < 15)
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
        //                         goto WR_CLOCK_EROR;
        //                     }
        //                     if (uart_buffer[3] > 12 || (!uart_buffer[3]))
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
        //                         goto WR_CLOCK_EROR;
        //                     }
        //                     if (uart_buffer[4] > 31 || (!uart_buffer[4]))
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
        //                         goto WR_CLOCK_EROR;
        //                     }

        //                     if ((uart_buffer[5] > 23) || (uart_buffer[6] > 59) || (uart_buffer[7] > 59))
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
        //                         goto WR_CLOCK_EROR;
        //                     }

        //                     for (i = 2; i < 8; i++)
        //                     {
        //                         /* 十六进制转为BCD码 */
        //                         i2c_time_code[8 - i] = hex2bcd(uart_buffer[i]);
        //                     }
        //                     i2c_time_code[0] = i2c_time_code[1];
        //                     i2c_time_code[1] = i2c_time_code[2];
        //                     i2c_time_code[2] = i2c_time_code[3];
        //                     i2c_time_code[3] = i2c_time_code[4];
        //                     i2c_time_code[4] = 0;

        //                     /* 写入数据 */
        //                     Master_Write_Data();

        //                     delay_1ms(100);
        //                     i2c_get_time();
        //                     /* 月：表示写入时间不成功 */
        //                     if (i2c_time_code[5] != uart_buffer[3] && (i2c_time_code[5] != (uart_buffer[3] + 1)))
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                         ;
        //                     }
        //                     else
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0);
        //                     }
        //                     /* 年：表示写入时间不成功 */
        //                     if (i2c_time_code[6] != uart_buffer[2] && (i2c_time_code[6] != (uart_buffer[2] + 1)))
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                     }
        //                     /* 日：表示写入时间不成功 */
        //                     if (i2c_time_code[3] != uart_buffer[4] && (i2c_time_code[3] != (uart_buffer[4] + 1)))
        //                     {
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                     }
        //                     /* 时：表示写入时间不成功 */
        //                     if (i2c_time_code[2] != uart_buffer[5] && (i2c_time_code[2] != (uart_buffer[5] + 1)))
        //                     {
        //                         if (!((uart_buffer[5] == 23) && (i2c_time_code[2] == 0)))
        //                             uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                     }
        //                     /* 分：表示写入时间不成功 */
        //                     if (i2c_time_code[1] != uart_buffer[6] && (i2c_time_code[1] != (uart_buffer[6] + 1)))
        //                     {
        //                         if (!((uart_buffer[6] == 59) && (i2c_time_code[1] == 0)))
        //                             uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                     }
        //                 }
        //                 else
        //                 {
        //                     uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //                     ;
        //                 }
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | BYTELOSS_EROR;
        //             }
        //         WR_CLOCK_EROR:
        //             uart_buffer[2] = device_status[0];
        //             uart_buffer[COMMAND_LEN_WRITE_CLOCK[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_WRITE_CLOCK[1]);
        //             uart_buffer[COMMAND_LEN_WRITE_CLOCK[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_WRITE_CLOCK[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出  延时函数，关键参数 */
        //                 delay_1ms(5);
        //             }

        //             /* 表示时钟写入成功 没有发生错误 */
        //             if ((uart_buffer[3] & (~0xe0)) == 0)
        //             {
        //                 LifeCheck[0] = 0xa5;
        //                 LifeCheck[1] = 0x36;
        //                 life_check_flag = 1;
        //                 WriteData(LifeCheck, 4, RECORD_FIRST_ADDRESS[LIFE_RECORD], 30);
        //             }
        //             break;
        //         }
        //         /* 写传感器寿命的超始时间 即出厂日期 */
        //         case 0x02:
        //         {
        //             if ((rx_index >= COMMAND_LEN_WR_DATEOFPRODUCTION[0])) 
        //             {
        //                 if (Get_crc(uart_buffer, COMMAND_LEN_WR_DATEOFPRODUCTION[0]) == uart_buffer[COMMAND_LEN_WR_DATEOFPRODUCTION[0] - 2]) //
        //                 {
        //                     WriteData(&uart_buffer[2], 5, RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 0);
        //                     ReadData(&uart_buffer[7], RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 5);
        //                     if (uart_buffer[8] != uart_buffer[3])
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                     else
        //                         uart_buffer[3] = (device_status[1] & 0xe0);
        //                     if (uart_buffer[7] != uart_buffer[2])
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                     if (uart_buffer[9] != uart_buffer[4])
        //                         uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
        //                 }
        //                 else
        //                 {
        //                     uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //                 }
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | BYTELOSS_EROR;
        //             }
        //             uart_buffer[2] = device_status[0];
        //             uart_buffer[COMMAND_LEN_WR_DATEOFPRODUCTION[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_WR_DATEOFPRODUCTION[1]);
        //             uart_buffer[COMMAND_LEN_WR_DATEOFPRODUCTION[1] - 1] = 0x55;

        //             for (i = 0; i < COMMAND_LEN_WR_DATEOFPRODUCTION[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             /* 表示出厂日期写入成功 */
        //             if ((uart_buffer[3] & (~0xe0)) == 0)
        //             {
        //                 LifeCheck[2] = 0x5a;
        //                 LifeCheck[3] = 0xe7;
        //                 life_check_flag = 1;
        //                 WriteData(LifeCheck, 4, RECORD_FIRST_ADDRESS[LIFE_RECORD], 30);
        //             }
        //             break;
        //         }
        //         default:
        //         {
        //             uart_buffer[2] = device_status[0];
        //             uart_buffer[3] = (device_status[1] & 0xe0) | COMMAND_EROR;
        //             uart_buffer[4] = Get_crc(uart_buffer, 6);
        //             uart_buffer[5] = 0x55;
        //             for (i = 0; i < 6; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         }
        //         break;
        //     }
        //     /* 0xaa开头为 国标规定的协议 读命令 */
        //     case 0xaa:
        //     {
        //         /* 检测查询的记录类型 应小于0x09 */
        //         if (uart_buffer[2] <= GBCOMMANDMAX)
        //         {
        //             /* 所查询的索引号小于该类型的记录总数 */
        //             if ((uart_buffer[1] <= RecordLEN[uart_buffer[2]]))
        //             {
        //                 /* 若检查校验和通过 */
        //                 if ((rx_index >= (READFrameLEN - 1)) && uart_buffer[4] == Get_crc(uart_buffer, READFrameLEN))
        //                 {
        //                     ReadRecordData(uart_buffer[2], uart_buffer[1]);
        //                 }
        //             }
        //         }
        //         break;
        //     }
        //     /* 0xac开头 为读命令 */
        //     case 0xac:
        //     {
        //         switch (uart_buffer[1])
        //         {
        //         /* 读时间 带秒 */
        //         case 0x01:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_RE_CLOCK[0]) == uart_buffer[COMMAND_LEN_RE_CLOCK[0] - 2])
        //             {
        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //             }
        //             uart_buffer[2] = device_status[0];
        //             i2c_get_time();
        //             uart_buffer[4] = i2c_time_code[6];
        //             uart_buffer[5] = i2c_time_code[5];
        //             uart_buffer[6] = i2c_time_code[3];
        //             uart_buffer[7] = i2c_time_code[2];
        //             uart_buffer[8] = i2c_time_code[1];
        //             uart_buffer[9] = i2c_time_code[0];
        //             uart_buffer[COMMAND_LEN_RE_CLOCK[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_RE_CLOCK[1]);
        //             uart_buffer[COMMAND_LEN_RE_CLOCK[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_RE_CLOCK[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }

        //         /* 读模组型号等信息 */
        //         case 0x02:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_RE_MODEL[0]) == uart_buffer[COMMAND_LEN_RE_MODEL[0] - 2])
        //             {
        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //             }
        //             uart_buffer[2] = device_status[0];

        //             uart_buffer[4] = 0x01;
        //             uart_buffer[5] = 0;
        //             uart_buffer[6] = 0;
        //             uart_buffer[7] = 0;
        //             uart_buffer[8] = 0;
        //             uart_buffer[9] = 0;
        //             uart_buffer[COMMAND_LEN_RE_MODEL[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_RE_MODEL[1]);
        //             uart_buffer[COMMAND_LEN_RE_MODEL[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_RE_MODEL[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         /* 读AD */
        //         case 0x03:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_RE_CURTAD[0]) == uart_buffer[COMMAND_LEN_RE_CURTAD[0] - 2])
        //             {
        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //             }
        //             uart_buffer[2] = device_status[0];

        //             uart_buffer[4] = ch4_adc_value >> 8;
        //             uart_buffer[5] = ch4_adc_value;
        //             uart_buffer[COMMAND_LEN_RE_CURTAD[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_RE_CURTAD[1]);
        //             uart_buffer[COMMAND_LEN_RE_CURTAD[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_RE_CURTAD[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         /* 读标定相关信息 */
        //         case 0x04:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_RE_DEMA[0]) == uart_buffer[COMMAND_LEN_RE_DEMA[0] - 2])
        //             {
        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //             }
        //             uart_buffer[2] = device_status[0];
        //             ReadData(&uart_buffer[4], RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD] + Life_start_OFFSET_DEMA_sensor_ch4_0, 4);
        //             uart_buffer[8] = uart_buffer[4];
        //             uart_buffer[4] = uart_buffer[5];
        //             uart_buffer[5] = uart_buffer[8];
        //             uart_buffer[8] = uart_buffer[6];
        //             uart_buffer[6] = uart_buffer[7];
        //             uart_buffer[7] = uart_buffer[8];
        //             uart_buffer[COMMAND_LEN_RE_DEMA[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_RE_DEMA[1]);
        //             uart_buffer[COMMAND_LEN_RE_DEMA[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_RE_DEMA[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         /* 读传感器寿命的超始时间 即出厂日期 */
        //         case 0x05:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_RE_DATEOFPRODUCTION[0]) == uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[0] - 2])
        //             {
        //                 uart_buffer[2] = device_status[0];
        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //             }
        //             else
        //             {
        //                 uart_buffer[2] = device_status[0];
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //             }
        //             ReadData(&uart_buffer[4], RECORD_FIRST_ADDRESS[LIFE_START_DATE_RECORD], 5);
        //             uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_RE_DATEOFPRODUCTION[1]);
        //             uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_RE_DATEOFPRODUCTION[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         /* 读序列号 */
        //         case 0x06:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_RE_SERIALNUM[0]) == uart_buffer[COMMAND_LEN_RE_SERIALNUM[0] - 2])
        //             {
        //                 uart_buffer[2] = device_status[0];
        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //             }
        //             else
        //             {
        //                 uart_buffer[2] = device_status[0];
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //             }

        //             for (i = 0; i < 8; i++)
        //             {
        //                 uart_buffer[4 + i] = SERIAL_ADD[i];
        //             }
        //             uart_buffer[COMMAND_LEN_RE_SERIALNUM[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_RE_SERIALNUM[1]);
        //             uart_buffer[COMMAND_LEN_RE_SERIALNUM[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_RE_SERIALNUM[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         default:
        //         {

        //             uart_buffer[2] = device_status[0];
        //             uart_buffer[3] = (device_status[1] & 0xe0) | COMMAND_EROR;
        //             uart_buffer[4] = Get_crc(uart_buffer, 6);
        //             uart_buffer[5] = 0x55;
        //             for (i = 0; i < 6; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         }
        //         break;
        //     }
        //     case 0xad:
        //     {
        //         switch (uart_buffer[1])
        //         {
        //         /* 取消预热 */
        //         case 0x01:
        //         {
        //             if (Get_crc(uart_buffer, COMMAND_LEN_CANCEL_WARMUP[0]) == uart_buffer[COMMAND_LEN_CANCEL_WARMUP[0] - 2])
        //             {
        //                 STATUS1_NOMAL;
        //                 uart_buffer[3] = device_status[1] & 0xe0;
        //                 sensor_preheat_flag = 1;
        //             }
        //             else
        //             {
        //                 uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
        //             }
        //             uart_buffer[2] = device_status[0];
        //             uart_buffer[COMMAND_LEN_CANCEL_WARMUP[1] - 2] = Get_crc(uart_buffer, COMMAND_LEN_CANCEL_WARMUP[1]);
        //             uart_buffer[COMMAND_LEN_CANCEL_WARMUP[1] - 1] = 0x55;
        //             for (i = 0; i < COMMAND_LEN_CANCEL_WARMUP[1]; i++)
        //             {
        //                 uart_send(uart_buffer[i]);
        //                 /* 串口输出 延时函数 关键参数 */
        //                 delay_1ms(5);
        //             }
        //             break;
        //         }
        //         default:
        //         {
        //             break;
        //         }
        //         }
        //         break;
        //     }
        //     default:
        //     {
        //         break;
        //     }
        //     }
        //     rx_index = 0;
        //     rx_finished = 0;

        //     /* 使能UART0接收 */
        //     UART0_RX_ENABLE;
        // }
    }
}

/*******************************************************************************
 *                 File Static Function Define Section ('static function')
 ******************************************************************************/

/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

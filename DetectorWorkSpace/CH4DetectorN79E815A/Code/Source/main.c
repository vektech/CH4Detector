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
#include "utilities.h"
#include "adc.h"
#include "flash.h"

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

/*  
    下标0：查询各类记录总数;
    下标1：报警记录 200;
    下标2：报警恢复记录 200;
    下标3：故障记录 100;
    下标4：故障恢复记录 100;
    下标5：掉电记录 50;
    下标6：上电记录 50;
    下标7：传感器失效记录 1;
    下标8：内部定时器当前时间
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

#define NORMAL 0
#define Unknown_EROR 1
#define CHECKSUM_EROR 2
#define BYTELOSS_EROR 3
#define ILLEGAL_PARA_EROR 4
#define COMMAND_EROR 5

/* 产品序列号地址 0x31f0 处的数据 */
code uint8_t SERIAL_ADD[8] _at_ 0x31f0;

code uint8_t COMMAND_LEN_EASE_REC[2]={4,6};
code uint8_t COMMAND_LEN_WRITE_CLOCK[2]={10,6};
code uint8_t COMMAND_LEN_WR_DATEOFPRODUCTION[2]={9,6};
code uint8_t COMMAND_LEN_RE_DATEOFPRODUCTION[2]={4,11};
code uint8_t COMMAND_LEN_RE_SERIALNUM[2]={4,14};
code uint8_t COMMAND_LEN_RE_CLOCK[2]={4,12};
code uint8_t COMMAND_LEN_RE_MODEL[2]={4,12};
code uint8_t COMMAND_LEN_RE_CURTAD[2]={4,8};
code uint8_t COMMAND_LEN_RE_CURTSTATUS[2]={4,10};
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
    uint8_t expired_date[5] = {0};

    /* 已设置RTC标志 */
    bit set_rtc_flag = false;
    /* 已设置出厂日期标志 */
    bit set_production_date_flag = false;

    /* RTC和出厂日期存储标志 */
    uint8_t life_check[4] = {0x00, 0x00, 0x00, 0x00};

    /* 写传感器寿命到期记录标志 */
    bit write_expired_record_flag = false;

    /* 写传感器报警恢复记录标志 */
    bit write_no_exceeded_record_flag = false;

    /* 写传感器故障恢复记录标志 */
    bit write_no_fault_record_flag = false;

    /* 按键长按标志 */
    bit key_long_press_flag = false;

    /* 自检按键 */
    uint8_t selfcheck_key;

    /* 长按计数 */
    uint8_t key_long_press_count;
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

    /* 从Flash存储地址 RTC_RECORD_ADDR 中读取时钟和出厂日期 写入标志 */
    flash_read_data(life_check, RTC_RECORD_ADDR, 4);
    /* 设置了RTC时钟 和 出厂日期 */
    if ((life_check[0] == 0xa5) && 
        (life_check[1] == 0x36) && 
        (life_check[2] == 0x5a) && 
        (life_check[3] == 0xe7))
    {
        set_rtc_flag = true;
        set_production_date_flag = true;
    }

    /* 设备标定 */
    sersor_demarcation();

    /* 上电状态位为false 进行一次上电记录 */
    if (device_first_power_on == false)
    {
        device_first_power_on = true;
        /* 进行上电记录 */
        flash_write_record(POWER_ON_RECORD);
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
        delay_1ms(30);
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
    
    while (1)
    {
        /* Brown-Out Detector 电源电压检测 */
        check_BOD();

        /* 更新缓存设备时间 检查是否已写入RTC和出厂日期 传感器寿命过期信号执行 */
        if (timer2_second_expired_flag == true)
        {
            timer2_second_expired_flag = false;

            /* 读取当前时间 存入Time_Code */
            i2c_get_time();
            /* 读取的时间非法 */
            if (i2c_time_code[6] > 99 || (i2c_time_code[5] > 12) || (i2c_time_code[3] > 31) || (i2c_time_code[2] > 23) || (i2c_time_code[1] > 59) || (i2c_time_code[0] > 59))
            {
                /* 向RTC写入默认日期 */
                /* 19 年 */
                uart_buffer[2] = 19;
                /* 12 月 */
                uart_buffer[3] = 12;
                /* 31 日 */
                uart_buffer[4] = 31;
                /* 0 时、0 分、0 秒 */
                uart_buffer[5] = 0;
                uart_buffer[6] = 0;
                uart_buffer[7] = 0;

                for (i = 2; i < 8; i++)
                {
                    /* 十六进制转为BCD码 */
                    i2c_time_code[8 - i] = hex2bcd(uart_buffer[i]);
                }

                i2c_time_code[0] = i2c_time_code[1];
                i2c_time_code[1] = i2c_time_code[2];
                i2c_time_code[2] = i2c_time_code[3];
                i2c_time_code[3] = i2c_time_code[4];
                i2c_time_code[4] = 0;

                /* 写入数据 */
                i2c_set_time();

                delay_1ms(100);
            }
            else
            {
                /* EBO 为BOD电源电压检测的中断使能位 关中断 sbit EBO = IE ^ 5 */
                EBO = 0;
                /* 复制从时钟芯片获取的时间戳 将 i2c_time_code 存储至 time_data */
                store_device_time();
                /* EBO 为BOD电源电压检测的中断使能位 开中断 sbit EBO = IE ^ 5 */
                EBO = 1;
            }

            /* 未写出厂日期和RTC */
            if ((set_rtc_flag == false) || (set_production_date_flag == false))
            {
                /* 传感器寿命灯开 */
                LED_LIFE_ON;
            }
            /* 若传感器寿命已过期 */
            else if (sensor_expired_flag == true)
            {
                /* 传感器寿命灯翻转 */
                LED_LIFE_TOGGLE;
            }
            else
            {
                /* 传感器寿命灯关 */
                LED_LIFE_OFF;
            }
        }
        
        /* 已预热完成 没有从FLASH中读取标定值 尝试从FLASH中读取标定数据 */
        if (sensor_preheat_flag == true)
        {
            /* AAA Brown-Out Detector 电源电压检测 */
            check_BOD();
            
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
                    /* 从FLASH中读取标定数据 ch4_0 & ch4_3500 因两个标定数据地址相连共 4 bytes */
                    flash_read_data(sensor_demarcation_result, ADC_VALUE_OF_CH4_0_ADDR, 4);

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

        /* 写了厂日期和RTC 1小时时间到就进行一次寿命比较 ZZZ */
        if ((set_rtc_flag == true) && (set_production_date_flag == true))
        {
            /* 1小时时间到就进行一次寿命比较 */
            if (timer2_life_hour_flag == true)
            {
                timer2_life_hour_flag = false;

                /* Brown-Out Detector 电源电压检测 */
                check_BOD();
                /* 读取当前时间 存入Time_Code */
                i2c_get_time();

                /* 从Flash中读取生产日期 */
                flash_read_data(production_date, PRODUCTION_DATE_ADDR, 5);
                /* 读取生产日期失败 则设置默认生产日期为 19年12月31日23时59分 */
                if (production_date[0] >= 255 || (production_date[1] >= 255) || (production_date[2] >= 255))
                {
                    /* 时间写入错误需要重新写时间 */
                    set_production_date_flag = false;
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
                                    if (i2c_time_code[1] >= production_date[4])
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
                        /* 传感器寿命到期标志置位 */
                        sensor_expired_flag = true;

                        /* 写失效记录标志 */
                        if (write_expired_record_flag == false)
                        {
                            write_expired_record_flag = true;

                            /* 从FLASH中读取 寿命到期存储地址 的第一个字节 0x01代表失效 0xFF代表未失效 使用 production_date[0] 暂存 */
                            flash_read_data(production_date, SENSOR_EXPIRED_RECORD_ADDR, 1);
                            /* 该字节非0x01 表示未写过失效记录 */
                            if (production_date[0] != 0x01)
                            {
                                /* Brown-Out Detector 电源电压检测 */
                                check_BOD();
                                /* 写寿命到期记录 */
                                production_date[0] = 0x01;
                                flash_write_data(production_date, 1, DEVICE_INFO_ADDR, OFFSET_OF_SENSOR_EXPIRED);

                                /* 当前时间 年 */
                                production_date[0] = i2c_time_code[6];
                                /* 当前时间 年 */
                                production_date[1] = i2c_time_code[5];
                                /* 当前时间 年 */
                                production_date[2] = i2c_time_code[3];
                                /* 当前时间 年 */
                                production_date[3] = i2c_time_code[2];
                                /* 当前时间 年 */
                                production_date[4] = i2c_time_code[1];
                                /* 方式1 写寿命到期时间 */
                                flash_write_data(production_date, 5, DEVICE_INFO_ADDR, OFFSET_OF_SENSOR_EXPIRED + 1);
                            }
                        }
                    }
                    else
                    {
                        /* 传感器寿命到期标志复位 */
                        sensor_expired_flag = false;
                    }
                }
            }
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
                if (key_long_press_flag == false)
                {
                    /* 置位测试键长按标志 */
                    key_long_press_flag = true;

                    /* 自检计数清零 */
                    key_long_press_count = 0;
                }
                else
                {
                    key_long_press_count++;
                }

                /* 这里等于8 通过实际测试出延时时间为8.7s 国标规定测试键按下后 在7~30s内 应触发继电器 阀 */
                if (key_long_press_count >= 8)
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
                    key_long_press_count = 8;
                }
                device_alarm(Alarm_Selfcheck);
            }
        }
        /* 测试键未按下 */
        else
        {
            /* 测试键长按标志已置位 */
            if (key_long_press_flag == true)
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
        
        /* 初始化ADC */
        adc_init();
        /* 得到ADC采样并滤波之后的值 */
        ch4_adc_value = adc_sensor();

        /* 得到ADC采样并滤波之后的值 超过 sensor_ch4_3500 的报警阈值 且 小于故障值 Short_Fault_H 报警状态处理 */
        if ((ch4_adc_value >= sensor_ch4_3500) && (ch4_adc_value < Short_Fault_H))
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

                    /* 报警次数大于等于 4 实测4秒左右 */
                    if (exceeded_count >= 4)
                    {
                        exceeded_count = 4;
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
                    /* 向FLASH中 写报警记录 */
                    flash_write_record(ALARM_RECORD);
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
        else if ((ch4_adc_value <= Short_Fault_L) || (ch4_adc_value >= Short_Fault_H))
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
                    /* 向FLASH中 写故障记录 */
                    flash_write_record(FAULT_RECORD);
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
                /* 向FLASH中 写报警恢复记录 */
                flash_write_record(ALARM_RECOVERY_RECORD);
            }

            /* 未写传感器故障恢复记录 */
            if (write_no_fault_record_flag == 1)
            {
                write_no_fault_record_flag = 0;
                fault_flag = 0;

                /* Brown-Out Detector 电源电压检测 */
                check_BOD();
                /* 向FLASH中 写故障恢复记录 */
                flash_write_record(FAULT_RECOVERY_RECORD);
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

        /* ---- UART 通讯程序段 ----*/
        /* UART接收完成 */
        if (rx_finished)
        {   
            /* AAA Brown-Out Detector 电源电压检测 */
            check_BOD();
            /* ZZZ 取第一个字节的最高一位 如果为0B 表示是工装板发来的命令 */
            if (!(uart_buffer[0] & 0x80))
            {
                if (rx_index >= 9)
                {
                    if (get_crc(uart_buffer, rx_index) == uart_buffer[rx_index - 2])
                    {
                        for (i = 0; i < 5; i++)
                        {
                            if (SERIAL_ADD[i + 3] != uart_buffer[i + 1])
                                break;
                        }
                        /* 表示地址对上了 */
                        if (i == 5)
                        {
                            for (i = 6; i < rx_index; i++)
                            {
                                /* 把地址信息从接收帧中去掉 */
                                uart_buffer[i - 5] = uart_buffer[i];
                            }
                            /* 把地址的字节数减去 */
                            rx_index -= 5;
                            /* 把命令恢复 */ 
                            uart_buffer[0] |= 0x80;
                            /* 重新计算crc */
                            uart_buffer[rx_index - 2] = get_crc(uart_buffer, rx_index);
                        }
                        else
                        {
                            uart_buffer[0] = 0x00;
                        }
                    }
                    else
                    {
                        uart_buffer[0] = 0x00;
                    }
                }
                else
                {
                    uart_buffer[0] = 0x00;
                }
            }

            switch (uart_buffer[0])
            {
                /* 0xaa开头为 国标规定的协议 读命令 */
                case 0xaa:
                {
                    /* 检测查询的记录类型 应小于0x09 */
                    if (uart_buffer[2] <= GB_COMMAND_MAX)
                    {
                        /* 所查询的索引号小于该类型的记录总数 */
                        if ((uart_buffer[1] <= max_of_each_record[uart_buffer[2]]))
                        {
                            /* 若检查校验和通过 */
                            if ((rx_index >= (GB_READ_FRAME_LEN - 1)) && uart_buffer[4] == get_crc(uart_buffer, GB_READ_FRAME_LEN))
                            {
                                /* AAA Brown-Out Detector 电源电压检测 */
                                check_BOD();
                                /* 查询记录 */
                                flash_read_record(uart_buffer[2], uart_buffer[1]);
                            }
                        }
                    }
                    break;
                }
                /* 0xab开头为写命令 */
                case 0xab:
                {
                    switch (uart_buffer[1])
                    {
                        /* 擦除所有EEP 包括所有记录和设备信息 */
                        case 0x00:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_EASE_REC[0]) == uart_buffer[COMMAND_LEN_EASE_REC[0] - 2])
                            {
                                set_rtc_flag              = false;
                                set_production_date_flag  = false;
                                sensor_expired_flag       = false;
                                write_expired_record_flag = false;

                                /* 从FLASH中读取标定数据 ch4_0 & ch4_3500 因两个标定数据地址相连共 4 bytes */
                                flash_read_data(sensor_demarcation_result, ADC_VALUE_OF_CH4_0_ADDR, 4);
                                
                                for (i = 0; i < 28; i++)
                                {
                                    /* 按页擦除 */
                                    flash_erase_page(RECORD_START_ADDR + (i * BYTES_OF_PAGE));
                                    /* Brown-Out Detector 电源电压检测 */
                                    check_BOD();
                                }

                                /* 将采样结果数组存入FLASH中 */
                                flash_write_data(sensor_demarcation_result, 4, DEVICE_INFO_ADDR, OFFSET_OF_CH4_0);

                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }

                            uart_buffer[2] = device_status[0];
                            uart_buffer[COMMAND_LEN_EASE_REC[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_EASE_REC[1]);
                            uart_buffer[COMMAND_LEN_EASE_REC[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_EASE_REC[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }

                            break;
                        }
                        /* 写时钟指令 */
                        case 0x01:
                        {
                            /* 写时钟指令 */
                            if ((rx_index >= COMMAND_LEN_WRITE_CLOCK[0]))
                            {
                                if (get_crc(uart_buffer, COMMAND_LEN_WRITE_CLOCK[0]) == uart_buffer[COMMAND_LEN_WRITE_CLOCK[0] - 2])
                                {
                                    /* 如果输入的时间不正确 */
                                    /* 年 错误 */
                                    if (uart_buffer[2] > 99)
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }
                                    /* 月 错误 大于12 或为 0 */
                                    if (uart_buffer[3] > 12 || (!uart_buffer[3]))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }
                                    /* 日 错误 大于31 或为 0 */
                                    if (uart_buffer[4] > 31 || (!uart_buffer[4]))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }
                                    /* 时、分、秒 错误 */
                                    if ((uart_buffer[5] > 23) || (uart_buffer[6] > 59) || (uart_buffer[7] > 59))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | ILLEGAL_PARA_EROR;
                                        goto WR_CLOCK_EROR;
                                    }

                                    for (i = 2; i < 8; i++)
                                    {
                                        /* 十六进制转为BCD码 */
                                        i2c_time_code[8 - i] = hex2bcd(uart_buffer[i]);
                                    }

                                    i2c_time_code[0] = i2c_time_code[1];
                                    i2c_time_code[1] = i2c_time_code[2];
                                    i2c_time_code[2] = i2c_time_code[3];
                                    i2c_time_code[3] = i2c_time_code[4];
                                    i2c_time_code[4] = 0;

                                    /* 写入数据 */
                                    i2c_set_time();

                                    delay_1ms(100);

                                    /* 重新读出RTC时间 */
                                    i2c_get_time();

                                    /* 月：表示写入时间不成功 */
                                    if (i2c_time_code[5] != uart_buffer[3] && (i2c_time_code[5] != (uart_buffer[3] + 1)))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    else
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0);
                                    }
                                    /* 年：表示写入时间不成功 */
                                    if (i2c_time_code[6] != uart_buffer[2] && (i2c_time_code[6] != (uart_buffer[2] + 1)))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    /* 日：表示写入时间不成功 */
                                    if (i2c_time_code[3] != uart_buffer[4] && (i2c_time_code[3] != (uart_buffer[4] + 1)))
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    /* 时：表示写入时间不成功 */
                                    if (i2c_time_code[2] != uart_buffer[5] && (i2c_time_code[2] != (uart_buffer[5] + 1)))
                                    {
                                        if (!((uart_buffer[5] == 23) && (i2c_time_code[2] == 0)))
                                            uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    /* 分：表示写入时间不成功 */
                                    if (i2c_time_code[1] != uart_buffer[6] && (i2c_time_code[1] != (uart_buffer[6] + 1)))
                                    {
                                        if (!((uart_buffer[6] == 59) && (i2c_time_code[1] == 0)))
                                            uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
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

                        WR_CLOCK_EROR:
                            uart_buffer[2] = device_status[0];
                            uart_buffer[COMMAND_LEN_WRITE_CLOCK[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_WRITE_CLOCK[1]);
                            uart_buffer[COMMAND_LEN_WRITE_CLOCK[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_WRITE_CLOCK[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* 串口输出  延时函数，关键参数 */
                                delay_1ms(10);
                            }

                            /* 表示时钟输入成功 没有发生错误 */
                            if ((uart_buffer[3] & (~0xe0)) == 0)
                            {
                                /* 时钟写入标志 */
                                life_check[0] = 0xa5;
                                life_check[1] = 0x36;
                                set_rtc_flag = true;

                                /* 将时钟写入标志存在FLASH中 */
                                flash_write_data(&life_check[0], 2, DEVICE_INFO_ADDR, OFFSET_OF_RTC);
                            }
                            break;
                        }
                        /* 写传感器寿命的超时时间 即出厂日期 */
                        case 0x02:
                        {
                            if ((rx_index >= COMMAND_LEN_WR_DATEOFPRODUCTION[0])) 
                            {
                                if (get_crc(uart_buffer, COMMAND_LEN_WR_DATEOFPRODUCTION[0]) == uart_buffer[COMMAND_LEN_WR_DATEOFPRODUCTION[0] - 2]) //
                                {
                                    /* 写出厂日期至FLASH */
                                    flash_write_data(&uart_buffer[2], 5, DEVICE_INFO_ADDR, OFFSET_OF_PRODUCTION_DATE);
                                    /* 读取出厂日期 */
                                    flash_read_data(&uart_buffer[7], PRODUCTION_DATE_ADDR, 5);

                                    /* 月 */
                                    if (uart_buffer[8] != uart_buffer[3])
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }
                                    else
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0);
                                    }

                                    /* 年 */
                                    if (uart_buffer[7] != uart_buffer[2])   
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }

                                    /* 日 */
                                    if (uart_buffer[9] != uart_buffer[4])
                                    {
                                        uart_buffer[3] = (device_status[1] & 0xe0) | Unknown_EROR;
                                    }

                                    // /* ZZZ 方式2 计算气体传感器失效日期 年 */
                                    // uart_buffer[7] += SENSOR_LIFE;
                                    // /* 写气体传感器失效日期至FLASH 第一个位是失效标志位 故偏移要 + 1 */
                                    // flash_write_data(&uart_buffer[7], 5, DEVICE_INFO_ADDR, OFFSET_OF_SENSOR_EXPIRED + 1);
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
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            /* 表示出厂日期写入成功 */
                            if ((uart_buffer[3] & (~0xe0)) == 0)
                            {
                                life_check[2] = 0x5a;
                                life_check[3] = 0xe7;
                                set_production_date_flag = true;

                                flash_write_data(&life_check[2], 2, DEVICE_INFO_ADDR, OFFSET_OF_RTC + 2);
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
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }
                    }
                    break;
                }
                /* 0xac开头 为读命令 */
                /* Se:AC 0X CRC 55 */
                case 0xac:
                {
                    switch (uart_buffer[1])
                    {
                        /* 读时间 带秒 */
                        /* Re:AC 01 00 20 年 月 日 时 分 秒 CRC 55 */
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
                            /* 设备状态字 */
                            uart_buffer[2] = device_status[0];
                            /* 获取当前时间 */
                            i2c_get_time();
                            /* 年 */
                            uart_buffer[4] = i2c_time_code[6];
                            /* 月 */
                            uart_buffer[5] = i2c_time_code[5];
                            /* 日 */
                            uart_buffer[6] = i2c_time_code[3];
                            /* 时 */
                            uart_buffer[7] = i2c_time_code[2];
                            /* 分 */
                            uart_buffer[8] = i2c_time_code[1];
                            /* 秒 */
                            uart_buffer[9] = i2c_time_code[0];

                            uart_buffer[COMMAND_LEN_RE_CLOCK[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_CLOCK[1]);
                            uart_buffer[COMMAND_LEN_RE_CLOCK[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_CLOCK[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }

                        /* 读模组型号等信息 */
                        /* Re:AC 02 00 20 01 00 00 00 00 00 CRC 55 */
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
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }
                        
                        /* 读AD */
                        /* Re:AC 03 00 20 ADH ADL CRC 55 */
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
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }

                        /* 读标定相关信息 */
                        /* Re:AC 04 00 20 CH4_0H CH4_0L CH4_3500H CH4_3500L CRC 55 */
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

                            /* 从FLASH中读取标定数据 ch4_0 & ch4_3500 因两个标定数据地址相连共 4 bytes */
                            flash_read_data(&uart_buffer[4], ADC_VALUE_OF_CH4_0_ADDR, 4);
                            /* ZZZ 未测试 */
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
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }

                        /* 读传感器寿命的超始时间 即出厂日期 */
                        /* Re:AC 05 00 20 XX XX XX XX XX CRC 55 */
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

                            /* 从FLASH中读取出厂日期 */
                            flash_read_data(&uart_buffer[4], PRODUCTION_DATE_ADDR, 5);
                            uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_DATEOFPRODUCTION[1]);
                            uart_buffer[COMMAND_LEN_RE_DATEOFPRODUCTION[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_DATEOFPRODUCTION[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }

                        /* 读序列号 */
                        /* Re:AC 06 00 20 XX XX XX XX XX XX XX XX CRC 55 */
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
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }

                        /* 读设备状态 */
                        /* Re:AC 07 00 20 0X 00 ADH ADL CRC 55 */
                        case 0x07:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_RE_CURTSTATUS[0]) == uart_buffer[COMMAND_LEN_RE_CURTSTATUS[0] - 2])
                            {
                                uart_buffer[3] = device_status[1] & 0xe0;
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }
                            uart_buffer[2] = device_status[0];

                            uart_buffer[4] = 0x00;

                            /* 设备正常 */
                            if ((device_status[1] & 0xe0) == 0x20)
                            {
                                uart_buffer[4] |= 0x00;
                            }

                            /* 设备预热 */
                            if ((device_status[1] & 0xe0) == 0x00)
                            {
                                uart_buffer[4] |= 0x01;
                            }

                            /* 设备报警 */
                            if ((device_status[1] & 0xe0) == 0x40)
                            {
                                uart_buffer[4] |= 0x02;
                            }

                            /* 设备故障 */
                            if ((device_status[1] & 0xe0) == 0x60)
                            {
                                uart_buffer[4] |= 0x04;
                            }

                            /* 传感器过期 */
                            if (sensor_expired_flag == true)
                            {
                                uart_buffer[4] |= 0x08; 
                            }

                            uart_buffer[5] = 0x00;
                            uart_buffer[6] = ch4_adc_value >> 8;
                            uart_buffer[7] = ch4_adc_value;
                            uart_buffer[COMMAND_LEN_RE_CURTSTATUS[1] - 2] = get_crc(uart_buffer, COMMAND_LEN_RE_CURTSTATUS[1]);
                            uart_buffer[COMMAND_LEN_RE_CURTSTATUS[1] - 1] = 0x55;
                            for (i = 0; i < COMMAND_LEN_RE_CURTSTATUS[1]; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }
                        
                        default:
                        {
                            /* Re:AC 0X 00 25 CRC 55 */
                            uart_buffer[2] = device_status[0];
                            uart_buffer[3] = (device_status[1] & 0xe0) | COMMAND_EROR;
                            uart_buffer[4] = get_crc(uart_buffer, 6);
                            uart_buffer[5] = 0x55;
                            for (i = 0; i < 6; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }
                    }
                    break;
                }
                /* 0xad开头 为取消预热命令 */
                case 0xad:
                {
                    switch (uart_buffer[1])
                    {
                        /* 取消预热 */
                        case 0x01:
                        {
                            if (get_crc(uart_buffer, COMMAND_LEN_CANCEL_WARMUP[0]) == uart_buffer[COMMAND_LEN_CANCEL_WARMUP[0] - 2])
                            {
                                /* 取消预热 */
                                STATUS1_NOMAL;
                                uart_buffer[3] = device_status[1] & 0xe0;
                                sensor_preheat_flag = true;
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
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
                            }
                            break;
                        }
                        /* 写记录函数测试 0xad 0x02 0x01~6(record_type) CRC 0x55 */
                        /* Re:AD 02 00 20 record_type CRC 55 */
                        case 0x02:
                        {
                            /* 使用 uart_buffer[4] 暂存 record_type */
                            uart_buffer[4] = uart_buffer[2];

                            /* 命令长度为 5 */
                            if (get_crc(uart_buffer, 5) == uart_buffer[5 - 2])
                            {
                                if (uart_buffer[4] >= ALARM_RECORD && uart_buffer[4] <= POWER_ON_RECORD)
                                {
                                    flash_write_record(uart_buffer[4]);
                                    uart_buffer[3] = device_status[1] & 0xe0;
                                }
                                else
                                {
                                    uart_buffer[3] = (device_status[1] & 0xe0) | COMMAND_EROR;
                                }
                            }
                            else
                            {
                                uart_buffer[3] = (device_status[1] & 0xe0) | CHECKSUM_EROR;
                            }

                            /* 设备状态 */
                            uart_buffer[2] = device_status[0];
                            
                            /* 应答命令长度为 7 */
                            uart_buffer[7 - 2] = get_crc(uart_buffer, 7);
                            uart_buffer[7 - 1] = 0x55;

                            for (i = 0; i < 7; i++)
                            {
                                uart_send(uart_buffer[i]);
                                /* 串口输出 延时函数 关键参数 */
                                delay_1ms(10);
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

            /* 使能UART0接收 */
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

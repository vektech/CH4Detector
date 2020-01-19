/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      adc.h
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
 *                 Multi-Include-Prevent Section
 ******************************************************************************/
#ifndef _ADC_H_
#define _ADC_H_

/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/
#include "GlobalFormat8051Core.h"

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
/* ADCCON0 */
#define set_ADCEX   ADCEX = 1;
#define set_ADCI    ADCI  = 1;
#define set_ADCS    ADCS  = 1;
#define set_AADR2   AADR2 = 1;
#define set_AADR1   AADR1 = 1;
#define set_AADR0   AADR0 = 1;

#define clr_ADCEX   ADCEX = 0;
#define clr_ADCI    ADCI  = 0;
#define clr_ADCS    ADCS  = 0;
#define clr_AADR2   AADR2 = 0;
#define clr_AADR1   AADR1 = 0;
#define clr_AADR0   AADR0 = 0;

/* ADCCON1 */
#define set_ADCEN   ADCCON1 |= SET_BIT7;
#define set_RCCLK   ADCCON1 |= SET_BIT1;
#define set_ADC0SEL ADCCON1 |= SET_BIT0;

#define clr_ADCEN   ADCCON1 &= CLR_BIT7;
#define clr_RCCLK   ADCCON1 &= CLR_BIT1;
#define clr_ADC0SEL ADCCON1 &= CLR_BIT0;

/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/
typedef enum
{
    E_CHANNEL0,
    E_CHANNEL1,
    E_CHANNEL2,
    E_CHANNEL3,
    E_CHANNEL4,
    E_CHANNEL5,
    E_CHANNEL6,
    E_CHANNEL7,
} E_ADCCNL_SEL;

/*******************************************************************************
 *                 Prototype Declare Section
 ******************************************************************************/
void adc_init(void);
void adc_set_input_mode(enum E_ADCCNL_SEL channel);
void adc_channel_sel(enum E_ADCCNL_SEL channel);
void adc_init_interrupt(void);
void adc_enable_interrupt(void);
uint16_t adc_sensor(void);
uint16_t adc_single_sample(void);
void adc_trigger_convertion(void);
uint16_t adc_sensor_filter(uint16_t *p);

#endif /* _ADC_H_ */
/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

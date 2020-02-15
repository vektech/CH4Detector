/**
 *******************************************************************************
 * @copyright 2017-2020, Electronic Technology Co. Ltd
 * @file      GlobalFormat8051Core.h
 * @version   V1.0.0
 * @brief     定义使用8051内核系列MCU的数据类型
 * @warning   适用于N79E815A等8051内核系列
 *******************************************************************************
 * @remarks
 * 1. Version                Date                 Author
 *    v1.0.0                 2020年1月12日         Unknown
 *    Modification: 创建文档
 *******************************************************************************
 */

/*******************************************************************************
 *                 Multi-Include-Prevent Section
 ******************************************************************************/
#ifndef _GLOBAL_FORMAT_8051CORE__H_
#define _GLOBAL_FORMAT_8051CORE__H_

/*******************************************************************************
 *                 Include File Section ('#include')
 ******************************************************************************/

/*******************************************************************************
 *                 Macro Define Section ('#define')
 ******************************************************************************/
typedef bit                   BIT;
typedef unsigned char         uint8_t;
typedef unsigned int          uint16_t;
typedef unsigned long         uint32_t;

/* 16 --> 8 x 2 */
/* v1 is uint16_t */
#define HIBYTE(v1)              ((uint8_t)((v1)>>8))                      
#define LOBYTE(v1)              ((uint8_t)((v1)&0xFF))

/* 8 x 2 --> 16 */
/* v1,v2 is uint8_t */
#define MAKEWORD(v1,v2)         ((((uint16_t)(v1))<<8)+(uint16_t)(v2))      

/* 8 x 4 --> 32 */
/* v1,v2,v3,v4 is uint8_t */
#define MAKELONG(v1,v2,v3,v4)   (uint32_t)((v1<<32)+(v2<<16)+(v3<<8)+v4)  

/* 32 --> 16 x 2 */
/* v1 is uint32_t */
#define YBYTE1(v1)              ((uint16_t)((v1)>>16))                    
#define YBYTE0(v1)              ((uint16_t)((v1)&0xFFFF))

/* 32 --> 8 x 4 */
/* v1 is uint32_t */
#define TBYTE3(v1)              ((uint8_t)((v1)>>24))                     
#define TBYTE2(v1)              ((uint8_t)((v1)>>16))
#define TBYTE1(v1)              ((uint8_t)((v1)>>8))
#define TBYTE0(v1)              ((uint8_t)((v1)&0xFF))

#define SET_BIT0        0x01
#define SET_BIT1        0x02
#define SET_BIT2        0x04
#define SET_BIT3        0x08
#define SET_BIT4        0x10
#define SET_BIT5        0x20
#define SET_BIT6        0x40
#define SET_BIT7        0x80
#define SET_BIT8        0x0100
#define SET_BIT9        0x0200
#define SET_BIT10       0x0400
#define SET_BIT11       0x0800
#define SET_BIT12       0x1000
#define SET_BIT13       0x2000
#define SET_BIT14       0x4000
#define SET_BIT15       0x8000

#define CLR_BIT0        0xFE
#define CLR_BIT1        0xFD
#define CLR_BIT2        0xFB
#define CLR_BIT3        0xF7
#define CLR_BIT4        0xEF
#define CLR_BIT5        0xDF
#define CLR_BIT6        0xBF
#define CLR_BIT7        0x7F

#define CLR_BIT8        0xFEFF
#define CLR_BIT9        0xFDFF
#define CLR_BIT10       0xFBFF
#define CLR_BIT11       0xF7FF
#define CLR_BIT12       0xEFFF
#define CLR_BIT13       0xDFFF
#define CLR_BIT14       0xBFFF
#define CLR_BIT15       0x7FFF

/* 通用宏定义 */
#define X_Abs(Val) (((Val) >= 0U) ? (Val) : (-(Val)))    /*!< 宏定义函数, 取绝对值 */
#define X_Not(Val) (((Val) == false) ? (true) : (false)) /*!< 宏定义函数, 逻辑取反 */
#define X_Bit(Type, Val) ((Type)1U << (Val))             /*!< 宏定义函数, 取位 */

#ifndef NULL
#define NULL (0U) /*!< 空指针值 */
#endif

/* To avoid complier's warning for unused parameters */
#define UNUSED(x) (void)x
/* DEBUG conditional compilation */
#define _DEBUG_
/*******************************************************************************
 *                 Struct Define Section ('typedef')
 ******************************************************************************/
/* 布尔逻辑值 */
typedef enum bl_
{
    false = 0,
    true  = !false,
} bl;

/* 三值逻辑值(ternary) */
typedef enum tn_
{
    _False   = 0x0,
    _True    = 0x1,
    _Unknown = 0x2,
} tn;

/* 8051内核基本数据类型中 无 u64 和 s64 */
typedef signed char   s8;
typedef signed int    s16;
typedef signed long   s32;

typedef unsigned char u8;
typedef unsigned int  u16;
typedef unsigned long u32;

typedef float         f32;
typedef double        f64;

/**
 * @brief   常用数据转换类型
 * @warning 在使用C51时 注意以下几点
 *          <1> 在使用联合体时 可能不支持匿名联合体 如果联合体中存在结构体 最好将其进行命名
 *          <2> u32 类型不支持位域操作 如 u32 s_Bit0 : 1; 等
 */
typedef union u8_1_ 
{
    u8 u8;
    struct u8_bit_feild_
    {
        u8 s_Bit0 : 1; /*!< 最低位 */
        u8 s_Bit1 : 1;
        u8 s_Bit2 : 1;
        u8 s_Bit3 : 1;
        u8 s_Bit4 : 1;
        u8 s_Bit5 : 1;
        u8 s_Bit6 : 1;
        u8 s_Bit7 : 1;
    } u8_bit_feild;
} u8_1;

typedef union u16_1_ 
{
    u16 u16;
    struct u16_bit_feild_
    {
        u16 s_Bit0 : 1; /*!< 最低位 */
        u16 s_Bit1 : 1;
        u16 s_Bit2 : 1;
        u16 s_Bit3 : 1;
        u16 s_Bit4 : 1;
        u16 s_Bit5 : 1;
        u16 s_Bit6 : 1;
        u16 s_Bit7 : 1;
        u16 s_Bit8 : 1;
        u16 s_Bit9 : 1;
        u16 s_Bit10 : 1;
        u16 s_Bit11 : 1;
        u16 s_Bit12 : 1;
        u16 s_Bit13 : 1;
        u16 s_Bit14 : 1;
        u16 s_Bit15 : 1;
    } u16_bit_feild;
} u16_1;

/**
 * @warning 位域操作类型必须为 char(u8) 或者 int(u16)
 *          否则会出现 error C155: 's_Bit0': char/int required for feilds
 */
typedef union u32_1_ 
{
    u32 u32;
    struct u32_bit_feild_
    {
        u8 s_Bit0 : 1; /*!< 最低位 */
        u8 s_Bit1 : 1;
        u8 s_Bit2 : 1;
        u8 s_Bit3 : 1;
        u8 s_Bit4 : 1;
        u8 s_Bit5 : 1;
        u8 s_Bit6 : 1;
        u8 s_Bit7 : 1;
        u8 s_Bit8 : 1;
        u8 s_Bit9 : 1;
        u8 s_Bit10 : 1;
        u8 s_Bit11 : 1;
        u8 s_Bit12 : 1;
        u8 s_Bit13 : 1;
        u8 s_Bit14 : 1;
        u8 s_Bit15 : 1;
        u8 s_Bit16 : 1;
        u8 s_Bit17 : 1;
        u8 s_Bit18 : 1;
        u8 s_Bit19 : 1;
        u8 s_Bit20 : 1;
        u8 s_Bit21 : 1;
        u8 s_Bit22 : 1;
        u8 s_Bit23 : 1;
        u8 s_Bit24 : 1;
        u8 s_Bit25 : 1;
        u8 s_Bit26 : 1;
        u8 s_Bit27 : 1;
        u8 s_Bit28 : 1;
        u8 s_Bit29 : 1;
        u8 s_Bit30 : 1;
        u8 s_Bit31 : 1;
    } u32_bit_feild;
} u32_1;

typedef union u16_8_ 
{
    u16 u16;
    struct u16_byte_feild_
    {
        u8 u8_Byte0; /*!< 最低位 */
        u8 u8_Byte1;
    } u16_byte_feild;
} u16_8;

typedef union u32_8_ 
{
    u32 u32;
    struct u32_byte_feild_
    {
        u8 u8_Byte0; /*!< 最低位 */
        u8 u8_Byte1;
        u8 u8_Byte2;
        u8 u8_Byte3;
    }u32_byte_feild;
} u32_8;

typedef union s16_8_ 
{
    s16 s16;
    struct s16_byte_feild_
    {
        u8 u8_Byte0; /*!< 最低位 */
        u8 u8_Byte1;
    }s16_byte_feild;
} s16_8;

typedef union s32_8_ 
{
    s32 s32;
    struct s32_byte_feild_
    {
        u8 u8_Byte0; /*!< 最低位 */
        u8 u8_Byte1;
        u8 u8_Byte2;
        u8 u8_Byte3;
    }s32_byte_feild;
} s32_8;

typedef union f32_8_ {
    f32 f32;
    struct f32_byte_feild_
    {
        u8 u8_Byte0; /*!< 最低位 */
        u8 u8_Byte1;
        u8 u8_Byte2;
        u8 u8_Byte3;
    }f32_byte_feild;
} f32_8;

typedef union f64_8_ {
    f64 f64;
    struct f64_byte_feild_
    {
        u8 u8_Byte0; /*!< 最低位 */
        u8 u8_Byte1;
        u8 u8_Byte2;
        u8 u8_Byte3;
        u8 u8_Byte4;
        u8 u8_Byte5;
        u8 u8_Byte6;
        u8 u8_Byte7;
    }f64_byte_feild;
} f64_8;

/*******************************************************************************
 *                 Prototype Declare Section
 ******************************************************************************/

#endif /* _GLOBAL_FORMAT_8051CORE__H_ */
/*******************************************************************************
 *                 End of File ('EOF')
 ******************************************************************************/

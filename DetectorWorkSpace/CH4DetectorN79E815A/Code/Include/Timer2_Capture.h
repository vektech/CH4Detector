typedef enum
{
    E_CAPTURE0,
    E_CAPTURE1,
    E_CAPTURE2,
} E_CAPTURE_SEL;

typedef enum
{
    E_FALLING_S,
    E_RISING_S,
    E_FALLING_RISING_S,
} E_EDGE_SEL;

typedef enum
{
    E_DIV4,
    E_DIV8,
    E_DIV16,
    E_DIV32,
    E_DIV64,
    E_DIV128,
    E_DIV256,
    E_DIV512,
} E_CLKDIV_SEL;

typedef enum
{
    E_TF0,
    E_CAPF0,
    E_CAPF1,
    E_CAPF2,
} E_AUTOREL_SEL;

//T2MOD
#define set_LDEN   T2MOD |= SET_BIT7;
#define set_T2DIV2 T2MOD |= SET_BIT6;
#define set_T2DIV1 T2MOD |= SET_BIT5;
#define set_T2DIV0 T2MOD |= SET_BIT4;
#define set_CAPCR  T2MOD |= SET_BIT3;
#define set_COMPCR T2MOD |= SET_BIT2;
#define set_LDTS1  T2MOD |= SET_BIT1;
#define set_LDTS0  T2MOD |= SET_BIT0;

#define clr_LDEN   T2MOD &= CLR_BIT7;
#define clr_T2DIV2 T2MOD &= CLR_BIT6;
#define clr_T2DIV1 T2MOD &= CLR_BIT5;
#define clr_T2DIV0 T2MOD &= CLR_BIT4;
#define clr_CAPCR  T2MOD &= CLR_BIT3;
#define clr_COMPCR T2MOD &= CLR_BIT2;
#define clr_LDTS1  T2MOD &= CLR_BIT1;
#define clr_LDTS0  T2MOD &= CLR_BIT0;

//CAPCON0
#define set_CAPEN2 CAPCON0 |= SET_BIT6;
#define set_CAPEN1 CAPCON0 |= SET_BIT5;
#define set_CAPEN0 CAPCON0 |= SET_BIT4;

#define clr_CAPEN2 CAPCON0 &= CLR_BIT6;
#define clr_CAPEN1 CAPCON0 &= CLR_BIT5;
#define clr_CAPEN0 CAPCON0 &= CLR_BIT4;

#define clr_CAPF2  CAPCON0 &= CLR_BIT2;
#define clr_CAPF1  CAPCON0 &= CLR_BIT1;
#define clr_CAPF0  CAPCON0 &= CLR_BIT0;

//CAPCON2
#define set_ENF2 CAPCON2 |= SET_BIT6;
#define set_ENF1 CAPCON2 |= SET_BIT5;
#define set_ENF0 CAPCON2 |= SET_BIT4;

#define clr_ENF2 CAPCON2 &= CLR_BIT6;
#define clr_ENF1 CAPCON2 &= CLR_BIT5;
#define clr_ENF0 CAPCON2 &= CLR_BIT4;

void Capture_Select(E_CAPTURE_SEL cap, E_EDGE_SEL edge);
void Timer2_Clock_Divider_Sel(E_CLKDIV_SEL clkd);
void Auto_Rel_Sel(E_AUTOREL_SEL rel_sel);
void Noise_Filter_Sel(E_CAPTURE_SEL cap);
void Set_IO_Input_Mode(E_CAPTURE_SEL cap);
void Capture_Init(void);
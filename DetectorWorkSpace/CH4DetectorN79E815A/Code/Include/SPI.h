
typedef enum
{
    E_ClockDev16,
    E_ClockDev32,
    E_ClockDev64,
    E_ClockDev128,
}E_SPICLK_Sel;

#define set_SSOE        SPCR |= SET_BIT7
#define set_SPIEN       SPCR |= SET_BIT6
#define set_LSBFE       SPCR |= SET_BIT5
#define set_MSTR        SPCR |= SET_BIT4
#define set_CPOL        SPCR |= SET_BIT3
#define set_CPHA        SPCR |= SET_BIT2
#define set_SPR1        SPCR |= SET_BIT1
#define set_SPR0        SPCR |= SET_BIT0

#define set_DISMODF     SPSR |= SET_BIT3

#define clr_SSOE        SPCR &= CLR_BIT7
#define clr_SPIEN       SPCR &= CLR_BIT6
#define clr_LSBFE       SPCR &= CLR_BIT5
#define clr_MSTR        SPCR &= CLR_BIT4
#define clr_CPOL        SPCR &= CLR_BIT3
#define clr_CPHA        SPCR &= CLR_BIT2
#define clr_SPR1        SPCR &= CLR_BIT1
#define clr_SPR0        SPCR &= CLR_BIT0

#define clr_DISMODF     SPSR &= CLR_BIT3
#define clr_SPIF        SPSR &= CLR_BIT7



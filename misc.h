#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

/*--------------------------------------------------------------------------------------------------
    system includes
--------------------------------------------------------------------------------------------------*/
#define F_CPU 8000000UL  /* 8 MHz */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/
#define FALSE 0
#define TRUE !FALSE

#define MIN( a, b ) ( ((a) < (b)) ? (a) : (b) )
#define MAX( a, b ) ( ((a) > (b)) ? (a) : (b) )

#define BIT0  (0x01)
#define BIT1  (0x02)
#define BIT2  (0x04)
#define BIT3  (0x08)
#define BIT4  (0x10)
#define BIT5  (0x20)
#define BIT6  (0x40)
#define BIT7  (0x80)

#define BASE_2  UINT8_C(2)
#define BASE_10 UINT8_C(10)

/* -> 6250000 = 10*8MHz/(clkdiv=8)*(60s)/(96clk) .
   -> 'x' is 'u16Tempo'
   -> u16Tempo = 1200 <=> bpm = 120.0, i.e. bpm = u16Tempo/10
      This where the '*10' comes from.
*/
#define MAC_CALCOCR1A(x) ((uint32_t)(6250000)/(uint32_t)(x) - 1)

/* button defines */
#define BUTCLRISE ( 1)
#define BUTCLFALL (-1)
#define BUTCLREST ( 0)

#define BUTPR_TEMPO  (swt8.states[2] & BIT4)

#define BUTCL_F0     (swtL.debs[1].edg[4])
#define BUTCL_F1     (swtL.debs[1].edg[5])

#define BUTCL_PLAY   (swtL.debs[1].edg[1])
#define BUTCL_STOP   (swtL.debs[1].edg[2])

#define BUTCL_OCTSFTUP (swt8.debs[0].edg[4])
#define BUTCL_OCTSFTDN (swt8.debs[0].edg[5])

/* rotary encoders defines */
#define ROTCLK     ( 1)
#define ROTCNTRCLK (-1)
#define ROTREST    ( 0)

/* LCD related defines */
#define PAGE0   0
#define PAGE1   1
#define PAGE2   2
#define PAGE3   3
#define PAGEMAX PAGE3

/* LEDs defines */
#define LED_RATE_ON {ledR.vals[1] |= BIT7;}

/*--------------------------------------------------------------------------------------------------
    typedefs
--------------------------------------------------------------------------------------------------*/
#define PIN_POUT_NOT_USED 0xFF
typedef struct
{
    volatile uint16_t begval;
    volatile uint16_t maxval;
    volatile uint16_t averag;
    volatile uint32_t buffer;
} TIMER;

typedef struct
{
    int8_t  cnt[8]; /* counter associated to 1 bit of the byte */
    uint8_t out;    /* bitfield containing states of bits */
    int8_t edg[8];  /* -1/0/1 : one for each bit */
    int8_t cntMax[8]; /* debounce time : 10 <=> ~50ms */
} DEBOUNCE;

#define Q7_COUNTMAX INT8_C(3)
typedef struct
{
    volatile uint8_t* port; /* port dedicated to both /PL and CP */
    volatile uint8_t* pins; /* pin group dedicated to all 74HC165 */
    const int8_t pout_pl; /* output pin for parallel load (/PL) */
    const int8_t pout_cp; /* output pin for clock (CP) */
    const int8_t pins_q7[Q7_COUNTMAX]; /* so far 3 74HC165 at most */
    uint8_t states[Q7_COUNTMAX]; /* same remark */
    DEBOUNCE debs[Q7_COUNTMAX]; /* here are the states debounced */
} PARALOAD;

#define DSA_COUNTMAX INT8_C(2)
typedef uint8_t VALSTOLED[DSA_COUNTMAX];
typedef struct
{
    volatile uint8_t* port; /* port dedicated to CP */
    const int8_t pout_cp; /* output pin for clock (CP) */
    const int8_t pout_dsa[DSA_COUNTMAX]; /* so far 2 DSA at most */
    VALSTOLED vals;
    VALSTOLED vals_mem;
} COMCPLED;

typedef struct
{
    PARALOAD* p_swt;
    const int8_t ind; /* 74HC165 index */
    const int8_t bitA;
    const int8_t bitB;
    int8_t state;
    uint8_t oldAB;
} ROTENC;

#define BUFMAX 100
typedef volatile struct
{
    uint8_t t[BUFMAX];
    int8_t i; /* number of bytes stored */
} BYTEBUF;

/*--------------------------------------------------------------------------------------------------
    extern const variables
--------------------------------------------------------------------------------------------------*/
//extern const PROGMEM unsigned char copyRightChar[];

/*--------------------------------------------------------------------------------------------------
    extern variables
--------------------------------------------------------------------------------------------------*/
extern uint16_t u16dbg1;
extern uint16_t u16dbg2;
extern uint16_t u16dbg3;

extern uint8_t u8dbg1;
extern uint8_t u8dbg2;
extern uint8_t u8dbg3;
extern uint8_t u8dbg4;
extern uint8_t u8dbg5;

/*--------------------------------------------------------------------------------------------------
    functions protoypes
--------------------------------------------------------------------------------------------------*/
uint32_t mittelwert(uint32_t newval, uint8_t factor, volatile uint32_t* pavgsum);
void calcTimer(TIMER* timer, uint8_t factor, uint16_t curval);
void myStrCpy(char * c, const char * constc);
void u8ToStrBin(uint8_t u8, char* str, uint8_t b);
void u8ToStrDec(uint8_t u8, char* str, uint8_t b);
void u16ToStrDec(uint16_t u16, char* str, uint8_t b);
void i7ToStrDec(int8_t i7, char* str, uint8_t b);
void i16toStrTempo(int16_t i16Tempo, char* str, uint8_t b);
void rpg8RateToStr(int8_t rpg8Rate, char* str, uint8_t b);
void readSwt(PARALOAD* swt);
void readRotEnc(ROTENC* rotEnc);
void putLeds( COMCPLED* led );
void putLedsOn( COMCPLED* led
              , uint8_t bitsOn0
              , uint8_t bitsOn1
              );
void putLedsOff( COMCPLED* led
               , uint8_t bitsOff0
               , uint8_t bitsOff1
               );
void putLedsVal( COMCPLED* led
               , uint8_t bitsOverwrite0
               , uint8_t bitsOverwrite1
               );
void debounce( uint8_t input, DEBOUNCE* pdeb );
void bytebuf_add_sort(uint8_t b, BYTEBUF* buf);
int8_t bytebuf_remove( uint8_t key
                     , BYTEBUF* buf
                     );
void bytebuf_queue(uint8_t b, BYTEBUF* buf);
uint8_t bytebuf_dequeue(BYTEBUF* buf);
void putC1(uint8_t c, BYTEBUF* buf);

#endif // MISC_H_INCLUDED

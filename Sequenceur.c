/*
RoT 15.04.2015
- rpg8Oct
TODO : - select rpg8Oct with switches
       - insert
       - pattern

RoT 14.04.2015
- putLedsxxx
- OctSft
TODO : Inserts
********************
Fire Test
---------
Idle  16600 20417
IHM   02200 03267
Tmpo  00248 00349
Midin 00017 00041
********************

RoT 13.04.2015
- 96 clk ppqn

RoT 11.04.2015
Midi-in ISR implemented
Warning : in ISR should be implemented a clever way
          to interpret MIDI messages according to MIDI spec.
          For now, logic only counts 3 MIDI bytes, stores them,
          and continues, which can imply de-sync and shift
          if only one or two MIDI byte are received.
TODO : Arpeggiator!

RoT 10.04.2015
Tempo change with poti implemented.
TODO:
- MIDI IN: transfer incoming keyboard messages to output
- arpeggiator!
- tests with Electribe and Midi-Ox

    keyboard -> seq -> Midi-Ox -> Electribe

RoT 15.08.2014
MMC Start and Stop implemented along with MIDI clock 24 per quarter note.
TODO: change tempo with poti. Send notes.

RoT 13.08.2014
Button F1 to send a fixed MIDI note on channel 1. Tested with gMidiMonitor
TODO : function playNote. Display midi interrupt durations.

RoT 16.07.2014
Button F0 used to switch pages
Tempo interrupt activated.
TODO: play note on MIDI channel 0 (USART0)

RoT 09.07.2014
function debounce() implemented. To be tested! => cnt and edg.
Move putLeds() to Idle?
Possibility to compile with -O2 instead of -Os.
Debounce times for swtR are 100 <=> 500 ms for tests only!

RoT 08.07.2014
function readSwt and putLeds impemented.
TODO: extend these new functions to left bord and RPG-8 board.

RoT 07.07.2014
structure PARALOAD implemented.
First version of a improvement of the old function readSwitchRight() in ISR routine.
TODO: -replace utoa and itoa by old functions due to variable length of output string.
      -write generic function associated to PARALOAD

RoT 06.07.2014
Basic program for LCD improved : function cpy...LcdXY
ISR for IHM activated : interrupt every 5 ms.
TODO: convert old readSwitch functions into function with a pointer to structure PARALLELLOAD (common to several shift register, to be defind)
These structures PARALLELLOAD should be defined depending on the pining.

RoT 05.07.2014
This is the basic program for the LCD. Idle loop : 5920us
TODO: read buttons from all ports.

RoT 04.07.2014
TODO : change definitions for display size cf. http://homepage.hispeed.ch/peterfleury/group__pfleury__lcd.html
and test if lines are well defined.

*/

#include "misc.h"

#include "midi.h"
#include "lcd.h"
#include "rpg8.h"

#define DEBUG_IDLE
#define DEBUG_IHM
#define DEBUG_TEMPO
#undef DEBUG_USART1
#define DEBUG_USART0RX

/*--------------------------------------------------------------------------------------------------
    static const variables
--------------------------------------------------------------------------------------------------*/
/* compilation date to be written in the ROM */
static const char strDate[12] = __DATE__;

/* compilation time to be written in the ROM */
static const char strTime[9] = __TIME__;

/*--------------------------------------------------------------------------------------------------
    static variables
--------------------------------------------------------------------------------------------------*/

#ifdef DEBUG_IDLE
/* timer for idle loop */
static TIMER timerIdle = {0, 0, 0, 0};
#endif /* DEBUG_IDLE */

#ifdef DEBUG_IHM
/* timer for IHM loop */
static TIMER timerIHM = {0, 0, 0, 0};
#endif /* DEBUG_IHM */

#ifdef DEBUG_TEMPO
/* timer for tempo loop */
static TIMER timerTempo = {0, 0, 0, 0};
#endif /* DEBUG_TEMPO */

#ifdef DEBUG_USART1
/* timer for tempo usart1 */
static TIMER timerUsart1 = {0, 0, 0, 0};
#endif /* DEBUG_USART1 */

#ifdef DEBUG_USART0RX
/* timer for tempo usart0 Rx */
static TIMER timerUsart0Rx = {0, 0, 0, 0};
#endif /* DEBUG_USART0RX */

/* push buttons on the right : 2 74HC165 */
static PARALOAD swtR = { &PORTA                            /* port */
                       , &PINA                             /* pins */
                       , PA1                               /* port_pl */
                       , PA3                               /* port_cp */
                       , {PINA2, PINA0, PIN_POUT_NOT_USED} /* pins_q7 */
                       , {0x00, 0x00, 0x00}                /* states */
                       , { { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         , { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         , { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         } /* debs */
                       };

/* leds on the right : 2 DSA */
COMCPLED ledR = { &PORTA       /* port */
                , PA6          /* port_cp */
                , {PA5, PA7}   /* pins_dsa */
                , {0x00, 0x00} /* vals */
                , {0xFF, 0xFF} /* vals_mem */
                };

/* push buttons on the left : 3 74HC165*/
static PARALOAD swtL = { &PORTL                            /* port */
                       , &PINL                             /* pins */
                       , PL1                               /* port_pl */
                       , PL3                               /* port_cp */
                       , {PINL2, PINL0, PINL4}             /* pins_q7 */
                       , {0x00, 0x00, 0x00}                /* states */
                       , { { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         , { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         , { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         } /* debs */
                       };

/* leds on the left : 2 DSA */
COMCPLED ledL = { &PORTL       /* port */
                , PL6          /* port_cp */
                , {PL5, PL7}   /* pins_dsa */
                , {0x00, 0x00} /* vals */
                , {0xFF, 0xFF} /* vals_mem */
                };

/* push buttons for RPG-8 : 3 74HC165 */
static PARALOAD swt8 = { &PORTC                            /* port */
                       , &PINC                             /* pins */
                       , PC0                               /* port_pl */
                       , PC1                               /* port_cp */
                       , {PINC2, PINC4, PINC6}             /* pins_q7 */
                       , {0x00, 0x00, 0x00}                /* states */
                       , { { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         , { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         , { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* cnt */
                           , 0x00 /* out */
                           , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /* edg */
                           , {10, 10, 10, 10, 10, 10, 10, 10} /* cntMax = 10 <=> ~50ms @ 5ms ISR */
                           }
                         } /* debs */
                       };

/* leds on RPG-8 : 1 DSA */
static COMCPLED led8 = { &PORTC                   /* port */
                       , PC5                      /* port_cp */
                       , {PC7, PIN_POUT_NOT_USED} /* pins_dsa */
                       , {0x00, 0x00}             /* vals */
                       , {0xFF, 0xFF}             /* vals_mem */
                       };

static ROTENC rotEnc1 = { &swtL /* p_swt */
                        , 2     /* ind */
                        , BIT6  /* bitA */
                        , BIT7  /* bitB */
                        , 0     /* state */
                        , 0x00  /* oldAB */
                        };

static ROTENC rotEnc4 = { &swt8 /* p_swt */
                        , 2     /* ind */
                        , BIT3  /* bitA */
                        , BIT2  /* bitB */
                        , 0     /* state */
                        , 0x00  /* oldAB */
                        };

static ROTENC rotEnc5 = { &swt8 /* p_swt */
                        , 2     /* ind */
                        , BIT1  /* bitA */
                        , BIT0  /* bitB */
                        , 0     /* state */
                        , 0x00  /* oldAB */
                        };

/* page displayed on LCD */
static volatile int8_t page = PAGE0;
static volatile int8_t pagePrv = -1;

static int16_t i16Tempo = 0x04B0 ; /* 0xFFF = 4095 <=> 409.5 bpm */

/* read IHM every x ms */
ISR( TIMER4_COMPA_vect )
{
    /* IHM interrupt can be interrupted */
    sei();

#ifdef DEBUG_IHM
    /* Count number of cycles for IHM loop */
    timerIHM.begval = TCNT3;
#endif /* DEBUG_IHM */

    /** DEBUT DE LECTURE DES SWITCHS */

    /* read push buttons on the right */
    readSwt(&swtR);
    swtR.states[1] = (swtR.states[1] & 0x0F); /* Q7 - Q4 not connected for 2nd 74HC165 */

    /* read push buttons on the left */
    readSwt(&swtL);

    /* read push buttons on the RPG-8 */
    readSwt(&swt8);
    swt8.states[1] = (swt8.states[1] & 0x0F); /* Q7 - Q4 not connected for 2nd 74HC165 */
    swt8.states[2] = (swt8.states[2] & 0x3F); /* Q7 - Q4 not connected for 3rd 74HC165 */

    /* read state for rotary encoders */
    readRotEnc(&rotEnc1);
    readRotEnc(&rotEnc4);
    readRotEnc(&rotEnc5);

    /** FIN DE LECTURE DES SWITCHS */

    if (BUTCLRISE == BUTCL_F0) /* next page */
    {
        pagePrv = page;
        page++;
        if (PAGEMAX < page)
        {
            page = 0;
        }
    }

    if (BUTCLRISE == BUTCL_PLAY) /* play */
    {
        cli();

        /* SysEx ? */
//        putC1(0xF0, &buf1);
//        putC1(0x7F, &buf1);
//        putC1(0x01, &buf1);
//        putC1(0x06, &buf1);
//        putC1(0x02, &buf1);
//        putC1(0xF7, &buf1);

        putC1(MIDI_COMMON_START, &buf1);

        sei();

    }

    if (BUTCLRISE == BUTCL_STOP) /* stop */
    {
        cli();

        /* SysEx ? */
//        putC1(0xF0, &buf1);
//        putC1(0x7F, &buf1);
//        putC1(0x01, &buf1);
//        putC1(0x06, &buf1);
//        putC1(0x01, &buf1);
//        putC1(0xF7, &buf1);

        putC1(MIDI_COMMON_STOP, &buf1);

        sei();

//        TCCR1B &= ~((1 << CS11) | (1 << CS10)); /* Stop timer 1 */
    }

    if (BUTCLRISE == BUTCL_OCTSFTUP) /* octave shift up */
    {
        if ( rpg8OctSft < 3 )
        {
            rpg8OctSft++;
            putLedsVal(&led8, led8.vals[0] << 1, 0x00);
        }
//        else
//        {
//            rpg8OctSft = 3;
//            putLedsVal(BIT6, 0x00);
//        }
    }

    if (BUTCLRISE == BUTCL_OCTSFTDN) /* octave shift down */
    {
        if ( -3 < rpg8OctSft )
        {
            rpg8OctSft--;
            putLedsVal(&led8, led8.vals[0] >> 1, 0x00);
        }
//        else
//        {
//            rpg8OctSft = -3;
//            putLedsVal(BIT0, 0x00);
//        }
    }

    /* set tempo */
    if (ROTREST != rotEnc4.state)
    {
        if ( !!BUTPR_TEMPO ) /* potentiometer is clicked => more precision */
        {
            i16Tempo += rotEnc4.state;
        }
        else
        {
            i16Tempo += 10*rotEnc4.state;
        }
        i16Tempo = MAX(300, i16Tempo);
        i16Tempo = MIN(2500, i16Tempo);

        OCR1A = MAC_CALCOCR1A(i16Tempo); /* Prescaler 8 => 96 clk per 1/4 note */
        if ( OCR1A < TCNT1 )
        {
            TCNT1 = 0;
        }
    }

    /* set gate */
    if (ROTREST != rotEnc5.state)
    {
        rpg8Gate += rotEnc5.state;

        rpg8Gate = MAX(1, rpg8Gate);
        rpg8Gate = MIN(48, rpg8Gate);
    }

    /* set rate */
    if (ROTREST != rotEnc1.state)
    {
        rpg8Rate += rotEnc1.state;

        rpg8Rate = MAX(0, rpg8Rate);
        rpg8Rate = MIN(RATECARDINAL-1, rpg8Rate);
    }

#ifdef DEBUG_IHM
    /* Count number of cycles for IHM loop */
    calcTimer(&timerIHM, 6, TCNT3);
#endif /* DEBUG_IHM */
}

/* tempo loop */
ISR( TIMER1_COMPA_vect )
{
#ifdef DEBUG_TEMPO
    /* Count number of cycles for tempo loop */
    timerTempo.begval = TCNT3;
#endif /* DEBUG_TEMPO */

    rpg8Out();

#ifdef DEBUG_TEMPO
    /* Count number of cycles for tempo loop */
    calcTimer(&timerTempo, 6, TCNT3);
#endif /* DEBUG_TEMPO */
}

/* transmit on USART1 */
ISR(USART1_UDRE_vect)
{
#ifdef DEBUG_USART1
    /* Count number of cycles for usart1 loop */
    timerUsart1.begval = TCNT3;
#endif /* DEBUG_USART1 */

    if (0 < buf1.i)
    {
        UDR1 = bytebuf_dequeue(&buf1);
    }
    else
    {
        UCSR1B &= ~(1 << UDRIE1); /* disable interrupt 'transmit on USART1' */
    }

#ifdef DEBUG_USART1
    /* Count number of cycles for usart1 loop */
    calcTimer(&timerUsart1, 2, TCNT3);
#endif /* DEBUG_USART1 */
}

ISR( USART0_RX_vect )
{

#ifdef DEBUG_USART0RX
    /* Count number of cycles for USART0RX loop */
    timerUsart0Rx.begval = TCNT3;
#endif /* DEBUG_USART0RX */

    uint8_t byteIn;

    /* read out received byte - do it even if read value not used.
    if not read => interrupt flag remain active */
    byteIn = UDR0;

    rpg8In(byteIn);

#ifdef DEBUG_USART0RX
    /* Count number of cycles for USART0RX loop */
    calcTimer(&timerUsart0Rx, 2, TCNT3);
#endif /* DEBUG_USART0RX */
}

int main(void)
{
    /***************************************
     Ports E/S Initialisation
     Carte droite : PORTA
        PA0 : Q7 - 2 ; /PL  : PA1
        PA2 : Q7 - 1 ; CP165: PA3
        PA4 : empty  ; DSA1 : PA5
        PA6 : CP164  ; DSA2 : PA7
     Carte gauche : PORTL
        PL0 : Q7 - 2 ; /PL  : PL1
        PL2 : Q7 - 1 ; CP165: PL3
        PL4 : Q7 - 3 ; DSA2 : PL5
        PL6 : CP164  ; DSA1 : PL7
     Carte RPG8 : PORTC
        PC0 : /PL    ; CP165: PC1
        PC2 : Q7 - 1 ;/CE165: PC3
        PC4 : Q7 - 2 ; CP164: PC5
        PC6 : Q7 - 3 ; DSA  : PC7
    ***************************************/
	DDRC = 0b10101011;	/* config for switchs RPG8 on PORTC */
    DDRA = 0b11111010;	/* Bits 0 et 2 en entrée pour les sorties des 74HC165 (carte droite) */
    DDRL = 0b11101010;	/* Bits 0, 2 et 4 en entrée pour les sorties des 74HC165 (carte gauche) */

    /* refresh LEDs in the right */
    putLedsVal(&ledR, BIT7, 0x00);

    /* refresh LEDs in the left */
    putLedsVal(&ledL, 0x00, 0x00);

    /* refresh LEDs in the Rpg-8 section */
    putLedsVal(&led8, BIT3, 0x00);

    #define BAUDRATE_UBRR 15

	/* Enable Receive on USART0 */
	UCSR0B = (1<<RXEN0);
	/* Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size */
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	/* set baud rate */
	UBRR0 = BAUDRATE_UBRR;
	/* enable interrupt for receiving on USART0 */
	UCSR0B |= (1<<RXCIE0);

	/* Enable Transmit on USART1 */
	UCSR1B = (1<<TXEN1);
	/* Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size */
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);
	/* set baud rate */
	UBRR1 = BAUDRATE_UBRR;

    /* Sequence Timers Initialisation */
    sei (); /* Enable global interrupts */
    TCCR1B |= (1 << WGM12); /* Configure timer 1 for CTC mode */
    TCCR4B |= (1 << WGM42); /* Configure timer 4 for CTC mode */
    TIMSK1 |= (1 << OCIE1A); /* Enable Timer 1 CTC interrupt */
    TIMSK4 |= (1 << OCIE4A); /* Enable timer 4 CTC interrupt */

    /* Timer 1 : Tempo Initialisation */
    OCR1A = MAC_CALCOCR1A(i16Tempo); /* Prescaler 8 => 96 clk per 1/4 note */
    TCNT1 = OCR1A - 1;

    /* Timer 3 : "Debug" Timer 3 start */
    TCCR3B |= (1 << CS31); /* Start timer 3 at Fcpu /8 */

    /* Timer 4 : Read Switch Interrupt start */
    OCR4A = 5000; /* Prescaler 8 : 5 ms <=> 5000 cycles */
    TCCR4B |= (1 << CS31); /* Start timer 4 at Fcpu /8 */

    /* initialize display, cursor off */
    lcd_init(LCD_DISP_ON);

    /* clear display and home cursor */
    lcd_clrscr();

    /* Start tempo loop */
    TCCR1B |= (1 << CS11); /* Start timer 1 at Fcpu /8 */

    while(1)
    {
#ifdef DEBUG_IDLE
        /* Count number of cycles for idle loop */
        timerIdle.begval = TCNT3;
#endif /* DEBUG_IDLE */

        if (pagePrv != page)
        {
            cpyStrLcdXY("                    ", 0, 0);
            cpyStrLcdXY("                    ", 0, 1);
            cpyStrLcdXY("                    ", 0, 2);
            cpyStrLcdXY("                    ", 0, 3);
        }

        switch (page)
        {
            default:
            case PAGE0:
            {
                cpyStrLcdXY("Tempo:", 0, 0);
                cpyTempoLcdXY(i16Tempo, 7, 0);

                cpyStrLcdXY("Gate:", 0, 1);
                cpyU8LcdXY((int16_t)rpg8Gate*(int16_t)100/48, BASE_10, 6, 1);
                cpyStrLcdXY("%", 10, 1);

                cpyStrLcdXY("Rate:", 0, 2);
                cpyRateLcdXY(rpg8Rate, 6, 2);

                cpyStrLcdXY("OctSft:", 0, 3);
                cpyI7LcdXY(rpg8OctSft, 8, 3);

                break;
            }
            case PAGE1:
            {
                cpyU8LcdXY(swtR.states[0], BASE_2, 0, 0);
                cpyU8LcdXY(swtR.states[1], BASE_2, 10, 0);

                cpyU8LcdXY(swtL.states[0], BASE_2, 0, 1);
                cpyU8LcdXY(swt8.states[0], BASE_2, 10, 1);

                cpyU8LcdXY(swtL.states[1], BASE_2, 0, 2);
                cpyU8LcdXY(swt8.states[1], BASE_2, 10, 2);

                cpyU8LcdXY(swtL.states[2], BASE_2, 0, 3);
                cpyU8LcdXY(swt8.states[2], BASE_2, 10, 3);

                break;
            }
            case PAGE2:
            {
#ifdef DEBUG_IDLE
                cpyStrLcdXY("    Idle:", 0, 0);
                cpyU16LcdXY(timerIdle.averag, 9, 0);
                cpyU16LcdXY(timerIdle.maxval, 15, 0);
#endif /* DEBUG_IDLE */

#ifdef DEBUG_IHM
                cpyStrLcdXY("     IHM:", 0, 1);
                cpyU16LcdXY(timerIHM.averag, 9, 1);
                cpyU16LcdXY(timerIHM.maxval, 15, 1);
#endif /* DEBUG_IHM */

#ifdef DEBUG_TEMPO
                cpyStrLcdXY("   Tempo:", 0, 2);
                cpyU16LcdXY(timerTempo.averag, 9, 2);
                cpyU16LcdXY(timerTempo.maxval, 15, 2);
#endif /* DEBUG_TEMPO */

#ifdef DEBUG_USART0RX
                cpyStrLcdXY(" MIDI IN:", 0, 3);
                cpyU16LcdXY(timerUsart0Rx.averag, 9, 3);
                cpyU16LcdXY(timerUsart0Rx.maxval, 15, 3);
#endif /* DEBUG_USART0RX */

#ifdef DEBUG_USART1
                cpyU16LcdXY(timerUsart1.averag, 9, 3);
                cpyU16LcdXY(timerUsart1.maxval, 15, 3);
#endif /* DEBUG_USART1 */
                break;
            }
            case PAGE3:
            {
//                cpyU16LcdXY(u16dbg1, 0, 1);
//                cpyU16LcdXY(u16dbg2, 6, 1);
//                cpyU16LcdXY(u16dbg3, 12, 1);

                cpyU8LcdXY(u8dbg1, BASE_10, 0, 1);
                cpyU8LcdXY(u8dbg2, BASE_10, 4, 1);
                cpyU8LcdXY(u8dbg3, BASE_10, 8, 1);
                cpyU8LcdXY(u8dbg4, BASE_10, 12, 1);
                cpyU8LcdXY(u8dbg5, BASE_10, 16, 1);

                cpyU8LcdXY(tMsgIn[0], BASE_10, 0, 2);
                cpyU8LcdXY(tMsgIn[1], BASE_10, 4, 2);
                cpyU8LcdXY(tMsgIn[2], BASE_10, 8, 2);

                cpyStrLcdXY(strDate, 0, 3);
                cpyStrLcdXY(strTime, 12, 3);
                break;
            }
        }
        /* refresh LCD */
        refreshLCD();

#ifdef DEBUG_IDLE
        /* Count number of cycles for idle loop */
        calcTimer(&timerIdle, 6, TCNT3);
#endif /* DEBUG_IDLE */
    }

    /* this code section is never reached */
    return 0;
}

#include "misc.h"
#include "rpg8.h"

/*--------------------------------------------------------------------------------------------------
    const variables
--------------------------------------------------------------------------------------------------*/
//const PROGMEM unsigned char copyRightChar[] =
//{
//	0x07, 0x08, 0x13, 0x14, 0x14, 0x13, 0x08, 0x07,
//	0x00, 0x10, 0x08, 0x08, 0x08, 0x08, 0x10, 0x00
//};

static const int8_t enc_states[] = {0,0,0,0,-1,0,0,1,1,0,0,-1,0,0,0,0};

/*--------------------------------------------------------------------------------------------------
    extern const variables
--------------------------------------------------------------------------------------------------*/
uint16_t u16dbg1 = 0;
uint16_t u16dbg2 = 0;
uint16_t u16dbg3 = 0;

uint8_t u8dbg1 = 0;
uint8_t u8dbg2 = 0;
uint8_t u8dbg3 = 0;
uint8_t u8dbg4 = 0;
uint8_t u8dbg5 = 0;

/*--------------------------------------------------------------------------------------------------
    functions definitions
--------------------------------------------------------------------------------------------------*/
uint32_t mittelwert(uint32_t newval, uint8_t factor, volatile uint32_t* pavgsum)
{
    // Filterlängen in 2er-Potenzen --> Compiler optimiert
    *pavgsum -= (*pavgsum >> factor);
    *pavgsum += newval;
    return (*pavgsum >> factor);
}

void calcTimer(TIMER* timer, uint8_t factor, uint16_t curval)
{
    uint16_t diff = curval - timer->begval;

    timer->maxval = MAX(timer->maxval, diff);
    timer->averag = mittelwert(diff, factor, &timer->buffer);
}

void myStrCpy(char * c, const char * constc)
{
    uint8_t constc_len = strlen(constc);

    for(uint8_t i = 0; i < constc_len; i++)
    {
//        if (c[i] != constc[i])
//        {
            c[i] = constc[i];
//        }
    }
}

void u8ToStrDec(uint8_t u8, char* str, uint8_t b)
{
    str[b]   = ((u8/100)%10) + 0x30;
    str[b+1] = ((u8/10)%10) + 0x30;
    str[b+2] = u8%10 + 0x30;
}

void u8ToStrMIDI(uint8_t u8, volatile uint8_t* str, uint8_t b)
{
    static const uint8_t MidiNoteChar1[] = { 'C', 'C', 'D', 'E', 'E', 'F', 'F', 'G', 'G', 'A', 'B', 'B' };
    static const uint8_t MidiNoteChar2[] = { ' ', '#', ' ', 'b', ' ', ' ', '#', ' ', '#', ' ', 'b', ' ' };

    if ( 0x80 <= u8 )
    {
        str[b] = ' ';
        str[b+1] = ' ';
        str[b+2] = ' ';
    }
    else
    {
        str[b] = MidiNoteChar1[u8%12];
        str[b+1] = MidiNoteChar2[u8%12];
        str[b+2] = u8/12 + 47;
    }
}

void u8ToStrBin(uint8_t u8, char* str, uint8_t b)
{
    str[b]   = ((u8 >> 7) & 0x01) + 0x30;
    str[b+1] = ((u8 >> 6) & 0x01) + 0x30;
    str[b+2] = ((u8 >> 5) & 0x01) + 0x30;
    str[b+3] = ((u8 >> 4) & 0x01) + 0x30;
    str[b+4] = ((u8 >> 3) & 0x01) + 0x30;
    str[b+5] = ((u8 >> 2) & 0x01) + 0x30;
    str[b+6] = ((u8 >> 1) & 0x01) + 0x30;
    str[b+7] = (u8 & 0x01) + 0x30;
}

void u16ToStrDec(uint16_t u16, char* str, uint8_t b)
{
    str[b]   = ((u16/10000)%10) + 0x30;
    str[b+1] = ((u16/1000)%10) + 0x30;
    str[b+2] = ((u16/100)%10) + 0x30;
    str[b+3] = ((u16/10)%10) + 0x30;
    str[b+4] = u16%10 + 0x30;
}

void i7ToStrDec(int8_t i7, char* str, uint8_t b)
{
    if (0 > i7)
    {
        str[b] = '-';
    }
    else
    {
        if (0 < i7)
        {
            str[b] = '+';
        }
        else
        {
            str[b] = ' ';
        }
    }
    str[b+1] = ((abs(i7)/10)%10) + 0x30;
    str[b+2] = abs(i7)%10 + 0x30;
}

void i16toStrTempo(int16_t i16Tempo, char* str, uint8_t b)
{
    str[b]   = ((i16Tempo/1000)%10) + 0x30;
    str[b+1] = ((i16Tempo/100)%10) + 0x30;
    str[b+2] = ((i16Tempo/10)%10) + 0x30;
    str[b+3] = '.';
    str[b+4] = i16Tempo%10 + 0x30;
}

void rpg8RateToStr(int8_t rpg8Rate, char* str, uint8_t b)
{
    str[b]   = '1';
    str[b+1] = '/';
    switch (rpg8Rate)
    {
    case RATE_1_1:   str[b+2] = '1'; str[b+3] = ' '; str[b+4] = ' '; break;
    case RATE_1_2:   str[b+2] = '2'; str[b+3] = ' '; str[b+4] = ' '; break;
    case RATE_1_2T:  str[b+2] = '2'; str[b+3] = 'T'; str[b+4] = ' '; break;
    case RATE_1_4:   str[b+2] = '4'; str[b+3] = ' '; str[b+4] = ' '; break;
    case RATE_1_4T:  str[b+2] = '4'; str[b+3] = 'T'; str[b+4] = ' '; break;
    case RATE_1_8:   str[b+2] = '8'; str[b+3] = ' '; str[b+4] = ' '; break;
    case RATE_1_8T:  str[b+2] = '8'; str[b+3] = 'T'; str[b+4] = ' '; break;
    case RATE_1_16:  str[b+2] = '1'; str[b+3] = '6'; str[b+4] = ' '; break;
    case RATE_1_16T: str[b+2] = '1'; str[b+3] = '6'; str[b+4] = 'T'; break;
    case RATE_1_32:  str[b+2] = '3'; str[b+3] = '2'; str[b+4] = ' '; break;
    case RATE_1_64:  str[b+2] = '6'; str[b+3] = '4'; str[b+4] = ' '; break;
    case RATE_1_128: str[b+2] = '1'; str[b+3] = '2'; str[b+4] = '8'; break;
    default:         str[b+2] = 'E'; str[b+3] = 'R'; str[b+4] = 'R'; break;
    }
}

void readSwt(PARALOAD* swt)
{
    int8_t i = 0;
    int8_t j = 0;
    uint8_t loc_states[Q7_COUNTMAX] = {0x00, 0x00, 0x00};

    *(swt->port) &=~(1 << swt->pout_pl); /* parallel load */
    *(swt->port) |= (1 << swt->pout_pl);

    /* all 74HC165 - Q7 */
    for (j = 0; j < Q7_COUNTMAX; j++)
    {
        if (PIN_POUT_NOT_USED != swt->pins_q7[j]) /* is the j-th pin used */
        {
            if ( !(*(swt->pins) & (1<<swt->pins_q7[j])) ) { loc_states[j] |= (0x01 << 7); }
        }
    }

    for (i = 6; 0 <= i; i--)
    {
        /* serial shift */
        *(swt->port) &=~(1 << swt->pout_cp); /* Horloge à 0 */
        *(swt->port) |= (1 << swt->pout_cp); /* Horloge à 1 */
        _delay_loop_1(1); /* pulse width tW in datasheet ? */

        /* all 74HC165 - Q6 to Q0 */
        for (j = 0; j < Q7_COUNTMAX; j++)
        {
            if (PIN_POUT_NOT_USED != swt->pins_q7[j])
            {
                if ( !(*(swt->pins) & (1<<swt->pins_q7[j])) ) { loc_states[j] |= (0x01 << i); }
            }
        }
    }

    for (j = 0; j < Q7_COUNTMAX; j++)
    {
        if (PIN_POUT_NOT_USED != swt->pins_q7[j])
        {
            swt->states[j] = loc_states[j]; /* UPDATE state */
            debounce(swt->states[j], &(swt->debs[j])); /* DEBOUNCE state */
        }
    }
}

void readRotEnc(ROTENC* rotEnc)
{
    rotEnc->oldAB = rotEnc->oldAB << 2; /* remember previous state */
    rotEnc->oldAB |= (   (   ( !!(rotEnc->p_swt->states[rotEnc->ind] & rotEnc->bitB) << 1 )
                           |   !!(rotEnc->p_swt->states[rotEnc->ind] & rotEnc->bitA)
                         )
                       & 0x03
                     ); /* add current state */
    rotEnc->state = enc_states[( rotEnc->oldAB & 0x0F )];
}

void putLeds( COMCPLED* led )
{
    int8_t i;
    int8_t j;

    if(    (led->vals[0] == led->vals_mem[0])
        && (led->vals[1] == led->vals_mem[1])
    )
    {
        return;
    }
    else
    {
        led->vals_mem[0] = led->vals[0];
        led->vals_mem[1] = led->vals[1];

        for (i = 7; 0 <= i; i--)
        {
            for (j = 0; j < DSA_COUNTMAX; j++)
            {
                if ( PIN_POUT_NOT_USED != led->pout_dsa[j]) /* is the j-th pout used ? */
                {
                    if ( !( led->vals[j] & (0x01 << i) ) ) /* Important note : inverted logic. FALSE => PORT = 1*/
                    {
                        *(led->port) |= ( 1 << led->pout_dsa[j] );
                    }
                    else
                    {
                        *(led->port) &= ~( 1 << led->pout_dsa[j] );
                    }
                }
            }
            _delay_loop_1(1);

            /* front montant sur CP */
            *(led->port) &=~(1 << led->pout_cp);
            *(led->port) |= (1 << led->pout_cp);
            _delay_loop_1(1);
        }
    }
}

void putLedsOn( COMCPLED* led
              , uint8_t bitsOn0
              , uint8_t bitsOn1
              )
{
    led->vals[0] |= bitsOn0; /* set led bits on designated dsa */
    led->vals[1] |= bitsOn1; /* set led bits on designated dsa */

    putLeds( led );
}

void putLedsOff( COMCPLED* led
               , uint8_t bitsOff0
               , uint8_t bitsOff1
               )
{
    led->vals[0] &= ~bitsOff0; /* reset led bits on designated dsa */
    led->vals[1] &= ~bitsOff1; /* reset led bits on designated dsa */

    putLeds( led );
}

void putLedsVal( COMCPLED* led
               , uint8_t bitsOverwrite0
               , uint8_t bitsOverwrite1
               )
{
    led->vals[0] = bitsOverwrite0; /* reset led bits on designated dsa */
    led->vals[1] = bitsOverwrite1; /* reset led bits on designated dsa */

    putLeds( led );
}

/* input: 8 bit variable.
   1 for a rising edge, -1 for a falling edge, 0 otherwise */
void debounce( uint8_t input, DEBOUNCE* pdeb )
{
    int8_t i;
    int8_t edg;

    for( i = 0; i < 8; i++ )
    {
        edg = 0;

        if( !!(input & (BIT0 << i)) )
        {
            (pdeb->cnt[i])++;
            if( pdeb->cntMax[i] <= pdeb->cnt[i] )
            {
                pdeb->cnt[i] = pdeb->cntMax[i];
            }
        }
        else
        {
            (pdeb->cnt[i])--;
            if(0 >= pdeb->cnt[i])
            {
                pdeb->cnt[i] = 0;
            }
        }

        if( pdeb->cntMax[i] <= pdeb->cnt[i] ) /* cnt is full */
        {
            if( !(pdeb->out & (BIT0 << i)) )
            {
                edg = 1; /* rising edge */
            }
            pdeb->out |= (BIT0 << i);
        }
        else
        {
            if( 0 >= pdeb->cnt[i] ) /* cnt is zero */
            {
                if( !!(pdeb->out & (BIT0 << i)) )
                {
                    edg = -1; /* falling edge */
                }
                pdeb->out &= ~(BIT0 << i);
            }
        }

        pdeb->edg[i] = edg;
    }
}

void bytebuf_add_sort(uint8_t b, BYTEBUF* buf)
{
    int8_t i;
    int8_t j;

    if ((buf->i) < BUFMAX)
    {
        /* case: b is smaller than smallest element - place it first */
        if ( b < buf->t[0] )
        {
            for(j = buf->i; 0 < j; j--)
            {
                buf->t[j] = buf->t[j-1];
            }
            buf->t[0] = b;
            (buf->i)++;
            return;
        }
        /* case: b is higher than smallest element - compare with i-th and (i+1)-th */
        for(i = 0; i < (buf->i - 1); i++)
        {
            if (    buf->t[i] < b
                 && b < buf->t[i+1]
               )
            {
                for(j = buf->i; (i+1) < j; j--)
                {
                    buf->t[j] = buf->t[j-1];
                }
                buf->t[i+1] = b;
                (buf->i)++;
                return;
            }
        }
        /* case: b is higher than all elements - place it at the end */
        buf->t[buf->i] = b;
        (buf->i)++;
    }
}

int8_t bytebuf_remove( uint8_t key
                     , BYTEBUF* buf
                     )
{
    int8_t i = 0;
    int8_t j = 0;

    if (0 < buf->i)
    {
        for(i = 0; i < buf->i; i++)
        {
            if ( key == buf->t[i] )
            {
                u8dbg4 = i;
                for(j = i; j < buf->i; j++)
                {
                    buf->t[j] = buf->t[j+1];
                    u8dbg5 = buf->t[j];
                }
                (buf->i)--;

                return 0;
            }
        }

        return -1;
    }

    return -2;
}

void bytebuf_queue(uint8_t b, BYTEBUF* buf)
{
    if ((buf->i) < BUFMAX)
    {
        buf->t[buf->i] = b;
        (buf->i)++;
    }
}

uint8_t bytebuf_dequeue(BYTEBUF* buf)
{
    int8_t i = 0;
    uint8_t out;

    out = buf->t[0];
    (buf->i)--;
    for (i = 0; i < (buf->i); i++)
    {
        buf->t[i] = buf->t[i+1];
    }
    return out;
}

void putC1(uint8_t c, BYTEBUF* buf)
{
    bytebuf_queue(c, buf);

    UCSR1B |= (1 << UDRIE1);
}

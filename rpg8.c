#include "misc.h"
#include "rpg8.h"

#include "midi.h"
#include "Sequenceur.h"

/*--------------------------------------------------------------------------------------------------
    extern variables
--------------------------------------------------------------------------------------------------*/
uint8_t tMsgIn[3] = {0xFF, 0xFF, 0xFF};
int8_t rpg8Rate = RATE_1_8;
int8_t rpg8Gate = 24;
int8_t rpg8OctSft = 0;
int8_t rpg8Oct = OCT_1;

/*--------------------------------------------------------------------------------------------------
    const
--------------------------------------------------------------------------------------------------*/
const int16_t rpg8RateTab[RATECARDINAL] =
{ 384 /* "1/1"   */
, 192 /* "1/2"   */
, 128 /* "1/2T"  */
, 96  /* "1/4"   */
, 64  /* "1/4T"  */
, 48  /* "1/8"   */
, 32  /* "1/8T"  */
, 24  /* "1/16"  */
, 16  /* "1/16T" */
, 12  /* "1/32"  */
, 6   /* "1/64"  */
, 3   /* "1/128" */
};

/*--------------------------------------------------------------------------------------------------
    static variables
--------------------------------------------------------------------------------------------------*/
static uint16_t cntMain = 0;
static uint8_t cOutp = 0;
static uint8_t bPendingNoteOn = FALSE;
static uint8_t rpg8Chn = 0;
static uint8_t rpg8Vel = 127;

static BYTEBUF nSort; /* sorted table of input MIDI notes */
static BYTEBUF nOcta; /* "octaved" table of input MIDI notes */
static BYTEBUF nOutp; /* output table of MIDI notes */

/*--------------------------------------------------------------------------------------------------
    functions
--------------------------------------------------------------------------------------------------*/
static void fOcta(const BYTEBUF * in, BYTEBUF * out)
{
    uint8_t u = 0;

    for (u = 0; u < in->i ; u++)
    {
        out->t[u] = in->t[u];
    }

    switch (rpg8Oct)
    {
        case OCT_1:
        default:
        {
            out->i = in->i;
            break;
        }
        case OCT_2:
        {
            out->i = 2*in->i;

            for (u = in->i; u < 2*in->i ; u++)
            {
                out->t[u] = in->t[u - in->i] + 12;
            }
            break;
        }
        case OCT_3:
        {
            out->i = 3*in->i;

            for (u = in->i; u < 2*in->i ; u++)
            {
                out->t[u] = in->t[u - in->i] + 12;
            }
            for (u = 2*in->i; u < 3*in->i ; u++)
            {
                out->t[u] = in->t[u - 2*in->i] + 24;
            }
            break;
        }
        case OCT_4:
        {
            out->i = 4*in->i;

            for (u = in->i; u < 2*in->i ; u++)
            {
                out->t[u] = in->t[u - in->i] + 12;
            }
            for (u = 2*in->i; u < 3*in->i ; u++)
            {
                out->t[u] = in->t[u - 2*in->i] + 24;
            }
            for (u = 3*in->i; u < 4*in->i ; u++)
            {
                out->t[u] = in->t[u - 3*in->i] + 36;
            }
            break;
        }
    }

    out->t[out->i] = 0; /* termination */
}

void rpg8In(uint8_t byteIn)
{
    static uint8_t tMsgInInd = 0;

//    putC1(byteIn, &buf1); /* send directly what is received <=> BYPASS */

    tMsgIn[tMsgInInd] = byteIn;

    tMsgInInd = tMsgInInd + 1;
    if (3 <= tMsgInInd)
    {
        /* 3 bytes have been received - MIDI message can be interpreted */

        tMsgInInd = 0;

        if( MIDI_NOTE_ON == (tMsgIn[0] & 0xF0) )
        {
            if( nSort.i < MAXKEYSPRESSED ) /* limit number of keys pressed at once */
            {
                bytebuf_add_sort(tMsgIn[1], &nSort);
            }
        }
        else if( MIDI_NOTE_OFF == (tMsgIn[0] & 0xF0) )
        {
            bytebuf_remove( tMsgIn[1]
                          , &nSort
                          );
        }

        fOcta(&nSort, &nOutp);

    }
}

/* This function should be called every 48th of a quarter note, sync with the tempo loop */
void rpg8Out()
{
    static uint8_t nOnKey;
    static uint8_t nOffKey;

    uint8_t cnt24ppqn;
    uint16_t cntSync;
    uint8_t cntBPM;

    uint16_t gOff = (uint16_t)rpg8Gate*(uint16_t)rpg8RateTab[rpg8Rate]/48;
    u16dbg1 = cntMain;

    /* MIDI clock 24 ppqn */
    cnt24ppqn = cntMain & 0x0003;
    if (0 == cnt24ppqn)
    {
        putC1(MIDI_COMMON_CLOCK, &buf1);
    }

    /* output notes in sync with rpg8Rate */
    cntSync = cntMain % rpg8RateTab[rpg8Rate];
    if( 0 == cntSync )
    {
        putLedsOn(&ledR, 0, BIT7);

        if ( nOutp.i <= cOutp )
        {
            cOutp = 0;
        }

        /*
        To avoid two notes being sent in a row, we get rid of any pending note off now.
        */
        if(bPendingNoteOn)
        {
            sendMidiMsgC1_a(MIDI_NOTE_OFF | rpg8Chn, nOffKey, 64);
            bPendingNoteOn = FALSE;
        }

        if (0 < nOutp.i)
        {
            nOnKey = nOutp.t[cOutp] + rpg8OctSft*NBRSEMTONESPEROCT;
            sendMidiMsgC1_a(MIDI_NOTE_ON | rpg8Chn, nOnKey, rpg8Vel);

            bPendingNoteOn = TRUE;
            nOffKey = nOnKey;
        }

        cOutp++;
    }
    else
    {
        if ( gOff <= cntSync ) /* '<=' is important for avoiding lost notes Off while changing the gate value */
        {                          /* 'else if' implies that a note last at least 1/48th of a note */
            putLedsOff(&ledR, 0, BIT7);

            if(bPendingNoteOn)
            {
                sendMidiMsgC1_a(MIDI_NOTE_OFF | rpg8Chn, nOffKey, 64);
                bPendingNoteOn = FALSE;
            }
        }
    }

    /* sync with the BPM */
    cntBPM = cntMain % 96;
    if( 0 == cntBPM )
    {
        putLedsOn(&ledL, 0xF0, 0);
    }
    else
    {
        putLedsOff(&ledL, 0xF0, 0);
    }

    cntMain++;
    if ( 96*4*4 <= cntMain )
    {
        cntMain = 0;
    }
}

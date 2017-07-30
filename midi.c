#include "misc.h"
#include "midi.h"

BYTEBUF buf1 = {{},0};

void sendMidiMsgC1_a( uint8_t b0
                    , uint8_t b1
                    , uint8_t b2
                    )
{
    putC1(b0, &buf1);
    putC1(b1, &buf1);
    putC1(b2, &buf1);
}

void sendMidiMsgC1_b( uint8_t* t3Bytes )
{
    int i = 0;

    for (i = 0; i < 3; i++)
    {
        putC1(t3Bytes[i], &buf1);
    }
}

void sendMidiNoteC1_a( uint8_t sts
                     , uint8_t chn
                     , uint8_t key
                     , uint8_t vel
                     )
{
    putC1((sts & 0xF0) | (chn & 0x0F), &buf1);
    putC1(key, &buf1);
    putC1(vel, &buf1);
}

//void sendMidi1Note(const MIDINOTE* n)
//{
//    putC1((n->sts & 0xF0) | (n->chn & 0x0F), &buf1);
//    putC1(n->key, &buf1);
//    putC1(n->vel, &buf1);
//}


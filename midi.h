#ifndef MIDI_H_INCLUDED
#define MIDI_H_INCLUDED

#define MIDI_NOTE_OFF               0x80
#define MIDI_NOTE_ON                0x90
#define MIDI_NOTE_PRESSURE          0xA0
#define MIDI_CONTROL                0xB0
#define MIDI_PGM_CHANGE             0xC0
#define MIDI_CHANNEL_PRESSURE       0xD0
#define MIDI_BENDER                 0xE0
#define MIDI_COMMON_SYSEX           0xF0
#define MIDI_COMMON_MTC_QUARTER     0xF1
#define MIDI_COMMON_SONG_POS        0xF2
#define MIDI_COMMON_SONG_SELECT     0xF3
#define MIDI_COMMON_TUNE_REQUEST    0xF6
#define MIDI_COMMON_SYSEX_END       0xF7
#define MIDI_COMMON_CLOCK           0xF8
#define MIDI_COMMON_START           0xFA
#define MIDI_COMMON_CONTINUE        0xFB
#define MIDI_COMMON_STOP            0xFC
#define MIDI_COMMON_SENSING         0xFE
#define MIDI_COMMON_RESET           0xFE

#define MIDI_DATA1_ALL_NOTES_OFF        0xB0
#define MIDI_DATA2_ALL_NOTES_OFF        0x00

#define MIDIOUT0 0x01
#define MIDIOUT1 0x02
#define MIDIOUT2 0x04
#define MIDIOUT3 0x08

typedef struct
{
    uint8_t sts; /* status (8bits) */
    uint8_t chn; /* channel (8bits) */
    uint8_t key; /* 0 - 127 (7 bits) */
    uint8_t vel; /* 0 - 127 (7 bits) */
} MIDINOTE;

/*--------------------------------------------------------------------------------------------------
    extern variables
--------------------------------------------------------------------------------------------------*/
extern BYTEBUF buf1;

/*--------------------------------------------------------------------------------------------------
    functions protoypes
--------------------------------------------------------------------------------------------------*/
void sendMidiMsgC1_a( uint8_t b0
                    , uint8_t b1
                    , uint8_t b2
                    );
void sendMidiMsgC1_b( uint8_t* t3Bytes );
void sendMidiNoteC1_a( uint8_t sts
                     , uint8_t chn
                     , uint8_t key
                     , uint8_t vel
                     );

#endif // MIDI_H_INCLUDED

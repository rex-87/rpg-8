#ifndef RPG8_H_INCLUDED
#define RPG8_H_INCLUDED

/*--------------------------------------------------------------------------------------------------
    typedef
--------------------------------------------------------------------------------------------------*/
typedef struct
{
    uint8_t chn;
    uint8_t key;
    uint8_t vel;
} RPG8NOTE;

/*--------------------------------------------------------------------------------------------------
    defines/enums
--------------------------------------------------------------------------------------------------*/
#define RATECARDINAL 12
enum
{
    RATE_1_1 = 0
,   RATE_1_2
,   RATE_1_2T
,   RATE_1_4
,   RATE_1_4T
,   RATE_1_8
,   RATE_1_8T
,   RATE_1_16
,   RATE_1_16T
,   RATE_1_32
,   RATE_1_64
,   RATE_1_128
};

enum
{
    OCT_1 = 0
,   OCT_2
,   OCT_3
,   OCT_4
};

#define MAXKEYSPRESSED 8
#define NBRSEMTONESPEROCT 12

/*--------------------------------------------------------------------------------------------------
    extern variables
--------------------------------------------------------------------------------------------------*/
extern uint8_t tMsgIn[3];
extern int8_t rpg8Gate;
extern int8_t rpg8Rate;
extern int8_t rpg8OctSft;

const int16_t rpg8RateTab[RATECARDINAL];

/*--------------------------------------------------------------------------------------------------
    functions prototypes
--------------------------------------------------------------------------------------------------*/
void rpg8In(uint8_t byteIn);
void rpg8Out();

#endif // RPG8_H_INCLUDED

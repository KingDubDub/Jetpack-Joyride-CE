
#include <stddef.h>
#include <stdint.h>

#include "headers.h"



// After all these years I still can't create an array of dynamic length char arrays at compile
// time, so this long/gross method of an array of pointers will have to do.
const char txt0[]  = "      YEP, THIS IS ON A CALCULATOR.";
const char txt1[]  = "I FIRST LEARNED TO PROGRAM MAKING";
const char txt2[]  = "PONG ON THE TI-84 + CE, AND THEN";
const char txt3[]  = "IMMEDIATELY STARTED THIS PROJECT AS";
const char txt4[]  = "A REAL TEST OF MY SKILLS. I'VE BEEN";
const char txt5[]  = "MAKING THIS AS ACCURATE AS POSSIBLE";
const char txt6[]  = "AND AS A LEARNING RESOURCE FOR";
const char txt7[]  = "OTHERS. IT'S BEEN A LONG TIME COMING,";
const char txt8[]  = "AND HERE IT IS.";

const char txt9[]  = "I'D LIKE TO THANK THE FOLLOWING:";
const char txt10[] = "HALFBRICK, WHO MADE THE GAME.";

const char txt11[] = "TESTING AND FEEDBACK: TINY_HACKER,";
const char txt12[] = "ROCCOLOX, RANDOMGUY, AND";
const char txt13[] = "THELASTMILLENIAL.";

const char txt14[] = "CODING HELP: COMMANDBLOCKGUY,";
const char txt15[] = "MATEO CON LECHUGA, LIONEL DEBROUX,";
const char txt16[] = "KRYPTONICDRAGON, AND CEMETECH.NET.";

const char txt17[] = "MOTIVATION: LIL' MATTY AND";
const char txt18[] = "SCRANT, WHO GOT ME TO FINISH THIS.";

const char txt19[] = "THANKS FOR PLAYING, CHECK OUT THE";
const char txt20[] = "PROJECT HERE AND REPORT ANY BUGS:";

const char txt21[] = "HTTPS://CEMETECH.NET/FORUM/";
const char txt22[] = "VIEWTOPIC.PHP?T=16984";


const char blank[] = {0};

const char *about_txt[] =
{
    txt0,
    txt1,
    txt2,
    txt3,
    txt4,
    txt5,
    txt6,
    txt7,
    txt8,
        blank,
    txt9,
    txt10,
        blank,
    txt11,
    txt12,
    txt13,
        blank,
    txt14,
    txt15,
    txt16,
        blank,
    txt17,
    txt18,
        blank,
    txt19,
    txt20,
        blank,
    txt21,
    txt22,
        blank
};

//coin formation type arrays
const uint8_t coin_x_0[] =
{
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108
};

const uint8_t coin_x_1[] =
{
    0,     24,   48, 60,
    0,     24,   48,     72,
    0, 12, 24,   48, 60,
    0,     24,   48,     72,
    0,     24,   48, 60
};

const uint8_t coin_x_2[] =
{
    0, 12,             72, 84,               144, 156,
    0, 12,   36, 48,   72, 84,   108, 120,   144, 156,   180, 192,
             36, 48,             108, 120,               180, 192
};

const uint8_t coin_x_3[] =
{
                               84,
                               84, 96,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
                               84, 96,
                               84
};

const uint8_t coin_x_4[] =
{
           24,
       12, 24, 36, 48,
    0,     24,
       12, 24, 36,
           24,     48,
    0, 12, 24, 36,
           24
};

const uint8_t coin_x_5[] =
{
    0,     24,     48,     72,     96,
       12,     36,     60,     84,     108,
    0,     24,     48,     72,     96,
       12,     36,     60,     84,     108,
    0,     24,     48,     72,     96,
       12,     36,     60,     84,     108,
};

const uint8_t coin_y_0[] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

const uint8_t coin_y_1[] =
{
    0,      0,    0,  0,
    12,     12,   12,    12,
    24, 24, 24,   24, 24,
    36,     36,   36,    36,
    48,     48,   48, 48
};

const uint8_t coin_y_2[] =
{
    0,  0,              0,  0,              0,  0,
    12, 12,   12, 12,   12, 12,   12, 12,   12, 12,   12, 12,
              24, 24,             24, 24,             24, 24
};

const uint8_t coin_y_3[] =
{
                                0,
                                12, 12,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
                                48, 48,
                                60
};

const uint8_t coin_y_4[] =
{
            0,
        12, 12, 12, 12,
    24,     24,
        36, 36, 36,
            48,     48,
    60, 60, 60, 60,
            72,
};

const uint8_t coin_y_5[] =
{
    0,      0 ,     0 ,     0 ,     0 ,
        12,     12,     12,     12,     12,
    24,     24,     24,     24,     24,
        36,     36,     36,     36,     36,
    48,     48,     48,     48,     48,
        60,     60,     60,     60,     60
};

const uint8_t coin_max[6] =
{
    sizeof(coin_x_0),
    sizeof(coin_x_1),
    sizeof(coin_x_2),
    sizeof(coin_x_3),
    sizeof(coin_x_4),
    sizeof(coin_x_5)
};

const uint8_t *coin_x[sizeof(coin_max)] =
{
    coin_x_0,
    coin_x_1,
    coin_x_2,
    coin_x_3,
    coin_x_4,
    coin_x_5
};

const uint8_t *coin_y[sizeof(coin_max)] =
{
    coin_y_0,
    coin_y_1,
    coin_y_2,
    coin_y_3,
    coin_y_4,
    coin_y_5
};

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
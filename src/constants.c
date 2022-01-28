/* constants.c

All variables and datatypes for Jetpack Joyride CE that are hard-coded and used by functions and such.

*/

#include <stddef.h>
#include <stdint.h>

#include "headers.h"

/* --- Strings --- */

//There's got to be a better way.
const char txt0[] = "YES, THIS IS A CALCULATOR. WHEN I WAS";
const char txt1[] = "SMOL, I SAW SOME HIGHSCOLLERS TRYING";
const char txt2[] = "TO PUT OIRAM ON THEIR CALCULATORS.";
const char txt3[] = "FROM THAT TIME I WANTED TO PLAY AND";
const char txt4[] = "MAKE GAMES FOR STUFF THAT WASN'T";
const char txt5[] = "MADE FOR ENTERTAINMENT. THIS DUMB";
const char txt6[] = "PROJECT WAS GOING TO BE AN EXCERCISE";
const char txt7[] = "OF MY SKILLZ THAT I TURNED INTO A";
const char txt8[] = "FULL PORT OF JETPACK JOYRIDE, WHICH";
const char txt9[] = "IS OWNED BY HALFBRICK STUDIOS (PLS NO";
const char txt10[] = "SUE). GO PLAY THE ORIGINAL, IT'S MORE";
const char txt11[] = "FUN; THIS IS JUST MILDLY BLURSED. I";
const char txt12[] = "ALSO WANNA SAY THANKS TO EVERYBODY";
const char txt13[] = "WHO TAUGHT ME HOW TO USE THE TOOLS I";
const char txt14[] = "HAD AND ENABLED MY IDIOCY, THIS IS ALL";
const char txt15[] = "THEIR FAULT:";

const char txt16[] = "TESTING AND FEEDBACK: TINY_HACKER,";
const char txt17[] = "ROCCOLOX, RANDOMGUY, AND";
const char txt18[] = "THELASTMILLENIAL.";

const char txt19[] = "CODING HELP: COMMANDBLOCKGUY, MATEO";
const char txt20[] = "CON LECHUGA, LIONEL DEBROUX, AND";
const char txt21[] = "KRYPTONICDRAGON.";

const char txt22[] = "MOTIVATION: LITERALLY EVERYONE ON";
const char txt23[] = "CEMETECH, THEY'RE GOOD FOLKS.";

const char txt24[] = "I HOPE YOU ENJOY THIS HACKNEYED MESS";
const char txt25[] = "I'VE MADE, POKE ME ON CEMETECH.NET";
const char txt26[] = "OR SUBMIT AN ISSUE ON GITHUB IF YOU";
const char txt27[] = "FIND ANY BUGS OR GRAMMAR MISTAKES!";

//The shortlink sends you through 8+ redirects to a catgirl gacha game page if typed in caps, so I can't use it with the font.
//That was such a sentence I had to keep it; If I were a chaotic evil, I would've left the link in the game until someone noticed.
//const char txt28[] = "https://git.io/JRhEp";

const char txt28[] = "HTTPS://CEMETECH.NET/FORUM/";
const char txt29[] = "VIEWTOPIC.PHP?T=16984";

//const char txt17[] = "All your RAM is belong to us!";

//I'm surprised that NULL or 0 don't work for line breaks, but as the purple man once said: "Fine, I'll do it myself"
const char blank[] = {0};

const char *about_txt[36] =
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
    txt9,
    txt10,
    txt11,
    txt12,
    txt13,
    txt14,
    txt15,
        blank,
    txt16,
    txt17,
    txt18,
        blank,
    txt19,
    txt20,
    txt21,
        blank,
    txt22,
    txt23,
        blank,
    txt24,
    txt25,
    txt26,
    txt27,
        blank,
    txt28,
    txt29,
        blank,
};

//the max number of coins in each shape, used so the coin-calculation runs through the minimum
//number of coins
const uint8_t coin_max[COIN_FORMATIONS] =
{
    30,
    21,
    24,
    26,
    17,
    30
};

//coin formation type arrays
const uint24_t coin_form_x[COIN_FORMATIONS][MAX_COINS] =
{{
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108
},{
    0,     24,   48, 60,
    0,     24,   48,     72,
    0, 12, 24,   48, 60,
    0,     24,   48,     72,
    0,     24,   48, 60
},{
    0, 12,             72, 84,               144, 156,
    0, 12,   36, 48,   72, 84,   108, 120,   144, 156,   180, 192,
             36, 48,             108, 120,               180, 192
},{
                               84,
                               84, 96,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108,
                               84, 96,
                               84
},{
           24,
       12, 24, 36, 48,
    0,     24,
       12, 24, 36,
           24,     48,
    0, 12, 24, 36,
           24
},{
    0,     24,     48,     72,     96,
       12,     36,     60,     84,     108,
    0,     24,     48,     72,     96,
       12,     36,     60,     84,     108,
    0,     24,     48,     72,     96,
       12,     36,     60,     84,     108,
}};

const uint8_t coin_form_y[COIN_FORMATIONS][MAX_COINS] =
{{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24
},{
    0,      0,    0,  0,
    12,     12,   12,    12,
    24, 24, 24,   24, 24,
    36,     36,   36,    36,
    48,     48,   48, 48
},{
    0,  0,              0,  0,              0,  0,
    12, 12,   12, 12,   12, 12,   12, 12,   12, 12,   12, 12,
              24, 24,             24, 24,             24, 24
},{
                                0,
                                12, 12,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
                                48, 48,
                                60
},{
            0,
        12, 12, 12, 12,
    24,     24,
        36, 36, 36,
            48,     48,
    60, 60, 60, 60,
            72,
},{
    0,      0 ,     0 ,     0 ,     0 ,
        12,     12,     12,     12,     12,
    24,     24,     24,     24,     24,
        36,     36,     36,     36,     36,
    48,     48,     48,     48,     48,
        60,     60,     60,     60,     60
}};

//max number of lasers per formation, so only the ones used are updated
const uint8_t formation_max_lasers[LASER_FORMATIONS] =
{
    1,
    2,
    4,
    4,
    7
};

//Y-coords, no X's needed
const uint8_t laser_y[LASER_FORMATIONS][MAX_LASERS] =
{{
    114
},{
    24,
    204
},{
    24,
    144,
    174,
    204
},{
    24,
    54,
    84,
    204
},{
    24,
    54,
    84,
    114,
    144,
    174,
    204
}};

//TTL for lasers, it's the last 108 cycles that count
const uint8_t half_life[LASER_FORMATIONS][MAX_LASERS] =
{{
    108
},{
    108,
    108
},{
    108,
    108,
    108,
    108
},{
    108,
    108,
    108,
    108
},{
    108,
    108,
    216,
    216,
    216,
    108,
    108
}};
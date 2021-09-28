/* constants.c

All variables and datatypes for Jetpack Joyride CE that are hard-coded and used by functions and such.

*/

#include <stddef.h>
#include <stdint.h>

#include "headers.h"

/* --- Strings --- */

//There's got to be a better way:
const char txt0[] = "Yes, this is a calculator. When I was";
const char txt1[] = "smol, I saw some highschoolers trying";
const char txt2[] = "to put Oiram on their calculators.";
const char txt3[] = "From that time I wanted to play and";
const char txt4[] = "make games on stuff that wasn't";
const char txt5[] = "made for entertainment. This dumb";
const char txt6[] = "project was going to be an exercise";
const char txt7[] = "of my skillz that I turned into a";
const char txt8[] = "full port of Jetpack Joyride, which";
const char txt9[] = "is owned by Halfbrick Studios (pls no";
const char txt10[] = "sue). Go play the original, it's more";
const char txt11[] = "fun; this is just mildly blursed. I";
const char txt12[] = "also wanna say thanks to everybody";
const char txt13[] = "who taught me how to use the tools I";
const char txt14[] = "had and enabled my idiocy, this is all";
const char txt15[] = "their fault:";

const char txt16[] = "Testing and feedback: TIny_Hacker,";
const char txt17[] = "RoccoLox, and TheLastMillenial.";

const char txt18[] = "Coding help: CommandBlockGuy, Mateo";
const char txt19[] = "Con Lechuga, and KryptonicDragon.";

const char txt20[] = "Motivation: literally everyone on";
const char txt21[] = "Cemetech, they're good folks.";

const char txt22[] = "I hope you enjoy this hackneyed mess";
const char txt23[] = "I've made, poke me on Cemetech.net";
const char txt24[] = "or submit an issue on GitHub if you";
const char txt25[] = "find any bugs or grammar mistakes!";

//The shortlink sends you through 8+ redirects to a catgirl gacha game page if typed in caps, so I can't use it with the font.
//That was such a sentence I had to keep it; If I were a chaotic evil, I would've left the link in the game until someone noticed.
const char txt26[] = "https://git.io/JRhEp";

const char txt27[] = "https://cemetech.net/forum/";
const char txt28[] = "viewtopic.php?t=16984";

//const char txt17[] = "All your RAM is belong to us!";

//I'm surprised that NULL or 0 don't work for line breaks, but as the purple man once said: "Fine, I'll do it myself"
const char blank[] = {0};

const char *about_txt[34] =
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
        blank,
    txt18,
    txt19,
        blank,
    txt20,
    txt21,
        blank,
    txt22,
    txt23,
    txt24,
    txt25,
        blank,
    txt27,
    txt28,
        blank,
};

//the max number of coins in each shape, used so the coin-calculation runs through the minimum
//number of coins:
const uint8_t coin_max[COIN_FORMATIONS] =
{
    30,
    21,
    24,
    26,
    17,
    30
};

//coin formation type arrays:
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

//max number of lasers per formation, so only the ones used are updated:
const uint8_t formation_max_lasers[LASER_FORMATIONS] =
{
    1,
    2,
    4,
    4,
    7
};

//Y-coords, no X's needed:
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

//TTL for lasers, it's the last 108 cycles that count:
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
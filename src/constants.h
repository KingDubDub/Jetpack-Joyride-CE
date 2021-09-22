/* constants.c

All variables and datatypes for Jetpack Joyride CE that are hard-coded and used by functions and such.

*/

#include <stddef.h>
#include <stdint.h>

/* --- Defines --- */

#define DATA_APPVAR    "JTPKDAT"
#define APPVAR_VERSION 4

//HEY FUTURE ME, REMEMBER TO UPDATE THE VERSION WHEN WE ADD STUFF!
//SCREW YOU PAST-FUTURE ME, hopefully future-future me does better.
//No, we definitely did not.

//max number of various obstacles that are allowed to spawn, mostly used for array sizes:
#define MAX_ZAPPERS  3
#define MAX_MISSILES 1
#define MAX_LASERS   7

//the starting X-coords for various obstacles and things:
#define COIN_ORIGIN    330
#define ZAPPER_ORIGIN  352
#define MISSILE_ORIGIN 1466

/* --- Types --- */

typedef struct
{
    uint8_t health;       //the number of hits Barry can take (increased with vehicles and shields)
    uint32_t monies;      //money collected in the run
    uint32_t college_fund; //total money collected from all runs, can be used to purchase stuff (when can I add this?)
    uint32_t distance;    //distance travelled in the run, measured in pixels
    uint32_t highscore;   //highest distance travelled in all runs
}
game_data_t;

//all the data used to control Barry's position, acceleration, etc. are consolidated under one pointer.
//x and y positions, time 2nd is pressed/released to fly/fall, animation frame, animation cycling control,
//and jetpack animation frame:
typedef struct
{
    int24_t x;
    uint8_t y;
    uint8_t theta; //in calculator land, there're only 256 degrees, not 360 oogabooga degrees.
    int8_t  input_duration;
    uint8_t player_animation;
    uint8_t player_animation_toggle;
    uint8_t exhaust_animation;
    uint8_t corpse_bounce;
    uint8_t death_delay;
}
avatar_t; //looks gross, but it saves 309 bytes in read/write calls to the appvar. It's staying.

typedef struct
{
    int24_t x;
    uint8_t y;
    int8_t  h_accel; //horizontal acceleration
    int8_t  v_accel; //vertical acceleration
    uint8_t theta;
    uint8_t bounce;
}
jetpack_t;

//hey looky, a coin struct type for easy use, WHY DIDN'T I MAKE THIS EARLIER?
//stores x and y positions, along with the coin's animation frame:
typedef struct
{
    int24_t x[30];
    uint8_t y[30];
    uint8_t animation[30];

    //coin formation variable to keep track of coin list shapes:
    uint8_t formation;
}
coin_t;

//again, WHY did I not make these before... I think there was a reason...
//laser x and y positions, length, and animation frame:
typedef struct
{
    int24_t x[MAX_ZAPPERS];
    uint8_t y[MAX_ZAPPERS];
    int8_t length[MAX_ZAPPERS];

    //zapper type, vertical, horizontal, or positve/negative diagonal:
    int8_t orientation[MAX_ZAPPERS];

    //zapper sprite animation frame:
    int8_t animate;
}
zapper_t;

//I believe the ZDS toolchain broke them and doubled the program size, praise God I upgraded to LLVM.
//missile x and y positions:
typedef struct
{
    int24_t x[MAX_MISSILES];
    uint8_t y[MAX_MISSILES];

    //keep track of animations for missiles:
    int8_t icon_animate;
    int8_t animation;
    int8_t animation_toggle;
}
missile_t;

//the point is, updates are good; ergo, Windows is exciting
//y position (x position is shared among all lasers), laser's time to function:
typedef struct
{
    uint8_t y[7];
    uint24_t lifetime[7];

    //all lasers have a universal X that doesn't move very far:
    int8_t x;
    //laser animation sprite:
    int8_t animation;
    //which laser formation is being used:
    uint8_t formation;
    //keep track of how many lasers have fired already:
    uint8_t deactivated;
}
laser_t;

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

//global maximum coins, technically redundant but still useful for debugging:
#define MAX_COINS 30

#define COIN_FORMATIONS 6

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

//max number of... do I really need to explain this one?
#define LASER_FORMATIONS 5

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
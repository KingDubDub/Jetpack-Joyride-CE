/*

Optimized and "complete" functions and defines used in Jetpack Joyride CE, but just because they've been
set aside doesn't mean they aren't perfect yet.

*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <tice.h>
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>

//#include "constants.c"
#include "gfx/jetpackia.h"

//name of the savedata appvar:
#define DATA_APPVAR "JTPKDAT"

//max number of various obstacles that are allowed to spawn:
#define MAX_ZAPPERS 3
#define MAX_MISSILES 1

//the starting X-coords for various obstacles and things:
#define COIN_ORIGIN 330
#define ZAPPER_ORIGIN 352
#define MISSILE_ORIGIN 1466

typedef struct
{
    uint8_t health;       //the number of hits Barry can take (increased with vehicles and shields)
    uint32_t monies;      //money collected in the run
    uint32_t collegeFund; //total money collected from all runs, can be used to purchase stuff (when can I add this?)
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
    int8_t inputDuration;
    uint8_t playerAnimation;
    uint8_t playerAnimationToggle;
    uint8_t exhaustAnimation;
    uint8_t corpseBounce;
    uint8_t deathDelay;
}
avatar_t; //looks gross, but it saves 309 bytes in read/write calls to the appvar. It's staying.

typedef struct
{
    int24_t x;
    uint8_t y;
    int8_t h_accel; //horizontal acceleration
    int8_t v_accel; //vertical acceleration
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
    int8_t iconAnimate;
    int8_t animation;
    int8_t animationToggle;
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
//That was such a sentence I had to keep it; If I were a chaotic evil, I would've left the link in the game.
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
const uint24_t ctx[COIN_FORMATIONS][MAX_COINS] =
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

const uint8_t cty[COIN_FORMATIONS][MAX_COINS] =
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

//max number of lasers, used for initializing:
#define MaxLasers 7

//max number of... do I really need to explain this one?
#define LASER_FORMATIONS 5

//max number of lasers per formation, so only the ones used are updated:
const uint8_t laserMax[LASER_FORMATIONS] =
{
    1,
    2,
    4,
    4,
    7
};

//Y-coords, no X's needed:
const uint8_t lsrY[LASER_FORMATIONS][MaxLasers] =
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

//TTL for lasers, it's the last 120 cycles that count:
const uint8_t halfLife[LASER_FORMATIONS][MaxLasers] =
{{
    120
},{
    120,
    120
},{
    120,
    120,
    120,
    120
},{
    120,
    120,
    120,
    120
},{
    120,
    120,
    232,
    232,
    232,
    120,
    120
}};

//all the externs from the main program (they don't need to be clean, who looks in here anyway?):
extern game_data_t save_data;
extern const uint8_t APPVAR_VERSION;
extern int8_t scrollSpeed;
extern uint16_t incrementDelay;
extern int24_t bg_scroll;
extern uint8_t bg_list[9];
extern uint8_t secondary_bg_list[9];
extern int24_t spawnDelay;
extern avatar_t avatar;
extern jetpack_t jetpackEntity;
extern coin_t coins;
extern zapper_t zappers;
extern missile_t missiles;
extern laser_t lasers;
extern uint8_t opening_delay;

//gfx_CheckRectangleHotspot but only the Y checks, since 4 C++ masters write better code than me:
#define Yspot(master_y, master_height, test_y, test_height) \
    (((test_y) < ((master_y) + (master_height))) && \
    (((test_y) + (test_height)) > (master_y)))

//Mmm smells like stolen code... adapted from StackOverflow, fills an array of a given size with a given value very quickly:
#define Fill_Array(array, entries, value) \
do{ \
    size_t n = entries; \
    for(size_t i = 0; i < n; ++i) (array)[i] = value; \
} while (0);
//NOTE: initial tests saw smaller size than memset and equivalent speed. Further testing will be done when I feel like it.

//takes an input sprite and pastes it into another sprite at given coordinates:
void CopyPasta(const gfx_sprite_t *spriteIn, gfx_sprite_t *spriteOut, uint24_t x, uint8_t y)
{
    const uint24_t widthIn = spriteIn->width;
    const uint24_t spriteIn_size = spriteIn->height * widthIn;
    const uint24_t widthOut = spriteOut->width;
    uint24_t start_write = (widthOut * y) + x;

    //write out input sprite row by row into the output sprite:
    for(uint24_t j = 0; j < spriteIn_size; j += widthIn)
    {
        //copy the row of the input to the position needed in the output:
        memcpy(&spriteOut->data[start_write], &spriteIn->data[j], widthIn);

        //add the output sprite's width to move to the next row plus the given X that was added at the start:
        start_write += widthOut;
    }
}

//a function for drawing buttons, will hopefully save on flash size and stack usage:
void draw_button(gfx_sprite_t *sprites[], char *text, uint8_t buttonSelect)
{
    //first 14 pixels of the button:
    gfx_Sprite_NoClip(sprites[buttonSelect], 70, 33 + (buttonSelect * 60));

    //I'm up to my cheaty tricks again, I turn the 14th column of pixels into 152 columns:
    gfx_CopyRectangle(gfx_buffer, gfx_buffer, 83, 33 + (buttonSelect * 60), 84, 33 + (buttonSelect * 60), 152, 50);

    //and the last 14 pixels:
    gfx_Sprite_NoClip(sprites[3], 236, 33 + (buttonSelect * 60));

    //words 'n stuff:
    gfx_SetTextFGColor(2);
    gfx_SetTextScale(3, 3);

    //pretty much all the letters are odd numbers of pixels wide or tall, so that sucks:
    gfx_PrintStringXY(text, 160 - gfx_GetStringWidth(text)/2, 47 + (buttonSelect * 60));
}

//set the background tiles starting at a given point:
void Set_Background(const uint8_t start)
{
    for(uint8_t i = 0; i < + 9; ++i)
    {
        bg_list[i] = start + i;
        secondary_bg_list[i] = bg_list[i];
    }
}

//this is purely for streamlining, it should only used twice:
void save_state(void)
{
    //create a new appvar, erases the old one and writes the variable data to it:
    ti_var_t savegame = ti_Open(DATA_APPVAR, "w");

    //game save data and stats:
    ti_Write(&save_data, sizeof(save_data), 1, savegame);

    //version of the appvar, for future use with updater fix programs and hopefully help with debugging:
    ti_Write(&APPVAR_VERSION, 1, 1, savegame);

    //game environment variables:
    ti_Write(&scrollSpeed, sizeof(scrollSpeed), 1, savegame);
    ti_Write(&incrementDelay, sizeof(incrementDelay), 1, savegame);
    ti_Write(&bg_scroll, sizeof(bg_scroll), 1, savegame);
    ti_Write(&bg_list, sizeof(bg_list), 1, savegame);
    ti_Write(&secondary_bg_list, sizeof(secondary_bg_list), 1, savegame);
    ti_Write(&spawnDelay, sizeof(spawnDelay), 1, savegame);

    //single pointer to avatar struct:
    ti_Write(&avatar, sizeof(avatar), 1, savegame);

    //jetpack variables:
    ti_Write(&jetpackEntity, sizeof(jetpackEntity), 1, savegame);

    //coin variables:
    ti_Write(&coins, sizeof(coins), 1, savegame);

    //zapper variables:
    ti_Write(&zappers, sizeof(zappers), 1, savegame);

    //missile variables:
    ti_Write(&missiles, sizeof(missiles), 1, savegame);

    //laser variables:
    ti_Write(&lasers, sizeof(lasers), 1, savegame);

    ti_Write(&opening_delay, sizeof(opening_delay), 1, savegame);

    //uncomment this in the NEAR FINAL release, right now it should a temporary save until I iron out the bugs:
    //ti_SetArchiveStatus(true, savegame);

    ti_Close(savegame);
}

//A function for returning the pointers of tileset sprites in appvars with 2-byte LookUp Table (LUT) entries,
//this only works if the appvar only has single images loaded into it, but you can use a single tileset if you
//add 1 to your tiles input, note that the sprite type (normal or RLET) doesn't matter:
void* GetTile_Ptr(void *ptr, uint8_t tile)
{
    // pointer to data + offset of LUT data from start + stored offset at LUT entry "tile"
    return (void*)(ptr + *((uint16_t*)ptr + 1) + *((uint16_t*)ptr + tile + 1));
}

//The exact same thing as the above, but we don't add an extra offset to the tileset LUT; meaning this only works
//for lists of sprites in an appvar (no tilesets). Again, type doesn't matter:
void* GetSprite_Ptr(void *ptr, uint8_t tile)
{
    // pointer to data + stored offset at LUT entry "tile"
    return (void*)(ptr + *((uint16_t*)ptr + tile + 1));
}

//Draws the opening tiles and the button selector for choosing what to do, handles all menus and popups at the
//beginning, I have to have pointers to the sprites because they're local variables:
void TitleMenu(gfx_sprite_t *ceiling[], gfx_sprite_t *background[], gfx_sprite_t *floor[], gfx_sprite_t *menusprite)
{
    uint8_t selectorY = 5;

    //the selector rectangle is white, so the color has to be 2; why I chose 2 as white is unknown to me:
    gfx_SetColor(2);

    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(2);

    //temporary flipped menu sprite with hardcoded numbers to annoy people:
    gfx_sprite_t *flipped_menusprite = gfx_MallocSprite(8, 167);

    //menuing loop:
    do{
        //graphics and input handling for main menu:
        do{
            kb_Scan();

            for(uint8_t i = 0; i < 7; ++i)
            {
                gfx_Sprite(ceiling[i],    i * 46, 0);
                gfx_Sprite(background[i], i * 46, 40);
                gfx_Sprite(floor[i],      i * 46, 200);
            }

            if((kb_Data[7] & kb_Down) && !(kb_Data[7] & kb_Up))
            {
                if(selectorY == 110)
                {
                    selectorY = 5;
                } else {
                    selectorY += 35;
                }
            }
            else if((kb_Data[7] & kb_Up) && !(kb_Data[7] & kb_Down))
            {
                if(selectorY == 5)
                {
                    selectorY = 110;
                } else {
                    selectorY -= 35;
                }
            }

            gfx_Rectangle_NoClip(36, selectorY, 98, 31);

            gfx_BlitBuffer();

            //wait until none of the arrow keys are pressed:
            while(((kb_Data[7] & kb_Down) || (kb_Data[7] & kb_Up)) && !(kb_Data[6] & kb_Clear)) kb_Scan();
        }
        while(!((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter)) && !(kb_Data[6] & kb_Clear));

        //make sure the 2nd and enter keys aren't still pressed:
        while(((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter)) && !(kb_Data[6] & kb_Clear)) kb_Scan();

        //where to start my rambling monologue about coding and friends and motivation and crap:
        uint8_t txt_start = 0;

        //figure out what to do based on selection with my favorite evaluation technique, and that [clear] check
        //is a big brain play where the key being pressed adds one to selectorY and keeps it from matching anything:
        switch(selectorY + (kb_Data[6] & kb_Clear))
        {
            case 40: break; //shop

            case 75: break; //settings

            case 110: //about page
                do{
                    kb_Scan();

                    gfx_TransparentSprite_NoClip(menusprite, 33, 33);

                    gfx_CopyRectangle(gfx_buffer, gfx_buffer, 40, 33, 41, 33, 238, 162);

                    gfx_TransparentSprite_NoClip(gfx_FlipSpriteY(menusprite, flipped_menusprite), 279, 33);

                    //adjust which twelve lines of text show up based on up and down inputs:
                    if((kb_Data[7] & kb_Down) && !(kb_Data[7] & kb_Up) && (txt_start < ((sizeof(about_txt) / 3) - 12)))
                    {
                        txt_start += 2;
                    }
                    if((kb_Data[7] & kb_Up) && !(kb_Data[7] & kb_Down) && (txt_start > 0))
                    {
                        txt_start -= 2;
                    }

                    for(uint8_t i = 0; i < 12; ++i)
                    {
                        gfx_PrintStringXY(about_txt[i + txt_start], 38, 60 + i*12);
                    }

                    //cover the bit of bottom text that clips out of the window:
                    gfx_CopyRectangle(gfx_buffer, gfx_buffer, 38, 195, 39, 195, 240, 5);

                    gfx_BlitBuffer();

                    //make sure those keys are released:
                    while(((kb_Data[7] & kb_Down) || (kb_Data[7] & kb_Up)) && !(kb_Data[6] & kb_Clear)) kb_Scan();
                }
                while(!((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter)) && !(kb_Data[6] & kb_Clear));

                while( ((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter)) && !(kb_Data[6] & kb_Clear)) kb_Scan();
            break;
        }
    }
    while(!(kb_Data[6] & kb_Clear) && (selectorY != 5));

    //almost forgot this part, that would've been bad:
    free(flipped_menusprite);
}
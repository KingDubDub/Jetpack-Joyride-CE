/*Jetpack Joyride CE

A Jetpack Joyride port for the TI-84 Plus CE calculators.

Made by King Dub Dub

I'm pretty sure you have the readme if you have this source code, but if you want
to mod this or something then get ready for some over-commented trash!

In case it wasn't clear, modding this should have my permission and credit to me,
but other than that you are obliged to have as much fun as will kill you!

*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <keypadc.h>
#include <compression.h>
#include <graphx.h>
#include <fileioc.h>

//my little headers thingy that's full of data I neither touch much nor count towards total lines:
#include "headers.h"

//max number of various obstacles that are allowed to spawn:
#define MAX_ZAPPERS 3
#define MAX_MISSILES 1

//the starting X-coords for various obstacles and things:
#define COIN_ORIGIN 330
#define ZAPPER_ORIGIN 352
#define MISSILE_ORIGIN 1466

//appvar name and version since I decided to add versions, it's my code and I'll do what I want:
#define DATA_APPVAR "JTPKDAT"
const uint8_t APPVAR_VERSION = 3;

//HEY FUTURE ME, REMEMBER TO UPDATE THE VERSION WHEN WE ADD STUFF!

//our wonderful appvar for the save data:
ti_var_t savegame;

//we read APPVAR_VERSION to this var for testing later:
uint8_t saveIntegrity;

//time it takes to complete the game loop, used to control the FPS; if it overflows
//then we have real speed problems:
uint8_t FPS;

//speed of scrolling and time before incrementing it:
int8_t scrollSpeed;
uint16_t incrementDelay;

//color to flash when hit by bullet, it's a really cool effect:
uint8_t deathColor;

//measures timings for delays between spawning coins, obstacles, etc.:
int24_t spawnDelay;
int24_t missileDelay;

//used for a bad background scroll function that is actually the best for this scenario:
int24_t bg_scroll;

//arrays for storing background tileset pointer values to draw:
uint8_t bg_list[9];
uint8_t secondary_bg_list[9];

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

jetpack_t jetpackEntity;

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name
//my child if I had the desire to marry and have children.
avatar_t avatar;

typedef struct
{
    uint8_t health;        //the number of hits Barry can take (increased with vehicles and shields)
    uint32_t monies;      //money collected in the run
    uint32_t collegeFund; //total money collected from all runs, can be used to purchase stuff (when can I add this?)
    uint32_t distance;    //distance travelled in the run, measured in pixels
    uint32_t highscore;   //highest distance travelled in all runs
}
game_data_t;

//all important game data, again, all in one clean struct pointer:
game_data_t save_data;

//hey looky, a coin struct type for easy use, WHY DIDN'T I MAKE THIS EARLIER?
//stores x and y positions, along with the coin's animation frame:
typedef struct
{
    int24_t x[MAX_COINS];
    uint8_t y[MAX_COINS];
    uint8_t animation[MAX_COINS];

    //coin formation variable to keep track of coin list shapes:
    uint8_t formation;
}
coin_t;

//initialize some coins, about 30 is the max:
coin_t coins;

//again, WHY did I not make these before...
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

zapper_t zappers;

//I think the ZDS toolchain broke them and doubled the program size, I love LLVM
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

missile_t missiles;

//the point is, updates are good; ergo, Windows is exciting
//y position (x position is shared among all lasers), laser's time to function:
typedef struct
{
    uint8_t y[MaxLasers];
    uint24_t lifetime[MaxLasers];

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

laser_t lasers;

//flipped zapper sprites:
gfx_sprite_t *zapper_array_flipped[4];

//horizontal zapper sprites:
gfx_sprite_t *horizontal_zapper[4];
gfx_sprite_t *horizontal_zapper_flipped[4];

//flipped laser sprites and effects:
gfx_sprite_t *powering_tiles_flipped[4];
gfx_sprite_t *firing_tiles_flipped[3];

//buffers for resizing the smaller sprites, and another to rotate them once enlarged:
gfx_sprite_t *barryHit_resized;
gfx_sprite_t *barryHit_rotated;

//buffers for the jetpack:
gfx_sprite_t *jetpack_resized;
gfx_sprite_t *jetpack_rotated;

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

//clears all objects from gameplay by moving their X-coords out of bounds:
void clrObjects(void)
{
    for(uint8_t i = 0; i < coin_max[coins.formation]; ++i)
    {
        coins.x[i] = 2000;
    }

    for(uint8_t i = 0; i < MAX_ZAPPERS; ++i)
    {
        zappers.x[i] = ZAPPER_ORIGIN;
    }

    for(uint8_t i = 0; i < MAX_MISSILES; ++i)
    {
        missiles.x[i] = (MISSILE_ORIGIN + 10);
    }

    lasers.x = 0;
}

//a function for drawing buttons, will hopefully save on flash size and stack usage:
void draw_button(gfx_sprite_t *sprites[], char *text, uint8_t buttonSelect, uint8_t x)
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
    gfx_PrintStringXY(text, x, 47 + (buttonSelect * 60));
}

//this is purely for streamlining, it should only used twice:
void save_state(void)
{
    //create a new appvar, which erases the old one:
    savegame = ti_Open(DATA_APPVAR, "w");

    //game save data and stats:
    ti_Write(&save_data, sizeof(save_data), 1, savegame);

    //version of the appvar, for future use with updater packs and to hopefully provide some more debugging support:
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

    //uncomment this in the NEAR FINAL release, right now it should a temporary save until I iron out the bugs:
    //ti_SetArchiveStatus(true, savegame);
}

int main(void)
{
    //close any files that may have been left open from the last program:
    ti_CloseAll();

    //get the pointers to the palette and sprites in the flash:
    void *palette_ptr = ti_GetDataPtr(ti_Open("JPJRPAL", "r"));
    ti_CloseAll();
    void *start_background_ptr = ti_GetDataPtr(ti_Open("JPJRBG1", "r"));
    ti_CloseAll();
    void *background_ptr = ti_GetDataPtr(ti_Open("JPJRBG2", "r"));
    ti_CloseAll();
    void *background_extras_ptr = ti_GetDataPtr(ti_Open("JPJRBG3", "r"));
    ti_CloseAll();
    void *menu_ptr = ti_GetDataPtr(ti_Open("JPJRGFX1", "r"));
    ti_CloseAll();
    void *obstacles_ptr = ti_GetDataPtr(ti_Open("JPJRGFX2", "r"));
    ti_CloseAll();
    void *avatar_ptr = ti_GetDataPtr(ti_Open("BARRY", "r"));
    ti_CloseAll();

    //make sure that the appvar checks didn't return NULL:
    if( (!palette_ptr)           ||
        (!start_background_ptr)  ||
        (!background_ptr)        ||
        (!background_extras_ptr) ||
        (!menu_ptr)              ||
        (!obstacles_ptr)         ||
        (!avatar_ptr)
        ) return 1;

    //used to save which offset in the obstacles appvar's LUT entries we need to use:
    uint8_t offsets_offset = 0;

    //pointers to the background sprites:
    gfx_sprite_t *ceiling_tiles[11];
    gfx_sprite_t *background_tiles[15];
    gfx_sprite_t *floor_tiles[11];

    //set the pointers for the opening background where we bust through the wall:
    for(uint8_t i = 0; i < 8; ++i)
    {
        background_tiles[i] = (gfx_sprite_t*)(start_background_ptr + *((uint16_t*)start_background_ptr + 1) + *((uint16_t*)start_background_ptr + i + 2));
    }

    //there's this one opening background bit that has to go with the normal backgrounds since it doesn't fit in the appvar:
    background_tiles[8] = (gfx_sprite_t*)(background_ptr + *((uint16_t*)background_ptr + 1));

    //move the background sprites directly into the RAM:
    for(uint8_t i = 1; i < 7; ++i)
    {
        background_tiles[8 + i] = gfx_MallocSprite(46, 160);
        zx7_Decompress(background_tiles[8 + i], (gfx_sprite_t*)(background_ptr + *((uint16_t*)background_ptr + i + 1)));
    }

    //set the pointers for the ceiling sprites, some are reused with different backgrounds:
    for(uint8_t i = 0; i < 11; ++i)
    {
        ceiling_tiles[i] = (gfx_sprite_t*)(background_extras_ptr + *((uint16_t*)background_extras_ptr + 1) + *((uint16_t*)background_extras_ptr + i + 2));
    }

    //and get the floor bits too:
    for(uint8_t i = 11; i < 22; ++i)
    {
        floor_tiles[i - 11] = (gfx_sprite_t*)(background_extras_ptr + *((uint16_t*)background_extras_ptr + 1) + *((uint16_t*)background_extras_ptr + i + 2));
    }

    gfx_sprite_t *button_on_tiles[4];
    gfx_sprite_t *button_off_tiles[4];

    //now we get the sprites for the menus, here's the unpressed pause buttons:
    for(uint8_t i = 0; i < (sizeof(button_on_tiles) / 3); ++i)
    {
        button_on_tiles[i] = (gfx_sprite_t*)(menu_ptr + *((uint16_t*)menu_ptr + (offsets_offset++) + 1));
    }

    //and the pressed buttons:
    for(uint8_t i = 0; i < (sizeof(button_off_tiles) / 3); ++i)
    {
        button_off_tiles[i] = (gfx_sprite_t*)(menu_ptr + *((uint16_t*)menu_ptr + (offsets_offset++) + 1));
    }

    offsets_offset = 0;

    //pointers to pickup and obstacle sprites 'n stuff, in the order that they're arranged in the appvar:
    gfx_rletsprite_t *coin_tiles[5];
    gfx_sprite_t     *beam[2];
    gfx_sprite_t     *zapper_tiles[4];
    gfx_rletsprite_t *missileWarning_tiles[3];
    gfx_rletsprite_t *missileIncoming_tiles[2];
    gfx_rletsprite_t *missile_tiles[7];
    gfx_sprite_t     *powering_tiles[4];
    gfx_sprite_t     *firing_tiles[3];
    gfx_rletsprite_t *shutdown_tiles[6];
    gfx_sprite_t     *laser_tiles[4];

    //locate coin sprite pointers:
    for(uint8_t i = 0; i < (sizeof(coin_tiles) / 3); ++i)
    {
        coin_tiles[i] = (gfx_rletsprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //lookit the zapper beam being all weird and stuff:
    beam[0] = (gfx_sprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));

    //get the ding-dang zapper sprite pointers:
    for(uint8_t i = 0; i < (sizeof(zapper_tiles) / 3); ++i)
    {
        zapper_tiles[i] = (gfx_sprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //find the missile warning and incoming icon pointers:
    for(uint8_t i = 0; i < (sizeof(missileWarning_tiles) / 3); ++i)
    {
        missileWarning_tiles[i] = (gfx_rletsprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }
    for(uint8_t i = 0; i < (sizeof(missileIncoming_tiles) / 3); ++i)
    {
        missileIncoming_tiles[i] = (gfx_rletsprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //calculate pointers for missile sprites:
    for(uint8_t i = 0; i < (sizeof(missile_tiles) / 3); ++i)
    {
        missile_tiles[i] = (gfx_rletsprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //the laser node itself, but only the left side, we create flipped copies later:
    for(uint8_t i = 0; i < (sizeof(powering_tiles) / 3); ++i)
    {
        powering_tiles[i] = (gfx_sprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //pew pew pew
    for(uint8_t i = 0; i < (sizeof(firing_tiles) / 3); ++i)
    {
        firing_tiles[i] = (gfx_sprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //when the lasers power down:
    for(uint8_t i = 0; i < (sizeof(shutdown_tiles) / 3); ++i)
    {
        shutdown_tiles[i] = (gfx_rletsprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(laser_tiles) / 3); ++i)
    {
        laser_tiles[i] = (gfx_sprite_t*)(obstacles_ptr + *((uint16_t*)obstacles_ptr + (offsets_offset++) + 1));
    }

    //we start reading from a different appvar after this:
    offsets_offset = 0;

    gfx_sprite_t     *jetpack;
    gfx_sprite_t     *nozzle;
    gfx_rletsprite_t *barry_run_tiles[4];
    gfx_sprite_t     *barry_hit_tiles[3];
    gfx_sprite_t     *barry_ded_tiles[3];
    gfx_rletsprite_t *exhaust_tiles[6];

    //Oh my gosh, a post-increment was actually useful for once. Huh.
    jetpack = (gfx_sprite_t*)(avatar_ptr + *((uint16_t*)avatar_ptr + (offsets_offset++) + 1));

    nozzle = (gfx_sprite_t*)(avatar_ptr + *((uint16_t*)avatar_ptr + (offsets_offset++) + 1));

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(barry_run_tiles) / 3); ++i)
    {
        barry_run_tiles[i] = (gfx_rletsprite_t*)(avatar_ptr + *((uint16_t*)avatar_ptr + (offsets_offset++) + 1));
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(barry_hit_tiles) / 3); ++i)
    {
        barry_hit_tiles[i] = (gfx_sprite_t*)(avatar_ptr + *((uint16_t*)avatar_ptr + (offsets_offset++) + 1));
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(barry_ded_tiles) / 3); ++i)
    {
        barry_ded_tiles[i] = (gfx_sprite_t*)(avatar_ptr + *((uint16_t*)avatar_ptr + (offsets_offset++) + 1));
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(exhaust_tiles) / 3); ++i)
    {
        exhaust_tiles[i] = (gfx_rletsprite_t*)(avatar_ptr + *((uint16_t*)avatar_ptr + (offsets_offset++) + 1));
    }

    //flipped zapper sprites:
    for(uint8_t i = 0; i < 4; ++i)
    {
        zapper_array_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_FlipSpriteX(zapper_tiles[i], zapper_array_flipped[i]);
    }

    //horizontal zapper sprites:
    for(uint8_t i = 0; i < 4; ++i)
    {
        horizontal_zapper[i] = gfx_MallocSprite(32, 32);
        horizontal_zapper_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_RotateSpriteC(zapper_tiles[i], horizontal_zapper[i]);
        gfx_RotateSpriteC(zapper_array_flipped[i], horizontal_zapper_flipped[i]);
    }

    //horizontal zapper beam:
    beam[1] = gfx_MallocSprite(10, 10);
    gfx_RotateSpriteC(beam[0], beam[1]);

    //flipping laser powering up animations:
    for(uint8_t i = 0; i < 4; ++i)
    {
        powering_tiles_flipped[i] = gfx_MallocSprite(19, 15);
        gfx_FlipSpriteY(powering_tiles[i], powering_tiles_flipped[i]);
    }

    //flipping laser sprites:
    for(uint8_t i = 0; i < 3; ++i)
    {
        firing_tiles_flipped[i] = gfx_MallocSprite(30, 37);
        gfx_FlipSpriteY(firing_tiles[i], firing_tiles_flipped[i]);
    }

    //make space for the buffer, they need to be 36 pixels but the rotation functions tend to round up 1 pixel:
    barryHit_resized = gfx_MallocSprite(37, 37);
    barryHit_rotated = gfx_MallocSprite(37, 37);

    //clear the resizing buffer data, since malloc() just expands it; the size of the data is (37*37)-1:
    Fill_Array(barryHit_resized->data, 1368, 0);

    //jetpack sprite needs to be resized as well, square 24x24 with extra pixels:
    jetpack_resized = gfx_MallocSprite(25, 25);
    jetpack_rotated = gfx_MallocSprite(25, 25);

    //clear the jetpack sprite data too, (25 * 25) - 1 = 624:
    Fill_Array(jetpack_resized->data, 624, 0);

    //make a random damage sprite to start with:
    CopyPasta(barry_hit_tiles[randInt(0, 2)], barryHit_resized, 6, 0);

    //prepare the jetpack:
    CopyPasta(jetpack, jetpack_resized, 2, 0);

    //start up a timer for FPS monitoring, do not move:
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    //best scan mode according to the angry lettuce man:
    kb_SetMode(MODE_3_CONTINUOUS);

    //initialize GFX libraries:
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(palette_ptr, 512, 0);
    gfx_SetTransparentColor(0);

    //this single function is keeping the code running:
    ti_CloseAll();

    //read from appvar if it exists, fails if it returns 0
    savegame = ti_Open(DATA_APPVAR, "r");
    if(!savegame)
    {
        save_state();
    }

    //update all of our gameplay data from the save data:
    ti_Read(&save_data, sizeof(save_data), 1, savegame);

    //check the appvar version for some future reason:
    ti_Read(&saveIntegrity, 1, 1, savegame);

    //when I first started using C, I asked some friends if there were GOTO statements.
    //They proved they were good friends, and told me "No, that's stupid". I'm glad they lied.
    GAMESTART:
    //But it's still sometimes okay.

    //better random seed generation:
    srand(rtc_Time());

    //load all the appvar data as our "starting point" if distance isn't 0
    if(save_data.distance)
    {
        //game environment variables:
        ti_Read(&scrollSpeed, sizeof(scrollSpeed), 1, savegame);
        ti_Read(&incrementDelay, sizeof(incrementDelay), 1, savegame);
        ti_Read(&bg_scroll, sizeof(bg_scroll), 1, savegame);
        ti_Read(&bg_list, sizeof(bg_list), 1, savegame);
        ti_Read(&secondary_bg_list, sizeof(secondary_bg_list), 1, savegame);
        ti_Read(&spawnDelay, sizeof(spawnDelay), 1, savegame);

        //single pointer to avatar struct:
        ti_Read(&avatar, sizeof(avatar), 1, savegame);

        //jetpack variables:
        ti_Read(&jetpackEntity, sizeof(jetpackEntity), 1, savegame);

        //coin variables:
        ti_Read(&coins, sizeof(coins), 1, savegame);

        //zapper variables:
        ti_Read(&zappers, sizeof(zappers), 1, savegame);

        //missile variables:
        ti_Read(&missiles, sizeof(missiles), 1, savegame);

        //laser variables:
        ti_Read(&lasers, sizeof(lasers), 1, savegame);

        ti_CloseAll();
    }
    else
    {
        //reset variables for when a game starts:
        scrollSpeed = 6;
        incrementDelay = 0;
        spawnDelay = 512;
        
        save_data.distance = 0;
        save_data.health = 1;
        save_data.monies = 0;

        avatar.x = 24;
        avatar.y = 185;
        avatar.theta = 0;
        avatar.inputDuration = 0;
        avatar.playerAnimationToggle = 1;
        avatar.playerAnimation = 3;
        avatar.exhaustAnimation = 18;
        avatar.corpseBounce = 0;
        avatar.deathDelay = 0;

        //if there's a bug, it's always because the animation values are funky:
        missiles.iconAnimate = 0;
        missiles.animationToggle = -1;

        lasers.deactivated = MaxLasers;

        jetpackEntity.theta = 0;
        jetpackEntity.bounce = 0;
        jetpackEntity.h_accel = 2;

        clrObjects();

        //set the backgrounds up for the opening scene:
        for(int8_t i = 0; i < 9; ++i)
        {
            bg_list[i] = i;
            secondary_bg_list[i] = i;
        }
    }

    //Loop until clear is pressed:
    do
    {
        //update keys, fixes bugs with update errors that can lead to softlocks:
        kb_Scan();

        //controls bg_scroll, 46 is for background sprite width:
        if((bg_scroll + scrollSpeed) >= 46)
        {
            //seamlessly reset the position and increment it:
            bg_scroll -= (46 - scrollSpeed);

            //move the background tileset lists:
            for(uint8_t i = 0; i < 8; ++i)
            {
                bg_list[i] = bg_list[i + 1];
                secondary_bg_list[i] = secondary_bg_list[i + 1];
            }

            //if the last entry has been "dragged" through the next 2 list entries, make new sprite addresses,
            //note that I only check the background tiles since the timings are the same for all tiles:
            if((bg_list[7] == bg_list[8]) && (bg_list[6] == bg_list[8]))
            {
                bg_list[7] = 9 + randInt(0, 2) * 2;
                bg_list[8] = bg_list[7] + 1;

                secondary_bg_list[7] =  9;
                secondary_bg_list[8] = 10;
            }
            //the problem is that all the tiles are only half of a single hallway "chunk" of 92 pixels.
        }
        else
        {
            bg_scroll += scrollSpeed;
        }

        //if missile delay isn't zero, keep counting down:
        if(missileDelay > 0)
        {
            missileDelay -= scrollSpeed + 8;
        }

        //spawns stuff, SO much better than the debug methods I originally used, lasers act weird sometimes though:
        if((spawnDelay < 1) && (lasers.x < 1))
        {
            uint8_t randObject = randInt(0, 30);
            //uint8_t randObject = randInt(0, 0);

            if(!randObject && (missileDelay <= 0))
            {
                //picks a random formation from the data arrays in the laser_formations file:
                lasers.formation = randInt(0, (LASER_FORMATIONS - 1));

                //give each laser their Y-axis:
                for(uint8_t i = 0; i < laserMax[lasers.formation]; ++i)
                {
                    lasers.y[i] = lsrY[lasers.formation][i];
                    lasers.lifetime[i] = halfLife[lasers.formation][i];
                }

                lasers.x = 1;

                lasers.deactivated = 0;

                spawnDelay = scrollSpeed * 40;
            }
            else if((randObject < 3) && (missileDelay <= 0))
            {
                //sets coin coordinates from coordinate lists:
                uint8_t randVar = randInt(30, 150);

                coins.formation = randInt(0, (COIN_FORMATIONS - 1));

                for(uint8_t i = 0; i < MAX_COINS; ++i)
                {
                    coins.x[i] = (COIN_ORIGIN + ctx[coins.formation][i]);
                    coins.y[i] = (randVar + cty[coins.formation][i]);
                    coins.animation[i] = 0;
                }
                spawnDelay = 500;
            }
            else if(randObject < 10)
            {
                //spawns missiles (and missile swarms if I implement them):
                for(uint8_t i = 0; i < MAX_MISSILES; ++i)
                {
                    //if there's a missile offscreen, put it into play:
                    if((missiles.x[i] > MISSILE_ORIGIN) || (missiles.x[i] < -54))
                    {
                        missiles.x[i] = MISSILE_ORIGIN;
                        missiles.y[i] = 10 * randInt(2, 18);

                        i = MAX_MISSILES;
                    }
                }

                //if there's a missile I can't spawn lasers or coins, so here's a special thing just for it:
                missileDelay = MISSILE_ORIGIN;
            }
            else if(randObject < 30)
            {
                //randomly generates zapper coordinates and lengths:
                for(uint8_t i = 0; i < MAX_ZAPPERS; ++i)
                {
                    //if the zapper isn't in play:
                    if((zappers.x[i] >= ZAPPER_ORIGIN) || (zappers.x[i] < -100))
                    {
                        //put it in play:
                        zappers.x[i] = ZAPPER_ORIGIN - 1;

                        //zapper length is always set the same way, regardless of orientation:
                        zappers.length[i] = randInt(2, 4) * 10;

                        //if randInt returns anything that isn't zero, make a vertical zapper:
                        if(randInt(0, 2))
                        {
                            zappers.orientation[i] = 0;

                            //vertical zappers can appear from y-20 to y-170:
                            zappers.y[i] = 20 * randInt(1, 8 - (zappers.length[i] / 20));

                            //delay the next spawn:
                            spawnDelay = randInt(140, 200);
                        }
                        else //if zero, then it's a horizontal zapper:
                        {
                            zappers.orientation[i] = 1;

                            //horizontal zappers do be spawnin' everywhere tho:
                            zappers.y[i] = (20 * randInt(1, 10)) - 5;

                            //delay the next spawping with respect for the elongation of the zapper:
                            spawnDelay = zappers.length[i] + randInt(140, 200);
                        }

                        //if we made a zapper, stop making more:
                        i = MAX_ZAPPERS;
                    }
                }
            }
        }
        else
        {
            spawnDelay -= scrollSpeed;
        }

        //run controls and scrolling speed until Barry gets wasted, then bounces his corpse around:
        if(save_data.health > 0)
        {
            //very small bit of code to increase speed with a decreasing frequency over time, max of 12:
            if((incrementDelay >= ((scrollSpeed - 5) * 250)) && (scrollSpeed < 12))
            {
                ++scrollSpeed;
                incrementDelay = 0;
            } else {
                ++incrementDelay;
            }

            //when flying:
            if(kb_Data[1] & kb_2nd)
            {
                //add to avatar.inputDuration, which is added to avatar.y:
                if(avatar.inputDuration < 12)
                {
                    avatar.inputDuration += 1;
                }

                //numbers for jetpack output (bullets or fire or whatever):
                if(avatar.exhaustAnimation < 7)
                {
                    ++avatar.exhaustAnimation;
                } else if(avatar.exhaustAnimation > 7) {
                    avatar.exhaustAnimation = 0;
                }

            //when falling (2nd isn't pressed):
            } else {
                //subtract from avatar.inputDuration, added to avatar.y (it can be a negative number):
                if((avatar.y < (185)) && (avatar.inputDuration > -12))
                {
                    avatar.inputDuration -= 1;
                }

                //increases numbers for jetpack powering down animation:
                if(avatar.exhaustAnimation < 7)
                {
                    avatar.exhaustAnimation = 8;
                }
                else if(avatar.exhaustAnimation < 11)
                {
                    ++avatar.exhaustAnimation;
                }
                else
                {
                    avatar.exhaustAnimation = 18;
                }
            }

            //sees if Y-value rolled past 20 or 185, figures out if it was going up
            //or down, and auto-corrects accordingly (FUTURE ME: IT'S OPTIMIZED ENOUGH PLEASE DON'T WASTE ANY MORE TIME):
            if(((avatar.y - avatar.inputDuration) > 185) || ((avatar.y - avatar.inputDuration) < 20))
            {
                if(avatar.inputDuration > 0)
                {
                    avatar.y = 20;
                } else {
                    avatar.y = 185;
                }
                avatar.inputDuration = 0;
            }
            else //if Y-value didn't roll past:
            {
                //SAX got pretty mad about this part, turns out a linear equation can model a curve and is actually better
                //than a cubic function:
                avatar.y -= avatar.inputDuration;
            }

            //bit that runs avatar animations:
            if(avatar.y < 185)
            {
                //flying animation:
                avatar.playerAnimation = 6;
            }
            else
            {
                //quickly reset sprite animation to the second running sprite:
                if(avatar.playerAnimation == 6)
                {
                    avatar.playerAnimation = 2;
                }

                //toggle the animation frame order from first to last and vice versa:
                if(avatar.playerAnimation < 1)
                {
                    avatar.playerAnimationToggle = 1;
                }
                else if(avatar.playerAnimation > 4)
                {
                    avatar.playerAnimationToggle = -1;
                }
                avatar.playerAnimation += avatar.playerAnimationToggle;
            }
        }
        else //if "THE HEAVY IS DEAD!", calculate an inelastic collision on his corpse:
        {
            //when the crap hits the fan, or the Barry hits the floor I dunno I didn't read the manga:
            if((avatar.y - avatar.inputDuration) >= 185)
            {
                avatar.y = 185;

                //if scrollSpeed isn't zero:
                if(scrollSpeed)
                {
                    //slow down, rotate, and count the bounces:
                    --scrollSpeed;
                    ++avatar.corpseBounce;

                    avatar.inputDuration *= -1;
                    avatar.inputDuration -= 3;

                    if(avatar.inputDuration < 3)
                    {
                        //if the acceleration drops beneath a certain point, consider Barry bounced:
                        avatar.corpseBounce = 3;
                    }
                    else
                    {
                        //empty the resized sprite:
                        Fill_Array(barryHit_resized->data, 1369, 0);

                        //pick a new random sprite to spin around:
                        CopyPasta(barry_hit_tiles[randInt(0, 2)], barryHit_resized, 6, 0);
                    }
                }
                else //increment a timer for waiting a bit before going to death menu:
                {
                    ++avatar.deathDelay;
                }
                
            }
            else if((avatar.y - avatar.inputDuration) < 20) //if Barry hits the ceiling, flip his acceleration:
            {
                avatar.y = 20;
                avatar.inputDuration *= -1;
            }
            else
            {
                avatar.y -= avatar.inputDuration;
                avatar.theta += scrollSpeed;

                //downwards acceleration increase and cap, slightly faster than normal:
                if(avatar.inputDuration > -14)
                {
                    --avatar.inputDuration;
                }

                //correct previous acceleration if Barry is well and truly ded:
                if(avatar.corpseBounce >= 3)
                {
                    avatar.inputDuration = 0;
                    //this doesn't reset sometimes, still not sure why:
                    avatar.y = 185;
                }
            }

            //I'm keeping the jetpack rotation and control code seperate until I'm ready to hybridize the old code with it.

            //if it hits the floor, which is slighty lower for the jetpack in order to give a more 3D look:
            if((jetpackEntity.y - jetpackEntity.v_accel) >= 210)
            {
                jetpackEntity.y = 210;

                if(jetpackEntity.bounce < 3)
                {
                    ++jetpackEntity.bounce;

                    //stop the bouncing if the numbers are getting to small:
                    if(jetpackEntity.v_accel < -6)
                    {
                        jetpackEntity.v_accel *= -1;
                        jetpackEntity.v_accel -= 3;
                    }
                    else
                    {
                        jetpackEntity.v_accel = 0;
                        jetpackEntity.bounce = 3;
                    }

                    //slow down the forward motion after hitting the ground when the screen stops moving:
                    if(jetpackEntity.h_accel && !scrollSpeed)
                    {
                        --jetpackEntity.h_accel;
                    }
                }
                else
                {
                    //equivalent to rotating 270 degrees clockwise:
                    jetpackEntity.theta = 192;
                }
            }
            else if((jetpackEntity.y - jetpackEntity.v_accel) < 20) //if it hits the ceiling:
            {
                jetpackEntity.y = 20;
                jetpackEntity.v_accel *= -1;
            }
            else
            {
                jetpackEntity.x += jetpackEntity.h_accel;
                jetpackEntity.y -= jetpackEntity.v_accel;

                //the jetpack rotates faster than Barry because I feel like it:
                jetpackEntity.theta += (scrollSpeed * 2) + jetpackEntity.h_accel;

                if(jetpackEntity.v_accel > -12)
                {
                    --jetpackEntity.v_accel;
                }
            }

            //since rotating sprites is so slow, I'm doing it between drawing and buffer updates to increase speed:
            gfx_RotateSprite(barryHit_resized, barryHit_rotated, avatar.theta);
            gfx_RotateSprite(jetpack_resized, jetpack_rotated, jetpackEntity.theta);

            //An inelastic collision is when something bounces off something and loses some speed, which means the objects
            //stick together. Barry's corpse sticks to the floor a little bit (there's technically air friction and gravity
            //too but that's not gruesome enough) and I'm essentially emulating all of this with some physics modeling.
            //THANKS AP PHYSICS, YOU WERE ACTUALLY USEFUL!
        }

        if(zappers.animate < 11)
        {
            ++zappers.animate;
        } else {                    //see that else statment? That's code from 2020 and I don't plan on changing it.
            zappers.animate = 0;
        }                           //BUT OH DOES IT HURT TO LOOK AT.

        //control missile warning and incoming animation orders:
        if(missiles.iconAnimate < 1)
        {
            missiles.animationToggle = 1;
        }
        else if(missiles.iconAnimate > 4)
        {
            missiles.animationToggle = -1;
        }
        missiles.iconAnimate += missiles.animationToggle;

        //animate missile sprites:
        if(missiles.animation < 12)
        {
            ++missiles.animation;
        }
        else //but on the flip side, this else might be worse looking...
        {
            missiles.animation = 0;
        }

        //move lasers into play when needed:
        if(lasers.deactivated < laserMax[lasers.formation])
        {
            spawnDelay = scrollSpeed * 40;

            if(lasers.x < 20)
            {
                ++lasers.x;
            }
        }
        else if((lasers.deactivated >= laserMax[lasers.formation]) && (lasers.x > 0))
        {
            --lasers.x;
        }

        //mess with number for laser animations
        if(lasers.animation < 8)
        {
            ++lasers.animation;
        } else {
            lasers.animation = 0;
        }

        //draw the first and last background tiles with clipping since they go offscreen:
        gfx_Sprite(ceiling_tiles[secondary_bg_list[0]],   0 - bg_scroll, 0);
        gfx_Sprite(ceiling_tiles[secondary_bg_list[6]], 276 - bg_scroll, 0);
        gfx_Sprite(ceiling_tiles[secondary_bg_list[7]], 322 - bg_scroll, 0);

        gfx_Sprite(background_tiles[bg_list[0]],   0 - bg_scroll, 40);
        gfx_Sprite(background_tiles[bg_list[6]], 276 - bg_scroll, 40);
        gfx_Sprite(background_tiles[bg_list[7]], 322 - bg_scroll, 40);

        gfx_Sprite(floor_tiles[secondary_bg_list[0]],   0 - bg_scroll, 200);
        gfx_Sprite(floor_tiles[secondary_bg_list[6]], 276 - bg_scroll, 200);
        gfx_Sprite(floor_tiles[secondary_bg_list[7]], 322 - bg_scroll, 200);

        //this is the best way I've found to draw the backgrounds, smaller and faster than a smart system:
        for(uint8_t i = 1; i < 6; ++i)
        {
            //but all these are drawn all speedy-like without clipping:
            gfx_Sprite_NoClip(ceiling_tiles[secondary_bg_list[i]], (i * 46) - bg_scroll, 0);
            gfx_Sprite_NoClip(background_tiles[bg_list[i]], (i * 46) - bg_scroll, 40);
            gfx_Sprite_NoClip(floor_tiles[secondary_bg_list[i]], (i * 46) - bg_scroll, 200);
        }

        //draws avatar depending on health 'n stuff:
        if(save_data.health > 0)
        {
            //draws the avatar, everything is layered over it:
            gfx_RLETSprite_NoClip(barry_run_tiles[avatar.playerAnimation / 2], avatar.x, avatar.y);

            //bit that draws exhaust when in flight:
            if(avatar.exhaustAnimation < 18)
            {
                gfx_RLETSprite_NoClip(exhaust_tiles[avatar.exhaustAnimation / 2], avatar.x + randInt(1, 3), avatar.y + 31);
                gfx_TransparentSprite_NoClip(nozzle, avatar.x + 4, avatar.y + 31);
            }
        }
        else
        {
            //if Barry is ded and his body hath struck the ground thrice, then the end times are upon us:
            if(avatar.corpseBounce >= 3)
            {
                //I hardcoded barry's corpse's Y-pos for effeciency:
                gfx_TransparentSprite_NoClip(barry_ded_tiles[1], avatar.x, 210);
            }
            else
            {
                //we now draw that spinny sprite that's hopefully not going to suck up RAM like crazy:
                gfx_TransparentSprite_NoClip(barryHit_rotated, avatar.x - 5, avatar.y);
            }

            //draw the jetpack seperate from everything else:
            gfx_TransparentSprite_NoClip(jetpack_rotated, jetpackEntity.x, jetpackEntity.y);
        }

        //bit that runs coin collision and movement:
        for(uint8_t i = 0; i < coin_max[coins.formation]; ++i)
        {
            //do things if the coin is less than the origin plus some buffer I made up on the spot:
            if(coins.x[i] <= COIN_ORIGIN + 500)
            {
                //collision detection and appropriate sprite drawing:
                if(gfx_CheckRectangleHotspot(avatar.x + 2, avatar.y, 18, 37, coins.x[i] - 11, coins.y[i] + 1, 10, 10))
                {
                    //I saved the coin poof as the last tile, it's only drawn when Barry gets a coin (duh):
                    gfx_RLETSprite_NoClip(coin_tiles[4], coins.x[i] - 13, coins.y[i] - 1);

                    coins.x[i] = 1020;
                    ++save_data.monies;
                }
                else
                {
                    gfx_RLETSprite(coin_tiles[(coins.animation[i] / 10)], (coins.x[i] - 12), coins.y[i]);

                    if((coins.animation[i] < 38) && (coins.x[i] < COIN_ORIGIN))
                    {
                        coins.animation[i] += 2;
                    }
                    else
                    {
                        coins.animation[i] = 0;
                    }
                }
                coins.x[i] -= scrollSpeed;
            }
        }

        //bit that calculates zappers 'n stuff:
        for(uint8_t i = 0; i < MAX_ZAPPERS; ++i)
        {
            if(zappers.x[i] != ZAPPER_ORIGIN)
            {
                //if orientation is zero, use data to draw vertical sprites:
                if(!zappers.orientation[i])
                {
                    for(uint8_t j = 0; j < zappers.length[i]; j += 10)
                    {
                        gfx_TransparentSprite(beam[0], zappers.x[i] - 14, zappers.y[i] + 16 + j);
                    }

                    //draw zapper pairs and beams with distance of zapperLength between them:
                    gfx_TransparentSprite(zapper_array_flipped[zappers.animate/3], zappers.x[i] - 25, zappers.y[i] - 7);
                    gfx_TransparentSprite(zapper_tiles[zappers.animate/3], zappers.x[i] - 25, zappers.y[i] + 7 + zappers.length[i]);

                    //collision for vertical zappers:
                    if((save_data.health > 0) && (gfx_CheckRectangleHotspot(zappers.x[i] - 14, zappers.y[i] + 2, 10, zappers.length[i] + 30, avatar.x + 2, avatar.y, 18, 37)))
                    {
                        --save_data.health;

                        //screen flashes yellow:
                        deathColor = 4;
                    }
                }
                else //if the orientation is not zero:
                {
                    //draw the horizontal beam, which WAS painful as hekk to do mathwise before I changed everything. Again.
                    for(uint8_t j = 0; j < zappers.length[i]; j += 10)
                    {
                        //horizontal zappers are why I had to change all my x-values to int24 instead of uint24:
                        gfx_TransparentSprite(beam[1], zappers.x[i] + 32 + j, zappers.y[i] + 11);
                    }

                    //left 'n right emitter thingies:
                    gfx_TransparentSprite(horizontal_zapper[zappers.animate / 3], zappers.x[i], zappers.y[i]);
                    gfx_TransparentSprite(horizontal_zapper_flipped[zappers.animate / 3], zappers.x[i] + 32 + zappers.length[i], zappers.y[i]);
                
                    //collision for horizontal zappers, a 68-88x10 rectangle:
                    if((save_data.health > 0) && (gfx_CheckRectangleHotspot(zappers.x[i] + 8, zappers.y[i], zappers.length[i] + 48, 10, avatar.x, avatar.y, 20, 37)))
                    {
                        --save_data.health;

                         //screen flashes yellow:
                        deathColor = 4;
                    }
                }
                //all zappers move the same:
                zappers.x[i] -= scrollSpeed;
            }
        }

        //bit that draws and calculates the missiles 'o death:
        for(uint8_t i = 0; i < MAX_MISSILES; ++i)
        {
            if(missiles.x[i] <= MISSILE_ORIGIN)
            {
                if(missiles.x[i] < 366)
                {
                    gfx_RLETSprite(missile_tiles[missiles.animation / 2], missiles.x[i] - 46, missiles.y[i] - 18);

                    if((save_data.health > 0) && (gfx_CheckRectangleHotspot(missiles.x[i] - 45, avatar.y, 19, 37, avatar.x + 2, missiles.y[i] - 5, 18, 12)))
                    {
                        --save_data.health;

                        //screen flashes red
                        deathColor = 5;
                    }

                    //I'm using the X-position as timer, meaning there's less warning as time goes on:
                }
                else if(missiles.x[i] < 641)
                {
                    //AW CRAP HERE COME DAT BOI!
                    gfx_RLETSprite_NoClip(missileIncoming_tiles[missiles.iconAnimate / 3], 281 + randInt(-1, 1), missiles.y[i] - 16 + randInt(-1, 1));
                }
                else if(missiles.x[i] < MISSILE_ORIGIN)
                {
                    //plenty of time to dodge (at the beginning at least)
                    gfx_RLETSprite_NoClip(missileWarning_tiles[missiles.iconAnimate / 2], 293, missiles.y[i] - 11);

                    //tracking on avatar
                    if(missiles.y[i] < (avatar.y + 20))
                    {
                        missiles.y[i] += 2;
                    }
                    else if(missiles.y[i] > (avatar.y + 21))
                    {
                        missiles.y[i] -= 2;
                    }
                }

                //missiles are supposed to travel by 6 pixels each frame, but they travel too slowly. It was
                //surprisingly tedious to figure that out from frame-by-frame reviewing of game footage.
                missiles.x[i] -= scrollSpeed + 8;
            }
        }

        //bit for lasers, lots of moving parts:
        for(uint8_t i = 0; i < laserMax[lasers.formation]; ++i)
        {
            if(lasers.x > 0)
            {
                uint8_t animation_index = lasers.animation / 3;

                //reset the laser animations:
                if((lasers.x < 20) && (lasers.deactivated < laserMax[lasers.formation]))
                {
                    lasers.animation = 0;

                    //draw an inactive laser:
                    gfx_TransparentSprite(powering_tiles[0], lasers.x - 19, lasers.y[i]);
                    gfx_TransparentSprite(powering_tiles[0], 320 - lasers.x, lasers.y[i]);
                }
                else
                {
                    //if they are finished, finish the animation sequence and hide them again:
                    if(lasers.deactivated >= laserMax[lasers.formation])
                    {
                        lasers.animation = 0;

                        //if they aren't dead and have moved, run through their animations and attacks:
                    }
                    else
                    {
                        //check to see if the half-life ended:
                        if(lasers.lifetime[i] < 1)
                        {
                            ++lasers.deactivated;
                        }
                        else if(lasers.lifetime[i] < 9)
                        {
                            //lasers running out of juice:
                            gfx_RLETSprite_NoClip(shutdown_tiles[animation_index], 5, lasers.y[i] - 16);
                            gfx_RLETSprite_NoClip(shutdown_tiles[3 + animation_index], 285, lasers.y[i] - 16);

                            //drawing the laser beam line by line since that's faster and smaller than using a sprite:
                            for(uint8_t j = 0; j < 15; ++j)
                            {
                                if(laser_tiles[animation_index]->data[j])
                                {
                                    gfx_SetColor(laser_tiles[animation_index]->data[j]);
                                    gfx_HorizLine_NoClip(35, lasers.y[i] + j, 250);
                                }
                            }

                            gfx_Sprite_NoClip(laser_tiles[animation_index], 35, lasers.y[i]);
                        }
                        else if(lasers.lifetime[i] < 59)
                        {
                            //firing lasers:
                            gfx_TransparentSprite_NoClip(firing_tiles[animation_index], 5, lasers.y[i] - 11);
                            gfx_TransparentSprite_NoClip(firing_tiles_flipped[animation_index], 285, lasers.y[i] - 11);

                            for(uint8_t j = 0; j < 15; ++j)
                            {
                                if(laser_tiles[3]->data[j])
                                {
                                    gfx_SetColor(laser_tiles[3]->data[j]);
                                    gfx_HorizLine_NoClip(35, lasers.y[i] + j, 250);
                                }
                            }

                            //hitbox for damage. 3 pixel leeway above and below:
                            if((save_data.health > 0) && Yspot(avatar.y, 34, lasers.y[i], 12))
                            {
                                --save_data.health;

                                //again, the screen flashes red
                                deathColor = 5;
                            }

                            //makes sure the animations play correctly, I don't have time to fix math today:
                            if(lasers.lifetime[i] == 9)
                                lasers.animation = 0;
                        }
                        else if(lasers.lifetime[i] < 109)
                        {

                            //the simplest drawing code in this hot mess, for powering up:
                            gfx_SetColor(5);

                            gfx_Line_NoClip(20, lasers.y[i] + 7, 300, lasers.y[i] + 7);

                            gfx_Circle_NoClip(11, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle_NoClip(11, lasers.y[i]+7, 6 + ((lasers.lifetime[i]-50)/4));

                            gfx_Circle_NoClip(308, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle_NoClip(308, lasers.y[i]+7, 8 + ((lasers.lifetime[i]-50)/4));
                        }

                        //update the laser's timer:
                        --lasers.lifetime[i];
                    }
                }

                if(lasers.lifetime[i] < 109)
                {
                    gfx_TransparentSprite(powering_tiles[(animation_index) + 1], lasers.x - 19, lasers.y[i]);
                    gfx_TransparentSprite(powering_tiles_flipped[(animation_index) + 1], 320 - lasers.x, lasers.y[i]);
                }
                else
                {
                    gfx_TransparentSprite(powering_tiles[0], lasers.x - 19, lasers.y[i]);
                    gfx_TransparentSprite(powering_tiles_flipped[0], 320 - lasers.x, lasers.y[i]);
                }
            }
        }

        //sets a new highscore:
        if(save_data.highscore < ((save_data.distance += scrollSpeed) / 15))
        {
            save_data.highscore = (save_data.distance / 15);
        }
   
        //gold coin color for money counter:
        gfx_SetTextFGColor(4);

        gfx_SetTextScale(1, 1);
        gfx_SetTextXY(10, 41);
        gfx_PrintInt(save_data.monies, 3);

        //distance and FPS counter are gray:
        gfx_SetTextFGColor(3);

        gfx_PrintStringXY("BEST:", 10, 29);
        gfx_SetTextXY(50, 29);
        gfx_PrintInt(save_data.highscore, 1);


        gfx_SetTextXY(280, 10);
        gfx_PrintInt(FPS, 1);

        /*for(uint8_t i = 0; i < 15; ++i)
        {
            gfx_SetTextXY(260, 30 + (10 * i));
            gfx_PrintUInt((int)compressed_backgrounds[i], 1);
        }*/

        gfx_SetTextScale(2, 2);
        gfx_SetTextXY(10, 10);
        //these numbers are magic and just give good results
        gfx_PrintInt(save_data.distance / 15, 4);
        

        //speedy blitting, but works best when used a lot of cycles BEFORE any new drawing functions:
        gfx_SwapDraw();

        //afterwards, we check if Barry was hit, and if so then the screen will quickly flash a certain color:
        if(deathColor)
        {
            //we also set some variables that need to be updated once at time of death:
            jetpackEntity.y = avatar.y + 7;

            jetpackEntity.x = avatar.x;
            jetpackEntity.v_accel = avatar.inputDuration + 2;

            gfx_FillScreen(deathColor);
            
            //blit the color:
            gfx_SwapDraw();
            delay(80);
            //then swap the screen and buffer again to hide it:
            gfx_SwapDraw();
            //Basically, the color is drawn to the buffer and swapped with the screen, and then the process is reversed.
            //The solid-color screen becomes the buffer again and is overwritten after the next drawing loop.

            deathColor = 0;
        }

        //FPS counter data collection, time "stops" (being relevant) after this point:
        FPS = (32768 / timer_GetSafe(1, TIMER_UP));
        //the GetSafe is to make sure we don't read bad values as they change while they're read (AFIK)

        //time is frozen for a delay, and I wanna make a Jo-Jo reference but I don't speak Japanese
        if(FPS > 25)
        {
            delay(40 - (1000 / FPS));
        }

        //pause menu controls and drawing, can be accessed as long as in main loop:
        if(kb_Data[1] & kb_Del)
        {
            //button debouncing for menu:
            bool debounced;
            //value for keeping track of menuing:
            int8_t menuSelect;

            //strings used in pause menu:
            char pauseOptions[3][7] = {"Quit", "Retry", "Resume"};
            //their X-positions, since they don't follow a mathmatical trend:
            int8_t pauseOptionX[3] = {120, 103, 90};

            //drawing the base color for all the menu stuff:
            gfx_FillScreen(6);

            //using that draw_button() function to write out the 3 menu options:
            draw_button(button_on_tiles, pauseOptions[0], 0, pauseOptionX[0]);
            draw_button(button_on_tiles, pauseOptions[1], 1, pauseOptionX[1]);
            draw_button(button_on_tiles, pauseOptions[2], 2, pauseOptionX[2]);

            gfx_SetColor(2);

            //draw initial selector position:
            gfx_Rectangle(69, 32, 182, 52);

            debounced = false;
            menuSelect = 0;

            //menu control loop:
            while (!(kb_Data[6] & kb_Clear))
            {
                kb_Scan();

                if((kb_Data[7] & kb_Down) || (kb_Data[7] & kb_Up))
                {
                    debounced = false;

                    //erase the old selection rectangle as soon as any key updates are made that change the selection:
                    gfx_SetColor(6);
                    gfx_Rectangle(69, 32 + (menuSelect * 60), 182, 52);

                    //re-draw the unpressed form of whatever button is being moved on from:
                    draw_button(button_on_tiles, pauseOptions[menuSelect], menuSelect, pauseOptionX[menuSelect]);

                    //if down is pressed, add to menu select and correct overflow when necessary:
                    if(kb_Data[7] & kb_Down)
                    {
                        if(menuSelect > 1)
                        {
                            menuSelect = 0;
                        } else {
                            ++menuSelect;
                        }
                    }

                    //and if up is pressed, subtract from menu select and correct overflow:
                    if(kb_Data[7] & kb_Up)
                    {
                        if(menuSelect < 1)
                        {
                            menuSelect = 2;
                        } else {
                            --menuSelect;
                        }
                    }

                    //draw the new selection rectangle:
                    gfx_SetColor(2);
                    gfx_Rectangle(69, 32 + (menuSelect * 60), 182, 52);
                }
                else if((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter))
                {
                    debounced = true;

                    gfx_SetColor(6);

                    //edges of larger buttons need to be hidden for smaller buttons:
                    gfx_FillRectangle(69, 32 + (menuSelect * 60), 182, 52);

                    //draw the first and last 14 pixels of the selected button:
                    gfx_Sprite_NoClip(button_off_tiles[menuSelect], 80, 35 + (menuSelect * 60));
                    gfx_Sprite_NoClip(button_off_tiles[3], 226, 35 + (menuSelect * 60));

                    //abuse the gfx libs:
                    gfx_CopyRectangle(gfx_buffer, gfx_buffer, 93, 35 + (menuSelect * 60), 94, 35 + (menuSelect * 60), 132, 50);

                    //smol selector:
                    gfx_SetColor(2);
                    gfx_Rectangle(79, 34 + (menuSelect * 60), 162, 48);

                    //button text:
                    gfx_PrintStringXY(pauseOptions[menuSelect], pauseOptionX[menuSelect], 47 + (menuSelect * 60));
                }
                else
                {
                    if(debounced)
                    {
                        break;
                    }
                }

                gfx_Blit(1);

                //simple way to wait until none of the arrow keys are pressed:
                while(((kb_Data[7] & kb_Down) || (kb_Data[7] & kb_Up)) && !(kb_Data[6] & kb_Clear)) kb_Scan();
            }

            //imagine completing menuing checks when [clear] is pressed and leaving it in the code for 4 updates.
            if(!(kb_Data[6] & kb_Clear))
            {
                //that dumb break means I can't use a switch case (yet) so it just exits the program loop when "Quit" is selected.
                if(!menuSelect)
                {
                    //set health to 1 to keep from triggering death menu on loop exit:
                    save_data.health = 1;

                    //and this to flag that the game is over:
                    save_data.distance = 0;
                    break;
                    //goto main menu later
                }
                else if(menuSelect == 1)
                {
                    save_data.distance = 0;
                    goto GAMESTART;
                }
                //if menuSelect is 2 or anything else it just resumes; everything resets when the pause menu is accessed again.
            }
        }

        //and with a timer reset "ZA WARUDO" is over (iS tHaT A jOJo rEfERenCe and also yes I looked it up):
        timer_1_Counter = 0;

    } while (!(kb_Data[6] & kb_Clear) && (avatar.deathDelay != 50));

    if(!(kb_Data[6] & kb_Clear) && (save_data.health < 1))
    {
        //reset distance to flag that a game is no longer in progress:
        save_data.distance = 0;

        gfx_FillScreen(1);
        gfx_SetTextScale(3, 3);
        gfx_PrintStringXY("Imagine having", 10, 60);
        gfx_PrintStringXY("a stable game", 15, 100);
        gfx_PrintStringXY("after 9 months.", 8, 140);
        gfx_SetTextScale(1, 1);
        gfx_PrintStringXY("*laughs in idiot*", (320 - gfx_GetStringWidth("*laughs in dumb*"))/2, 200);
        gfx_SwapDraw();

        while (kb_AnyKey()) kb_Scan();
        while (!kb_AnyKey()) kb_Scan();

        if(kb_Data[6] != kb_Clear)
        {
            //I still can't believe I used a goto label...
            goto GAMESTART;
        }
    }
    
    //we only need distance and a few other vars in most cases, but I made this function AND I'M GONNA USE THE WHOLE FUNCTION.
    save_state();

    //stop libraries, not doing so causes "interesting" results by messing up the color mode and scaling:
    gfx_End();
    ti_CloseAll();

    return 1;
}

//CONGRATULATIONS, YOU FOUND THE END OF THE FILE! I HOPE PEEKING BEHIND THE CURTAIN WAS WORTH YOUR SANITY!
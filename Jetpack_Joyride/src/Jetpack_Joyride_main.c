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

//my little headers full of data that I don't touch much or count towards total lines:
#include "headers.h"

//all the sprite include files:
#include "sprites/gfx.h"

//max number of various obstacles that are allowed to spawn:
#define MaxZappers 3
#define MaxMissiles 1

//the starting X-coords for various obstacles and things:
#define COIN_ORIGIN 330
#define ZAPPER_ORIGIN 352
#define MISSILE_ORIGIN 1466

//appvar name and version since I decided to add versions, it's my code and I'll do what I want:
#define DATA_APPVAR "JTPKDAT"
const uint8_t APPVAR_VERSION = 2;

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
int24_t spawnDelay = 200;
uint24_t missileDelay;

//used for a bad background scroll function that is actually the best for this scenario:
int24_t backgroundScroll;

//for randomization values that can be reused globally (normally just one-and-done use cases):
uint8_t randVar;
uint8_t randVar1;

uint8_t randObject;

//button debouncing for menu:
bool debounced;
//value for keeping track of menuing:
int8_t menuSelect;

//strings used in pause menu:
char pauseOptions[3][7] = {"Quit", "Retry", "Resume"};
//their X-positions, since they don't follow a mathmatical trend:
int8_t pauseOptionX[3] = {120, 103, 90};

//all the data used to control Barry's position, acceleratoin, etc. are consolidated under one pointer.
//x and y positions, time 2nd is pressed/released to fly/fall, animation frame, animation cycling control, and jetpack animation frame:
typedef struct
{
    int24_t x;
    uint8_t y;
    uint8_t theta; //in calculator land, there're only 256 degrees, not 360 oogabooga degrees. This is gonna hurt my brain.
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
    int8_t health;        //the number of hits Barry can take (increased with vehicles and shields)
    uint32_t monies;      //money collected in the run
    uint32_t collegeFund; //total money collected from all runs, can be used to purchase stuff (when can I add this?)
    uint32_t distance;    //distance travelled in the run
    uint32_t highscore;   //highest distance travelled in all runs
}
game_data_t;

//all important game data, again, all in one clean struct pointer:
game_data_t save_data;

//hey looky, a coin struct type for easy use, WHY DIDN'T I MAKE THIS EARLIER?
//stores x and y positions, along with the coin's animation frame:
typedef struct
{
    int24_t x[MaxCoins];
    uint8_t y[MaxCoins];
    uint8_t animation[MaxCoins];

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
    int24_t x[MaxZappers];
    uint8_t y[MaxZappers];
    int8_t length[MaxZappers];

    //zapper type, vertical, horizontal, or positve/negative diagonal:
    int8_t orientation[MaxZappers];

    //zapper sprite animation frame:
    int8_t animate;
}
zapper_t;

zapper_t zappers;

//I think the ZDS toolchain broke them and doubled the program size, I love LLVM
//missile x and y positions:
typedef struct
{
    int24_t x[MaxMissiles];
    uint8_t y[MaxMissiles];

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
gfx_sprite_t *horizontal_beam;

//flipped laser sprites and effects:
gfx_sprite_t *powering_tiles_flipped[4];
gfx_sprite_t *firing_tiles_flipped[3];
gfx_sprite_t *shutdown_tiles_flipped[3];

//buffers for resizing the smaller sprites, and another to rotate them once enlarged:
gfx_sprite_t *barryHit_resized;
gfx_sprite_t *barryHit_rotated;

//buffers for the jetpack:
gfx_sprite_t *jetpack_resized;
gfx_sprite_t *jetpack_rotated;

ti_var_t savegame;

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
void clrObjects()
{
    for(uint8_t i = 0; i < coin_max[coins.formation]; ++i)
    {
        coins.x[i] = 2000;
    }

    for(uint8_t i = 0; i < MaxZappers; ++i)
    {
        zappers.x[i] = ZAPPER_ORIGIN;
    }

    for(uint8_t i = 0; i < MaxMissiles; ++i)
    {
        missiles.x[i] = (MISSILE_ORIGIN + 10);
    }

    lasers.x = 0;
}

//a function for drawing buttons, will hopefully save on flash size and stack usage:
void draw_button(uint8_t ButtonSelect)
{
    //first 20 pixels of the button:
    gfx_Sprite_NoClip(pauseButtonOn_tiles[ButtonSelect], 70, 33 + (ButtonSelect * 60));

    //the middle bits are drawn 7 times:
    for(uint8_t i = 0; i < 140; i += 20)
    {
        gfx_Sprite_NoClip(pauseButtonOn_tile_3, 90 + i, 33 + (ButtonSelect * 60));
    }

    //and the last 20 pixels:
    gfx_Sprite_NoClip(pauseButtonOn_tile_4, 230, 33 + (ButtonSelect * 60));

    //words 'n stuff:
    gfx_SetTextFGColor(2);
    gfx_SetTextScale(3, 3);

    //pretty much all the letters are odd numbers of pixels wide or tall, so that sucks:
    gfx_PrintStringXY(pauseOptions[ButtonSelect], pauseOptionX[ButtonSelect], 47 + (ButtonSelect * 60));
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
    ti_Write(&backgroundScroll, sizeof(backgroundScroll), 1, savegame);
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

uint8_t backgroundList[5] = {0, 1, 2, 1, 0};

void main(void)
{
   //close any files that may have been left open from the last program:
    ti_CloseAll();

    //initialize GFX libraries:
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(jetpack_palette, sizeof_jetpack_palette, 0);
    gfx_SetTransparentColor(0);

    //read from appvar if it exists, fails if it returns 0
    savegame = ti_Open(DATA_APPVAR, "r");
    if(savegame == 0)
    {
        save_state();
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
    CopyPasta(barryHit_array_optimized[randInt(0, 2)], barryHit_resized, 6, 0);

    //prepare the jetpack:
    CopyPasta(jetpack, jetpack_resized, 2, 0);

    //flipped zapper sprites:
    for(uint8_t i = 0; i < 4; ++i)
    {
        zapper_array_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_FlipSpriteX(zapper_array_optimized[i], zapper_array_flipped[i]);
    }

    //horizontal zapper sprites:
    for(uint8_t i = 0; i < 4; ++i)
    {
        horizontal_zapper[i] = gfx_MallocSprite(32, 32);
        horizontal_zapper_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_RotateSpriteC(zapper_array_optimized[i], horizontal_zapper[i]);
        gfx_RotateSpriteC(zapper_array_flipped[i], horizontal_zapper_flipped[i]);
    }

    //horizontal zapper beam:
    horizontal_beam = gfx_MallocSprite(beam_width, beam_height);
    gfx_RotateSpriteC(beam, horizontal_beam);

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

    for(uint8_t i = 0; i < 3; ++i)
    {
        shutdown_tiles_flipped[i] = gfx_MallocSprite(30, 44);
        gfx_FlipSpriteY(shutdown_tiles[i], shutdown_tiles_flipped[i]);
    }

    //start up a timer for FPS monitoring, do not move:
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    //best scan mode according to the angry lettuce man:
    kb_SetMode(MODE_3_CONTINUOUS);

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
        ti_Read(&backgroundScroll, sizeof(backgroundScroll), 1, savegame);
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
        spawnDelay = 200;
        
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

        //if there's a bug, it's because the animation values are funky, check them first:
        missiles.iconAnimate = 0;
        missiles.animationToggle = -1;

        lasers.deactivated = MaxLasers;

        jetpackEntity.theta = 0;
        jetpackEntity.bounce = 0;
        jetpackEntity.h_accel = 2;

        clrObjects();
    }

    //Loop until clear is pressed:
    do
    {
        //update keys, fixes bugs with update errors that can lead to softlocks:
        kb_Scan();

        //controls backgroundScroll, 92 is for background sprite width:
        if((backgroundScroll + scrollSpeed) >= 92)
        {
            //seamlessly reset the position and increment it:
            backgroundScroll -= (92 - scrollSpeed);

            //move the background list and add a new entry at the end:
            for(uint8_t i = 0; i < 4; ++i)
            {
                backgroundList[i] = backgroundList[i + 1];
            }
            backgroundList[4] = randInt(0, 2);
        }
        else
        {
            backgroundScroll += scrollSpeed;
        }

        //spawns stuff, SO much better than the debug methods I originally used, lasers act weird sometimes though:
        if((spawnDelay < 1) && (lasers.x < 1))
        {
            randObject = randInt(0, 21);
            //randObject = randInt(0,0);

            if(randObject == 0)
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
            else if(randObject < 3)
            {
                //sets coin coordinates from coordinate lists:
                randVar = randInt(30, 150);
                coins.formation = randInt(0, (COIN_FORMATIONS - 1));
                for(uint8_t i = 0; i < MaxCoins; ++i)
                {
                    coins.x[i] = (COIN_ORIGIN + ctx[coins.formation][i]);
                    coins.y[i] = (randVar + cty[coins.formation][i]);
                    coins.animation[i] = 0;
                }
                spawnDelay = 500;
            }
            else if(randObject < 6)
            {
                //spawns missiles (and missile swarms if I implement them):
                for(uint8_t i = 0; i < MaxMissiles; ++i)
                {
                    //if there's a missile offscreen, put it into play:
                    if((missiles.x[i] > MISSILE_ORIGIN) || (missiles.x[i] < -54))
                    {
                        missiles.x[i] = MISSILE_ORIGIN;
                        missiles.y[i] = 10 * randInt(2, 18);

                        i = MaxMissiles;
                    }
                }

                //missiles have their own delay that keeps things from spawning in that shouldn't,
                //such as coins and lasers (currently doesn't work):
                missileDelay = 1466;
            }
            else if((randObject > 3) && (randObject <= 20))
            {
                //randomly generates zapper coordinates and lengths:
                for(uint8_t i = 0; i < MaxZappers; ++i)
                {
                    //if the zapper isn't in play:
                    if((zappers.x[i] >= ZAPPER_ORIGIN) || (zappers.x[i] < -100))
                    {
                        //put it in play:
                        zappers.x[i] = ZAPPER_ORIGIN - 1;

                        //zapper length is always set the same way, badly:
                        zappers.length[i] = randInt(2, 4);

                        //if randInt returns anything that isn't zero, make a vertical zapper:
                        if(randInt(0, 2))
                        {
                            zappers.orientation[i] = 0;

                            //vertical zappers can appear from y-20 to y-170:
                            zappers.y[i] = 20 * randInt(1, 9 - (zappers.length[i] / 2));

                            //delay the next spawn:
                            spawnDelay = randInt(140, 180);
                        }
                        else //if zero, then it's a horizontal zapper:
                        {
                            zappers.orientation[i] = 1;

                            //horizontal zappers do be spawnin' everywhere tho:
                            zappers.y[i] = (20 * randInt(1, 10)) - 5;

                            //delay the next spawping with respect for the elongation of the zapper:
                            spawnDelay = (zappers.length[i] * 10) + randInt(140, 180);
                        }

                        //if we made a zapper, stop making more:
                        i = MaxZappers;
                    }
                }
            }
        }
        else
        {
            spawnDelay -= scrollSpeed;

            //if missile delay isn't zero, bring it closer to it:
            if(missileDelay)
            {
                missileDelay -= scrollSpeed;
            }
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
                        CopyPasta(barryHit_array_optimized[randInt(0, 2)], barryHit_resized, 6, 0);
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
        else                        //but on the flip side, this else might be worse looking...
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

        //this is the best way I've found to draw the backgrounds, smaller and faster than a smart system:
        for(uint8_t i = 0; i < 5; ++i)
        {
            gfx_Sprite(background_tiles[backgroundList[i]], (i * 92) - backgroundScroll, 0);
        }

        //draws avatar depending on health 'n stuff:
        if(save_data.health > 0)
        {
            //draws the avatar, everything is layered over it:
            gfx_TransparentSprite_NoClip(barryRun_array[avatar.playerAnimation / 2], avatar.x, avatar.y);

            //bit that draws exhaust when in flight:
            if(avatar.exhaustAnimation < 18)
            {
                gfx_TransparentSprite_NoClip(exhaust_array_optimized[avatar.exhaustAnimation / 2], avatar.x + randInt(1, 3), avatar.y + 31);
                gfx_TransparentSprite_NoClip(nozzle, avatar.x + 4, avatar.y + 31);
            }
        }
        else
        {
            //if Barry is ded and his body hath struck the ground three times, then the end times are upon us:
            if(avatar.corpseBounce >= 3)
            {
                //I hardcoded barry's corpse's Y-pos for effeciency:
                gfx_TransparentSprite_NoClip(barryDed_array_optimized[1], avatar.x, 210);
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
                    gfx_TransparentSprite_NoClip(sparkle, coins.x[i] - 13, coins.y[i] - 1);

                    coins.x[i] = 1020;
                    ++save_data.monies;
                }
                else
                {
                    gfx_TransparentSprite(coin_array_optimized[(coins.animation[i] / 10)], (coins.x[i] - 12), coins.y[i]);

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
        for(uint8_t i = 0; i < MaxZappers; ++i)
        {
            if(zappers.x[i] != ZAPPER_ORIGIN)
            {
                //if orientation is zero, use data to draw vertical sprites:
                if(!zappers.orientation[i])
                {
                    for(uint8_t i_the_sequel = 0; i_the_sequel < zappers.length[i]; ++i_the_sequel)
                    {
                        gfx_TransparentSprite(beam, zappers.x[i] - 14, zappers.y[i] + 16 + (i_the_sequel * 10));
                    }

                    //draw zapper pairs and beams with distance of zapperLength between them:
                    gfx_TransparentSprite(zapper_array_flipped[zappers.animate/3], zappers.x[i] - 25, zappers.y[i] - 7);
                    gfx_TransparentSprite(zapper_array_optimized[zappers.animate/3], zappers.x[i] - 25, zappers.y[i] + 7 + (zappers.length[i] * 10));

                    //collision for vertical zappers:
                    if((save_data.health > 0) && (gfx_CheckRectangleHotspot(zappers.x[i] - 14, zappers.y[i] + 2, 10, (zappers.length[i] * 10) + 30, avatar.x + 2, avatar.y, 18, 37)))
                    {
                        --save_data.health;

                        //screen flashes yellow:
                        deathColor = 4;
                    }
                }
                else //if the orientation is not zero:
                {
                    //draw the horizontal beam, which WAS painful as hekk to do mathwise before I changed everything. Again.
                    for(uint8_t i_the_sequel = 0; i_the_sequel < (zappers.length[i] * 10); i_the_sequel += 10)
                    {
                        //horizontal zappers are why I had to change all my x-values to int24 instead of uint24:
                        gfx_TransparentSprite(horizontal_beam, zappers.x[i] + 32 + i_the_sequel, zappers.y[i] + 11);
                    }

                    //left side:
                    gfx_TransparentSprite(horizontal_zapper[zappers.animate/3], zappers.x[i], zappers.y[i]);
                    //right side:
                    gfx_TransparentSprite(horizontal_zapper_flipped[zappers.animate/3], zappers.x[i]+32+(zappers.length[i]*10), zappers.y[i]);
                
                    //collision for horizontal zappers, a 68-88x10 rectangle:
                    if((save_data.health > 0) && (gfx_CheckRectangleHotspot(zappers.x[i]+8, zappers.y[i], (zappers.length[i]*10)+48, 10, avatar.x, avatar.y, 20, 37)))
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
        for(uint8_t i = 0; i < MaxMissiles; ++i)
        {
            if(missiles.x[i] <= MISSILE_ORIGIN)
            {
                if(missiles.x[i] < 366)
                {
                    gfx_TransparentSprite(missile_array_optimized[missiles.animation / 2], missiles.x[i] - 46, missiles.y[i] - 18);

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
                    gfx_TransparentSprite_NoClip(missileIncoming_tiles[missiles.iconAnimate / 3], 281 + randInt(-1, 1), missiles.y[i] - 16 + randInt(-1, 1));
                }
                else if(missiles.x[i] < MISSILE_ORIGIN)
                {
                    //plenty of time to dodge (at the beginning at least)
                    gfx_TransparentSprite_NoClip(missileWarning_tiles[missiles.iconAnimate / 2], 293, missiles.y[i] - 11);

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

                //missiles travel by 6 pixels each frame. It was surprisingly tedious to figure that out
                //from frame-by-frame reviewing of missiles; however, it's too slow on the calc.
                missiles.x[i] -= scrollSpeed + 8;
            }
        }

        //bit for lasers, lots of moving parts:
        for(uint8_t i = 0; i < laserMax[lasers.formation]; ++i)
        {
            if(lasers.x > 0)
            {
                //reset the laser animations:
                if((lasers.x < 20) && (lasers.deactivated < laserMax[lasers.formation]))
                {
                    lasers.animation = 0;

                    //draw an inactive laser:
                    gfx_TransparentSprite(powering_tile_0, lasers.x - 19, lasers.y[i]);
                    gfx_TransparentSprite(powering_tile_0, 320 - lasers.x, lasers.y[i]);
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
                            gfx_TransparentSprite_NoClip(shutdown_tiles[lasers.animation / 3], 5, lasers.y[i] - 16);
                            gfx_TransparentSprite_NoClip(shutdown_tiles_flipped[lasers.animation / 3], 285, lasers.y[i] - 16);

                            gfx_RLETSprite(laser_tiles[(lasers.animation / 3) + 1], 35, lasers.y[i]);
                        }
                        else if(lasers.lifetime[i] < 59)
                        {
                            //firing lasers:
                            gfx_TransparentSprite_NoClip(firing_tiles[lasers.animation / 3], 5, lasers.y[i] - 11);
                            gfx_TransparentSprite_NoClip(firing_tiles_flipped[lasers.animation / 3], 285, lasers.y[i] - 11);

                            gfx_RLETSprite(laser_tile_0, 35, lasers.y[i]);

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

                            gfx_Line(20, lasers.y[i] + 7, 300, lasers.y[i] + 7);

                            gfx_Circle(11, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle(11, lasers.y[i]+7, 6 + ((lasers.lifetime[i]-50)/4));

                            gfx_Circle(308, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle(308, lasers.y[i]+7, 8 + ((lasers.lifetime[i]-50)/4));
                        }

                        //update the laser's timer:
                        --lasers.lifetime[i];
                    }
                }

                if(lasers.lifetime[i] < 109)
                {
                    gfx_TransparentSprite(powering_tiles[(lasers.animation / 3) + 1], lasers.x - 19, lasers.y[i]);
                    gfx_TransparentSprite(powering_tiles_flipped[(lasers.animation / 3) + 1], 320 - lasers.x, lasers.y[i]);
                }
                else
                {
                    gfx_TransparentSprite(powering_tile_0, lasers.x - 19, lasers.y[i]);
                    gfx_TransparentSprite(powering_tiles_flipped[0], 320 - lasers.x, lasers.y[i]);
                }
            }
        }

        //sets a new highscore:
        if(save_data.highscore < ((save_data.distance += scrollSpeed) /15))
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
        FPS = (32768 / timer_1_Counter);

        //time is frozen for a delay, and I wanna make a Jo-Jo reference but I don't speak Japanese
        if(FPS > 25)
        {
            delay(40 - (1000 / FPS));
        }

        //pause menu controls and drawing, can be accessed as long as in main loop:
        if(kb_Data[1] & kb_Del)
        {
            //drawing the base color for all the menu stuff:
            gfx_FillScreen(6);

            //using that draw_button() function to write out the 3 menu options:
            draw_button(0);
            draw_button(1);
            draw_button(2);

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
                    draw_button(menuSelect);

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

                    gfx_Sprite_NoClip(pauseButtonOff_tiles[menuSelect], 80, 35 + (menuSelect * 60));
                    gfx_Sprite_NoClip(pauseButtonOff_tile_4, 220, 35 + (menuSelect * 60));

                    //middle of pressed button is drawn (6 tiles):
                    for(uint8_t i = 0; i < 120; i += 20)
                    {
                        gfx_Sprite_NoClip(pauseButtonOff_tile_3, 100 + i, 35 + (menuSelect * 60));
                    }

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

            //that dumb break means I can't use a switch case (yet) so it just exits the program loop when "Quit" is selected.
            if(menuSelect == 0)
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

        //timer reset and "ZA WARUDO" is over (iS tHaT A jOJo rEfERenCe):
        timer_1_Counter = 0;

    } while (!(kb_Data[6] & kb_Clear) && (avatar.deathDelay != 50));

    if(!(kb_Data[6] & kb_Clear) && (save_data.health < 1))
    {
        //reset distance to flag that a game is no longer in progress:
        save_data.distance = 0;

        gfx_FillScreen(1);
        gfx_SetTextScale(3, 3);
        gfx_PrintStringXY("Congration!", 15, 70);
        gfx_PrintStringXY("You recieved:", 15, 130);
        gfx_PrintStringXY("Death", 15, 160);
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
}

//CONGRATULATIONS, YOU FOUND THE END OF THE FILE! I HOPE PEEKING BEHIND THE CURTAIN WAS WORTH YOUR SANITY!
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
#define ZAPPER_ORIGIN 330
#define MISSILE_ORIGIN 1466

#define DATA_APPVAR "JTPKDAT"

//time it takes to complete the game loop, used to control the FPS; if it overflows
//then we have real speed problems:
uint8_t frameTime;

//speed of scrolling and time before incrementing it:
uint8_t scrollSpeed;
uint16_t incrementDelay;

//measures timings for delays between spawning coins, obstacles, etc.:
int24_t spawnDelay = 200;
uint24_t missileDelay;

//used for a bad background scroll function that is actually the best for this scenario:
int24_t backgroundScroll;

//all the data used to control Barry's position, acceleratoin, etc. are consolidated under one pointer.
//x and y positions, time 2nd is pressed/released to fly/fall, animation frame, animation cycling control, and jetpack animation frame:
typedef struct
{
    uint24_t x;
    uint8_t y;
    int8_t inputDuration;
    uint8_t playerAnimation;
    uint8_t playerAnimationToggle;
    uint8_t exhaustAnimation;
}
avatar_t; //looks gross, but it saves 309 bytes in read/write calls to the appvar. It's staying.

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name
//my child if I had the desire to marry and have children.
avatar_t avatar;

typedef struct
{
    int8_t health;        //the number of hits Barry can take
    uint32_t monies;      //money collected in the run
    uint32_t collegeFund; //total money collected from all runs, can be used to purchase stuff (?)
    uint32_t distance;    //distance travelled in the run
    uint32_t highscore;   //highest distance travelled in all runs
}
game_data_t;

//all important game data, again, all in one clean struct pointer:
game_data_t save_data;

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

//hey looky, a coin struct type for easy use, WHY DIDN'T I MAKE THIS EARLIER?
//stores x and y positions, along with the coin's animation frame:
typedef struct
{
    uint24_t x;
    uint8_t y;
    uint8_t animation;
}
coin_t;

//initialize some coins, about 30 is the max:
coin_t coin[MaxCoins];

//coin formation variable to keep track of coin list shapes:
uint8_t coinFormation;

//again, WHY did I not make these before...
//laser x and y positions, length, and animation frame:
typedef struct
{
    uint24_t x;
    uint8_t y;
    uint8_t length;
    uint8_t animation;
}
zapper_t;

zapper_t zappers[MaxZappers];

//I think the ZDS toolchain broke them and doubled the program size, I love LLVM
//missile x and y positions:
typedef struct
{
    uint24_t x;
    uint8_t y;
}
missile_t;

missile_t missiles[MaxMissiles];

//keep track of animations for missiles:
int8_t missile_icon_animate;
int8_t missileAnimate;
int8_t MAvalue = -1;

//the point is, updates are good; ergo, Windows is exciting
//y position (x position is shared among all lasers), laser's time to function:
typedef struct
{
    uint8_t y;
    uint24_t lifetime;
}
laser_t;

laser_t lasers[MaxLasers];

//all lasers have a universal X that doesn't move very far:
int8_t laserX;
//laser animation variable
int8_t laserAnimate;
//which laser formation is being used:
uint8_t laserFormation;
//keep track of how many lasers have fired already:
uint8_t deadLasers;

//flipped sprites and animations:
gfx_sprite_t *zapper_tiles_flipped[3];
gfx_sprite_t *electric_animation[8];
gfx_sprite_t *powering_tiles_flipped[4];
gfx_sprite_t *firing_tiles_flipped[3];
gfx_sprite_t *shutdown_tiles_flipped[3];

ti_var_t savegame;

//gfx_CheckRectangleHotspot but only the Y checks, since 4 C++ masters write better code than me:
#define Yspot(master_y, master_height, test_y, test_height) \
    (((test_y) < ((master_y) + (master_height))) && \
    (((test_y) + (test_height)) > (master_y)))

//clears all objects from gameplay by moving their X-coords out of bounds:
void delObjects()
{
    for (uint8_t i = 0; i < coin_max[coinFormation]; ++i)
    {
        coin[i].x = 2000;
    }

    for (uint8_t i = 0; i < MaxZappers; ++i)
    {
        zappers[i].length = 0;
        zappers[i].x = (ZAPPER_ORIGIN + 1);
    }

    for (uint8_t i = 0; i < MaxMissiles; ++i)
    {
        missiles[i].x = (MISSILE_ORIGIN + 1);
    }

    laserX = 0;
}

//a function for drawing buttons, will hopefully save on flash size and stack usage:
void draw_button(uint8_t ButtonSelect)
{
    //first 20 pixels of the button:
    gfx_Sprite(pauseButtonOn_tiles[ButtonSelect], 70, 33 + (ButtonSelect * 60));

    //the middle bits are drawn 7 times:
    for(uint8_t i = 0; i < 140; i += 20)
    {
        gfx_Sprite(pauseButtonOn_tile_3, 90 + i, 33 + (ButtonSelect * 60));
    }

    //and the last 20 pixels:
    gfx_Sprite(pauseButtonOn_tile_4, 230, 33 + (ButtonSelect * 60));

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

    //game environment variables:
    ti_Write(&scrollSpeed, sizeof(scrollSpeed), 1, savegame);
    ti_Write(&incrementDelay, sizeof(incrementDelay), 1, savegame);
    ti_Write(&backgroundScroll, sizeof(backgroundScroll), 1, savegame);
    ti_Write(&spawnDelay, sizeof(spawnDelay), 1, savegame);

    //single pointer to avatar struct:
    ti_Write(&avatar, sizeof(avatar), 1, savegame);

    //coin variables:
    ti_Write(&coin, sizeof(coin), 1, savegame);
    ti_Write(&coinFormation, sizeof(coinFormation), 1, savegame);

    //zapper variables:
    ti_Write(&zappers, sizeof(zappers), 1, savegame);

    //missile variables:
    ti_Write(&missiles, sizeof(missiles), 1, savegame);
    ti_Write(&missile_icon_animate, sizeof(missile_icon_animate), 1, savegame);
    ti_Write(&missileAnimate, sizeof(missileAnimate), 1, savegame);
    ti_Write(&MAvalue, sizeof(MAvalue), 1, savegame);

    //laser variables:
    ti_Write(&lasers, sizeof(lasers), 1, savegame);
    ti_Write(&laserX, sizeof(laserX), 1, savegame);
    ti_Write(&laserFormation, sizeof(laserFormation), 1, savegame);
    ti_Write(&deadLasers, sizeof(deadLasers), 1, savegame);
    ti_Write(&laserAnimate, sizeof(laserAnimate), 1, savegame);
}

void main(void)
{
    //some initial values for the avatar struct:
    avatar.playerAnimationToggle = 1;
    avatar.playerAnimation = 3;
    avatar.exhaustAnimation = 18;

    //close any files that may have been left open from the last program:
    ti_CloseAll();

    //read from appvar if it exists, fails if it returns 0
    savegame = ti_Open(DATA_APPVAR, "r");
    if (savegame == 0)
    {
        save_state();
    }

    //flipped zapper sprites:
    for (uint8_t i = 0; i < 3; ++i)
    {
        zapper_tiles_flipped[i] = gfx_MallocSprite(18, 18);
        gfx_FlipSpriteX(zapper_tiles[i], zapper_tiles_flipped[i]);
    }

    for (uint8_t i = 0; i < 8; ++i)
    {
        //mallocing full flipped electric_animation array:
        electric_animation[i] = gfx_MallocSprite(32, 32);
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        //zapper lightning:
        electric_animation[i] = electric_tiles[i];
        gfx_FlipSpriteX(electric_animation[i], electric_animation[i + 2]);
        gfx_FlipSpriteY(electric_animation[i], electric_animation[i + 4]);
        gfx_RotateSpriteHalf(electric_animation[i], electric_animation[i + 6]);
    }

    //flipping laser powering up animations:
    for (uint8_t i = 0; i < 4; ++i)
    {
        powering_tiles_flipped[i] = gfx_MallocSprite(19, 15);
        gfx_FlipSpriteY(powering_tiles[i], powering_tiles_flipped[i]);
    }

    //flipping laser sprites:
    for (uint8_t i = 0; i < 3; ++i)
    {
        firing_tiles_flipped[i] = gfx_MallocSprite(30, 37);
        gfx_FlipSpriteY(firing_tiles[i], firing_tiles_flipped[i]);
    }

    for (uint8_t i = 0; i < 3; ++i)
    {
        shutdown_tiles_flipped[i] = gfx_MallocSprite(30, 44);
        gfx_FlipSpriteY(shutdown_tiles[i], shutdown_tiles_flipped[i]);
    }

    //initialize GFX libraries:
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(jetpack_palette, sizeof_jetpack_palette, 0);
    gfx_SetTransparentColor(0);

    //start up a timer for FPS monitoring, do not move:
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    //best scan mode according to the angry lettuce man:
    kb_SetMode(MODE_3_CONTINUOUS);

    //update all of our gameplay data from the save data:
    ti_Read(&save_data, sizeof(save_data), 1, savegame);

    //when I first started using C, I asked some friends if there were GOTO statements.
    //They proved they were good friends, and told me "No, that's stupid". I'm glad they lied.
    GAMESTART:
    //But it's still sometimes okay.

    //load all the appvar data as our "starting point" if distance isn't 0
    if (save_data.distance)
    {
        //game environment variables:
        ti_Read(&scrollSpeed, sizeof(scrollSpeed), 1, savegame);
        ti_Read(&incrementDelay, sizeof(incrementDelay), 1, savegame);
        ti_Read(&backgroundScroll, sizeof(backgroundScroll), 1, savegame);
        ti_Read(&spawnDelay, sizeof(spawnDelay), 1, savegame);

        //single pointer to avatar struct:
        ti_Read(&avatar, sizeof(avatar), 1, savegame);

        //coin variables:
        ti_Read(&coin, sizeof(coin), 1, savegame);
        ti_Read(&coinFormation, sizeof(coinFormation), 1, savegame);

        //zapper variables:
        ti_Read(&zappers, sizeof(zappers), 1, savegame);

        //missile variables:
        ti_Read(&missiles, sizeof(missiles), 1, savegame);
        ti_Read(&missile_icon_animate, sizeof(missile_icon_animate), 1, savegame);
        ti_Read(&missileAnimate, sizeof(missileAnimate), 1, savegame);
        ti_Read(&MAvalue, sizeof(MAvalue), 1, savegame);

        //laser variables:
        ti_Read(&lasers, sizeof(lasers), 1, savegame);
        ti_Read(&laserX, sizeof(laserX), 1, savegame);
        ti_Read(&laserFormation, sizeof(laserFormation), 1, savegame);
        ti_Read(&deadLasers, sizeof(deadLasers), 1, savegame);
        ti_Read(&laserAnimate, sizeof(laserAnimate), 1, savegame);

        ti_CloseAll();
    }
    else
    {
        delObjects();

        //reset variables for when a game starts:
        save_data.distance = 0;
        scrollSpeed = 6;
        incrementDelay = 0;
        avatar.x = 24;
        avatar.y = 185;
        avatar.inputDuration = 0;
        avatar.exhaustAnimation = 18;
        save_data.health = 1;
        spawnDelay = 100;
        save_data.monies = 0;
        deadLasers = MaxLasers;
    }

    //Loop until clear is pressed:
    do
    {
        //update keys, fixes bugs with update errors that can lead to softlocks:
        kb_Scan();

        //controls backgroundScroll, 192 is for background sprite width:
        if ((backgroundScroll - scrollSpeed) <= 0)
        {
            backgroundScroll += (192 - scrollSpeed);
        } else {
            backgroundScroll -= scrollSpeed;
        }

        //very small bit of code to increase speed with a decreasing frequency over time, max of 12:
        if ((incrementDelay >= ((scrollSpeed - 5) * 250)) && (scrollSpeed < 12))
        {
            ++scrollSpeed;
            incrementDelay = 0;
        } else {
            ++incrementDelay;
        }

        //spawns stuff, SO much better than the debug methods I originally used:
        if (spawnDelay <= 0)
        {
            randObject = randInt(0, 21);
            //randObject = randInt(0,0);

            if (randObject == 0)
            {
                //picks a random formation from the data arrays in the laser_formations file:
                laserFormation = randInt(0, (LASER_FORMATIONS - 1));

                //give each laser their Y-axis:
                for (uint8_t i = 0; i < laserMax[laserFormation]; ++i)
                {
                    lasers[i].y = lsrY[laserFormation][i];
                    lasers[i].lifetime = halfLife[laserFormation][i];
                }

                laserX = 1;

                deadLasers = 0;

                spawnDelay = scrollSpeed * 40;
            }
            else if (randObject < 3)
            {

                //sets coin coordinates from coordinate lists:
                randVar = randInt(30, 150);
                coinFormation = randInt(0, (COIN_FORMATIONS - 1));
                for (uint8_t i = 0; i < MaxCoins; ++i)
                {
                    coin[i].x = (COIN_ORIGIN + ctx[coinFormation][i]);
                    coin[i].y = (randVar + cty[coinFormation][i]);
                    coin[i].animation = 0;
                }

                spawnDelay = 500;
            }
            else if (randObject < 6)
            {

                //spawns missiles (and missile swarms if I implement them):
                for (uint8_t i = 0; i < MaxMissiles; ++i)
                {
                    if (missiles[i].x > MISSILE_ORIGIN)
                    {
                        missiles[i].x = MISSILE_ORIGIN;
                        missiles[i].y = 10 * randInt(2, 18);

                        i = MaxZappers;
                    }
                }

                //missiles have their own delay that keeps things from spawning in that shouldn't,
                //such as coins and lasers (currently doesn't work):
                missileDelay = 1466;
            }
            else if ((randObject > 3) && (randObject <= 20))
            {

                //randomly generates zapper coordinates and lengths:
                for (uint8_t i = 0; i < MaxZappers; ++i)
                {
                    if (zappers[i].x > ZAPPER_ORIGIN)
                    {
                        zappers[i].x = ZAPPER_ORIGIN;

                        zappers[i].length = randInt(2, 4);
                        zappers[i].y = 10 * randInt(2, 19 - zappers[i].length);

                        i = MaxZappers;
                    }
                }

                spawnDelay = 200;
            }
        }
        else
        {
            spawnDelay -= scrollSpeed;
            missileDelay -= scrollSpeed;
        }

        //run controls until Barry gets wasted, then bounces his corpse around:
        if (save_data.health > 0)
        {
            //when flying:
            if (kb_Data[1] & kb_2nd)
            {
                //add to avatar.inputDuration, which is added to avatar.y:
                if ((avatar.y > 20) && (avatar.inputDuration < 12))
                {
                    avatar.inputDuration += 1;
                }

                //numbers for jetpack output (bullets or fire or whatever):
                if (avatar.exhaustAnimation < 7)
                {
                    ++avatar.exhaustAnimation;
                } else if (avatar.exhaustAnimation > 7) {
                    avatar.exhaustAnimation = 0;
                }

            //when falling:
            } else {
                //subtract from avatar.inputDuration, added to avatar.y (it can be a negative number):
                if ((avatar.y < (185)) && (avatar.inputDuration > -10))
                {
                    avatar.inputDuration -= 1;
                }

                //increases numbers for jetpack powering down animation:
                if (avatar.exhaustAnimation < 7)
                {
                    avatar.exhaustAnimation = 8;
                }
                else if (avatar.exhaustAnimation < 11)
                {
                    ++avatar.exhaustAnimation;
                }
                else
                {
                    avatar.exhaustAnimation = 18;
                }
            }

            //SAX got pretty mad about this part, turns out a linear equation can model a curve and
            //is actually better than a cubic function:
            avatar.y -= (avatar.inputDuration);

            //sees if Y-value rolled past 20 or 186, figures out if it was going up
            //or down, and auto-corrects accordingly (FUTURE ME; IT'S OPTIMIZED ENOUGH PLEASE DON'T WASTE ANY MORE TIME):
            if ((avatar.y > (186)) || (avatar.y < 20))
            {
                if (avatar.inputDuration > 0)
                {
                    avatar.y = 20;
                } else {
                    avatar.y = 185;
                }
                avatar.inputDuration = 0;
            }
        }

        //bit that runs avatar animations:
        if (avatar.y < 185)
        {
            avatar.playerAnimation = 9;
        }
        else
        {

            if (avatar.playerAnimation == 9)
            {
                avatar.playerAnimation = 3;
            }

            if ((avatar.playerAnimation < 1) || (avatar.playerAnimation > 7))
            {
                avatar.playerAnimationToggle *= -1;
            }

            avatar.playerAnimation += avatar.playerAnimationToggle;
        }

        //control missile warning and incoming animations:
        if ((missile_icon_animate < 1) || (missile_icon_animate > 4))
        {
            MAvalue *= -1;
        }
        missile_icon_animate += MAvalue;

        //animate missile sprites:
        if (missileAnimate < 12)
        {
            ++missileAnimate;
        }
        else
        {
            missileAnimate = 0;
        }

        //move lasers into play when needed:
        if (deadLasers < laserMax[laserFormation])
        {
            spawnDelay = scrollSpeed * 40;

            if (laserX < 20)
            {
                ++laserX;
            }
        }
        else if ((deadLasers >= laserMax[laserFormation]) && (laserX > 0))
        {
            --laserX;
        }

        //mess with number for laser animations
        if (laserAnimate < 8)
        {
            ++laserAnimate;
        } else {
            laserAnimate = 0;
        }

        //this is the best way I've found to draw the backgrounds, smaller and faster than a smart system:
        gfx_Sprite(background, backgroundScroll - background_width, 0);
        gfx_Sprite(background, backgroundScroll, 0);
        gfx_Sprite(background, backgroundScroll + background_width, 0);

        //draws the avatar, everything is layered over it:
        gfx_TransparentSprite_NoClip(avatar_tiles[avatar.playerAnimation / 3], avatar.x, avatar.y - abs((avatar.playerAnimation / 3) - 1));

        //bit that draws exhaust when in flight:
        if (avatar.exhaustAnimation < 18)
        {
            gfx_TransparentSprite_NoClip(exhaust_tiles_optimized[avatar.exhaustAnimation / 2], avatar.x + randInt(1, 3), avatar.y + 31);
            gfx_TransparentSprite_NoClip(nozzle, avatar.x + 4, avatar.y + 31);
        }

        //bit that runs coin collision and movement:
        for (uint8_t i = 0; i < coin_max[coinFormation]; ++i)
        {
            //do things if the coin is less than the origin plus some buffer I made up on the spot:
            if (coin[i].x <= COIN_ORIGIN + 500)
            {
                //collision detection and appropriate sprite drawing:
                if (gfx_CheckRectangleHotspot(avatar.x + 6, avatar.y, 18, 40, coin[i].x - 11, coin[i].y + 1, 10, 10))
                {
                    gfx_TransparentSprite(sparkle, coin[i].x - 13, coin[i].y - 1);

                    coin[i].x = 1020;
                    ++save_data.monies;
                }
                else
                {

                    gfx_TransparentSprite(coin_tiles_optimized[(coin[i].animation / 10)], (coin[i].x - 12), coin[i].y);

                    if ((coin[i].animation < 38) && (coin[i].x < COIN_ORIGIN))
                    {
                        coin[i].animation += 2;
                    }
                    else
                    {
                        coin[i].animation = 0;
                    }
                }

                coin[i].x -= scrollSpeed;
            }
        }

        //bit that calculates zappers 'n stuff:
        for (uint8_t i = 0; i < MaxZappers; ++i)
        {
            //drawing the zapper beams:
            if (zappers[i].x <= ZAPPER_ORIGIN)
            {
                for (uint8_t i_the_sequel = 0; i_the_sequel < zappers[i].length; ++i_the_sequel)
                {
                    gfx_TransparentSprite(beam, zappers[i].x - 14, zappers[i].y + 16 + (i_the_sequel * 10));
                }

                //draw zapper pairs and beams with distance of zapperLength between them:
                if (zappers[i].x < 336)
                {
                    randVar = randInt(0, 1);
                    randVar1 = randInt(0, 1);

                    gfx_TransparentSprite(electric_animation[randVar + 2 + (randVar1 * 4)], zappers[i].x - 25, zappers[i].y - 7);
                    gfx_TransparentSprite(electric_animation[randVar], zappers[i].x - 25, zappers[i].y + 7 + (zappers[i].length * 10));

                    gfx_TransparentSprite(zapper_tiles_flipped[zappers[i].animation / 10], zappers[i].x - 18, zappers[i].y);
                    gfx_TransparentSprite(zapper_tiles[zappers[i].animation / 10], zappers[i].x - 18, zappers[i].y + 14 + (zappers[i].length * 10));

                    if (zappers[i].animation < 28)
                    {
                        zappers[i].animation += 2;
                    }
                    else
                    {
                        zappers[i].animation = 0;
                    }

                    //collision for zappers:
                    if ((save_data.health > 0) && (gfx_CheckRectangleHotspot(avatar.x + 6, zappers[i].y + 2, 18, (zappers[i].length * 10) + 30, zappers[i].x - 14, avatar.y, 10, 50)))
                    {
                        --save_data.health;
                    }
                }

                zappers[i].x -= scrollSpeed;
            }
        }

        //bit that draws and calculates the missiles 'o death:
        for (uint8_t i = 0; i < MaxMissiles; ++i)
        {
            if (missiles[i].x <= MISSILE_ORIGIN)
            {
                if (missiles[i].x < 366)
                {
                    gfx_TransparentSprite(missile_tiles_optimized[missileAnimate / 2], missiles[i].x - 46, missiles[i].y - 18);

                    if ((save_data.health > 0) && (gfx_CheckRectangleHotspot(missiles[i].x - 45, avatar.y, 19, 40, avatar.x + 6, missiles[i].y - 5, 18, 12)))
                    {
                        --save_data.health;
                    }

                    //I'm using the X-position as timer, meaning there's less warning as time goes on:
                }
                else if (missiles[i].x < 641)
                {
                    //AW CRAP HERE COME DAT BOI!
                    gfx_TransparentSprite(missileIncoming_tiles[missile_icon_animate / 3], 281 + randInt(-1, 1), missiles[i].y - 16 + randInt(-1, 1));
                }
                else if (missiles[i].x < MISSILE_ORIGIN)
                {
                    //plenty of time to dodge (at the beginning at least)
                    gfx_TransparentSprite(missileWarning_tiles[missile_icon_animate / 2], 293, missiles[i].y - 11);

                    //tracking on avatar
                    if (missiles[i].y < (avatar.y + 20))
                    {
                        missiles[i].y += 2;
                    }
                    else if (missiles[i].y > (avatar.y + 21))
                    {
                        missiles[i].y -= 2;
                    }
                }

                //missiles travel by 6 pixels each frame. It was surprisingly tedious to figure that out
                //from frame-by-frame reviewing of missiles; however, it's too slow on the calc.
                missiles[i].x -= scrollSpeed + 8;
            }
        }

        //bit for lasers, lots of moving parts:
        for (uint8_t i = 0; i < laserMax[laserFormation]; ++i)
        {
            if (laserX > 0)
            {
                //reset the laser animations:
                if ((laserX < 20) && (deadLasers < laserMax[laserFormation]))
                {
                    laserAnimate = 0;

                    //draw an inactive laser:
                    gfx_TransparentSprite(powering_tile_0, laserX - 19, lasers[i].y);
                    gfx_TransparentSprite(powering_tile_0, 320 - laserX, lasers[i].y);
                }
                else
                {
                    //if they are finished, finish the animation sequence and hide them again:
                    if (deadLasers >= laserMax[laserFormation])
                    {
                        laserAnimate = 0;

                        //if they aren't dead and have moved, run through their animations and attacks:
                    }
                    else
                    {

                        //check to see if the half-life ended:
                        if (lasers[i].lifetime < 1)
                        {
                            ++deadLasers;
                        }
                        else if (lasers[i].lifetime < 9)
                        {

                            //lasers running out of juice:
                            gfx_TransparentSprite(shutdown_tiles[laserAnimate / 3], 5, lasers[i].y - 16);
                            gfx_TransparentSprite(shutdown_tiles_flipped[laserAnimate / 3], 285, lasers[i].y - 16);

                            gfx_RLETSprite(laser_tiles[(laserAnimate / 3) + 1], 35, lasers[i].y);
                        }
                        else if (lasers[i].lifetime < 59)
                        {
                            //firing lasers:
                            gfx_TransparentSprite(firing_tiles[laserAnimate / 3], 5, lasers[i].y - 11);
                            gfx_TransparentSprite(firing_tiles_flipped[laserAnimate / 3], 285, lasers[i].y - 11);

                            gfx_RLETSprite(laser_tile_0, 35, lasers[i].y);

                            //hitbox for damage. 5 pixel leeway above and below:
                            if ((save_data.health > 0) && Yspot(avatar.y, 35, lasers[i].y, 10))
                            {
                                --save_data.health;
                            }

                            //makes sure the animations play correctly, I don't have time to fix math today:
                            if (lasers[i].lifetime == 9)
                                laserAnimate = 0;
                        }
                        else if (lasers[i].lifetime < 109)
                        {

                            //the simplest drawing code in this hot mess, for powering up:
                            gfx_SetColor(5);

                            gfx_Line(20, lasers[i].y + 7, 300, lasers[i].y + 7);

                            gfx_Circle(11, lasers[i].y + 7, 7 + ((lasers[i].lifetime - 50) / 4));
                            //gfx_Circle(11, lasers[i].y+7, 6 + ((lasers[i].lifetime-50)/4));

                            gfx_Circle(308, lasers[i].y + 7, 7 + ((lasers[i].lifetime - 50) / 4));
                            //gfx_Circle(308, lasers[i].y+7, 8 + ((lasers[i].lifetime-50)/4));
                        }

                        //update the laser's timer:
                        --lasers[i].lifetime;
                    }
                }

                if (lasers[i].lifetime < 109)
                {
                    gfx_TransparentSprite(powering_tiles[(laserAnimate / 3) + 1], laserX - 19, lasers[i].y);
                    gfx_TransparentSprite(powering_tiles_flipped[(laserAnimate / 3) + 1], 320 - laserX, lasers[i].y);
                }
                else
                {
                    gfx_TransparentSprite(powering_tile_0, laserX - 19, lasers[i].y);
                    gfx_TransparentSprite(powering_tiles_flipped[0], 320 - laserX, lasers[i].y);
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
        gfx_PrintInt(frameTime, 1);

        gfx_SetTextScale(2, 2);
        gfx_SetTextXY(10, 10);
        //these numbers are magic and just give good results
        gfx_PrintInt(save_data.distance / 15, 4);
        

        //speedy blitting, but works best when used a lot of cycles BEFORE any new drawing functions:
        gfx_SwapDraw();

        //FPS counter data collection, time "stops" (being relevant) after this point:
        frameTime = (32768 / timer_1_Counter);

        //time is frozen for a delay, and I wanna make a Jo-Jo reference but I don't speak Japanese
        if (frameTime > 25)
        {
            delay(40 - (1000 / frameTime));
        }

        //pause menu controls and drawing, can be accessed as long as in main loop:
        if (kb_Data[1] & kb_Del)
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
                    if (kb_Data[7] & kb_Down)
                    {
                        if (menuSelect > 1)
                        {
                            menuSelect = 0;
                        } else {
                            ++menuSelect;
                        }
                    }

                    //and if up is pressed, subtract from menu select and correct overflow:
                    if (kb_Data[7] & kb_Up)
                    {
                        if (menuSelect < 1)
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
                else if ((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter))
                {
                    debounced = true;

                    gfx_SetColor(6);

                    //edges of larger buttons need to be hidden for smaller buttons:
                    gfx_FillRectangle(69, 32 + (menuSelect * 60), 182, 52);

                    gfx_Sprite(pauseButtonOff_tiles[menuSelect], 80, 35 + (menuSelect * 60));
                    gfx_Sprite(pauseButtonOff_tile_4, 220, 35 + (menuSelect * 60));

                    //middle of pressed button is drawn (6 tiles):
                    for (uint8_t i = 0; i < 120; i += 20)
                    {
                        gfx_Sprite(pauseButtonOff_tile_3, 100 + i, 35 + (menuSelect * 60));
                    }

                    //smol selector:
                    gfx_SetColor(2);
                    gfx_Rectangle(79, 34 + (menuSelect * 60), 162, 48);

                    //button text:
                    gfx_PrintStringXY(pauseOptions[menuSelect], pauseOptionX[menuSelect], 47 + (menuSelect * 60));
                }
                else
                {
                    if (debounced)
                    {
                        break;
                    }
                }

                gfx_Blit(1);

                //simple way to wait until none of the arrow keys are pressed:
                while(((kb_Data[7] & kb_Down) || (kb_Data[7] & kb_Up)) && !(kb_Data[6] & kb_Clear)) kb_Scan();
            }

            //that dumb break means I can't use a switch case (yet) so it just exits the program loop.
            if (menuSelect == 0)
            {
                save_data.distance = 0;
                break;
                //goto main menu
            }
            else if (menuSelect == 1)
            {
                save_data.distance = 0;
                goto GAMESTART;
            }
            //if menuSelect is 2 or anything else it just resumes; everything resets when the pause menu is accessed again.
        }

        //timer reset and "DA WORLDO" is over:
        timer_1_Counter = 0;

    } while (!(kb_Data[6] & kb_Clear) && (save_data.health > 0));

    if (save_data.health < 1)
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

        if (kb_Data[6] != kb_Clear)
        {
            //I still can't believe I used a goto label...
            goto GAMESTART;
        }
    }
    
    //we only need distance and a few other vars, but I made this function AND I'M GONNA USE THE WHOLE FUNCTION.
    save_state();

    //stop libraries, not doing so causes "interesting" results by messing up the color mode and scaling:
    gfx_End();
    ti_CloseAll();
}

//CONGRATULATIONS, YOU FOUND THE END OF THE FILE! I HOPE PEEKING BEHIND THE CURTAIN WAS WORTH YOUR SANITY!
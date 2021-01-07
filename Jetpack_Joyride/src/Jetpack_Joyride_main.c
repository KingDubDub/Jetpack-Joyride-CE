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

#include <compression.h>
#include <graphx.h>
#include <keypadc.h>

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

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name
//my child if I had the desire to marry and have children.
uint16_t avatarX;
uint8_t avatarY;

//avatar's sprite array and values for keeping track of animation frames:
int8_t avatarAnimate = 1;
int8_t displacement = 3;

//variable used for calculating fire animations:
uint8_t exhaustAnimate = 18;

//time it takes to complete the game loop, used to control the FPS; if it overflows
//then we have real problems:
uint8_t frameTime;

//variables for when jetpack is on or not for math calculations:
int8_t holdTime;

//speed of scrolling for avatar, obstacles, map, etc.
uint8_t scrollSpeed;
//value added to scroll speed and the delay that is taken before incrementing it:
uint8_t scrollIncrement;
uint16_t incrementDelay;

//measures timings for delays between spawning coins, obstacles, etc.:
int24_t spawnDelay = 200;
uint24_t missileDelay;

//used for a bad background scroll function that is actually the best for this scenario:
int24_t backgroundScroll;

//stores if Barry done got wasted or not:
int8_t health;

//max monies at $4,294,967,295:
uint32_t monies;

//there's a limit to the distance you can fly in the original game, not sure I'll keep that or not...
uint32_t distance;

//for randomization values that need to be reused:
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

uint24_t coinX[MaxCoins];
uint8_t coinY[MaxCoins];
//coin formation variable to keep track of coin lists:
uint8_t coinFormation;
//values for keeping track of coin animation sprites, all start at zero:
uint8_t coinAnimate[MaxCoins];

//arrays for zapper coordinates:
uint24_t zapperY[MaxZappers];
uint16_t zapperX[MaxZappers];
//measured in beam units, 10x10 pixels:
uint8_t zapperLength[MaxZappers];
//zapper animation count, all start at zero:
uint8_t zapperAnimate[MaxZappers];

uint16_t missileX[MaxMissiles];
uint8_t missileY[MaxMissiles];
//keep track of animations for missiles:
int8_t missile_icon_animate;
int8_t MAvalue = -1;
int8_t missileAnimate;

//all lasers have a universal X that doesn't move very far:
int8_t laserX;
//laser Y array:
uint8_t laserY[MaxLasers];
//time till firing:
uint24_t laserLifetime[MaxLasers];
//keep track of laser animations:
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

//gfx_CheckRectangleHotspot but only the Y checks, since 4 C++ masters write better code than me:
#define Yspot(master_y, master_height, test_y, test_height) \
    (((test_y) < ((master_y) + (master_height))) &&         \
     (((test_y) + (test_height)) > (master_y)))

//clears all objects from gameplay by moving their X-coords out of bounds:
void delObjects()
{
    for (uint8_t i = 0; i < coin_max[coinFormation]; ++i)
    {
        coinX[i] = 2000;
    }

    for (uint8_t i = 0; i < MaxZappers; ++i)
    {
        zapperLength[i] = 0;
        zapperX[i] = (ZAPPER_ORIGIN + 1);
    }

    for (uint8_t i = 0; i < MaxMissiles; ++i)
    {
        missileX[i] = (MISSILE_ORIGIN + 1);
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

void main(void)
{
    //flipped zappers:
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

//when I first started using C, I asked some friends if there were GOTO statements.
//They proved they were good friends, and told me "No, that's stupid". I'm glad they lied.
GAMESTART:
    //But it's still sometimes okay.

    //reset variables for when a game starts:
    scrollSpeed = 6;
    incrementDelay = 0;
    avatarX = 24;
    avatarY = 185;
    holdTime = 0;
    exhaustAnimate = 18;
    health = 1;
    spawnDelay = 100;
    distance = 0;
    monies = 0;
    deadLasers = MaxLasers;

    delObjects();

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
                    laserY[i] = lsrY[laserFormation][i];
                    laserLifetime[i] = halfLife[laserFormation][i];
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
                    coinX[i] = (COIN_ORIGIN + ctx[coinFormation][i]);
                    coinY[i] = (randVar + cty[coinFormation][i]);
                    coinAnimate[i] = 0;
                }

                spawnDelay = 500;
            }
            else if (randObject < 6)
            {

                //spawns missiles (and missile swarms if I implement them):
                for (uint8_t i = 0; i < MaxMissiles; ++i)
                {
                    if (missileX[i] > MISSILE_ORIGIN)
                    {
                        missileX[i] = MISSILE_ORIGIN;
                        missileY[i] = 10 * randInt(2, 18);

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
                    if (zapperX[i] > ZAPPER_ORIGIN)
                    {
                        zapperX[i] = ZAPPER_ORIGIN;

                        zapperLength[i] = randInt(2, 4);
                        zapperY[i] = 10 * randInt(2, 19 - zapperLength[i]);

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
        if (health > 0)
        {
            //when flying:
            if (kb_Data[1] & kb_2nd)
            {
                //add to holdTime, which is added to avatarY:
                if ((avatarY > 20) && (holdTime < 12))
                {
                    holdTime += 1;
                }

                //numbers for jetpack output (bullets or fire or whatever):
                if (exhaustAnimate < 7)
                {
                    ++exhaustAnimate;
                } else if (exhaustAnimate > 7) {
                    exhaustAnimate = 0;
                }

            //when falling:
            } else {
                //subtract from holdtime, added to avatarY (it can be a negative number):
                if ((avatarY < (185)) && (holdTime > -10))
                {
                    holdTime -= 1;
                }

                //increases numbers for jetpack powering down animation:
                if (exhaustAnimate < 7)
                {
                    exhaustAnimate = 8;
                }
                else if (exhaustAnimate < 11)
                {
                    ++exhaustAnimate;
                }
                else
                {
                    exhaustAnimate = 18;
                }
            }

            //SAX got pretty mad about this part, turns out a linear equation can model a curve and
            //is actually better than a cubic function:
            avatarY -= (holdTime);

            //sees if Y-value rolled past 20 or 186, figures out if it was going up
            //or down, and auto-corrects accordingly (FUTURE ME; IT'S OPTIMIZED ENOUGH PLEASE DON'T WASTE ANY MORE TIME):
            if ((avatarY > (186)) || (avatarY < 20))
            {
                if (holdTime > 0)
                {
                    avatarY = 20;
                } else {
                    avatarY = 185;
                }
                holdTime = 0;
            }
        }

        //bit that runs avatar animations:
        if (avatarY < 185)
        {
            displacement = 9;
        }
        else
        {

            if (displacement == 9)
            {
                displacement = 3;
            }

            if ((displacement < 1) || (displacement > 7))
            {
                avatarAnimate *= -1;
            }

            displacement += avatarAnimate;
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
        gfx_TransparentSprite_NoClip(avatar_tiles[displacement / 3], avatarX, avatarY - abs((displacement / 3) - 1));

        //bit that draws exhaust when in flight:
        if (exhaustAnimate < 18)
        {
            gfx_TransparentSprite_NoClip(exhaust_tiles_optimized[exhaustAnimate / 2], avatarX + randInt(1, 3), avatarY + 31);
            gfx_TransparentSprite_NoClip(nozzle, avatarX + 4, avatarY + 31);
        }

        //bit that runs coin collision and movement:
        for (uint8_t i = 0; i < coin_max[coinFormation]; ++i)
        {
            //do things if the coin is less than the origin plus some buffer I made up:
            if (coinX[i] <= COIN_ORIGIN + 500)
            {
                //collision detection and appropriate sprite drawing:
                if (gfx_CheckRectangleHotspot(avatarX + 6, avatarY, 18, 40, coinX[i] - 11, coinY[i] + 1, 10, 10))
                {
                    gfx_TransparentSprite(sparkle, coinX[i] - 13, coinY[i] - 1);

                    coinX[i] = 1020;
                    ++monies;
                }
                else
                {

                    gfx_TransparentSprite(coin_tiles_optimized[(coinAnimate[i] / 10)], (coinX[i] - 12), coinY[i]);

                    if ((coinAnimate[i] < 38) && (coinX[i] < COIN_ORIGIN))
                    {
                        coinAnimate[i] += 2;
                    }
                    else
                    {
                        coinAnimate[i] = 0;
                    }
                }

                coinX[i] -= scrollSpeed;
            }
        }

        //bit that calculates zappers 'n stuff:
        for (uint8_t i = 0; i < MaxZappers; ++i)
        {
            //drawing the zapper beams:
            if (zapperX[i] <= ZAPPER_ORIGIN)
            {
                for (uint8_t i_the_sequel = 0; i_the_sequel < zapperLength[i]; ++i_the_sequel)
                {
                    gfx_TransparentSprite(beam, zapperX[i] - 14, zapperY[i] + 16 + (i_the_sequel * 10));
                }

                //draw zapper pairs and beams with distance of zapperLength between them:
                if (zapperX[i] < 336)
                {
                    randVar = randInt(0, 1);
                    randVar1 = randInt(0, 1);

                    gfx_TransparentSprite(electric_animation[randVar + 2 + (randVar1 * 4)], zapperX[i] - 25, zapperY[i] - 7);
                    gfx_TransparentSprite(electric_animation[randVar], zapperX[i] - 25, zapperY[i] + 7 + (zapperLength[i] * 10));

                    gfx_TransparentSprite(zapper_tiles_flipped[zapperAnimate[i] / 10], zapperX[i] - 18, zapperY[i]);
                    gfx_TransparentSprite(zapper_tiles[zapperAnimate[i] / 10], zapperX[i] - 18, zapperY[i] + 14 + (zapperLength[i] * 10));

                    if (zapperAnimate[i] < 28)
                    {
                        zapperAnimate[i] += 2;
                    }
                    else
                    {
                        zapperAnimate[i] = 0;
                    }

                    //collision for zappers:
                    if ((health > 0) && (gfx_CheckRectangleHotspot(avatarX + 6, zapperY[i] + 2, 18, (zapperLength[i] * 10) + 30, zapperX[i] - 14, avatarY, 10, 50)))
                    {
                        --health;
                    }
                }

                zapperX[i] -= scrollSpeed;
            }
        }

        //bit that draws and calculates the missiles 'o death:
        for (uint8_t i = 0; i < MaxMissiles; ++i)
        {
            if (missileX[i] <= MISSILE_ORIGIN)
            {
                if (missileX[i] < 366)
                {
                    gfx_TransparentSprite(missile_tiles_optimized[missileAnimate / 2], missileX[i] - 46, missileY[i] - 18);

                    if ((health > 0) && (gfx_CheckRectangleHotspot(missileX[i] - 45, avatarY, 19, 40, avatarX + 6, missileY[i] - 5, 18, 12)))
                    {
                        --health;
                    }

                    //I'm using the X-position as timer, meaning there's less warning as time goes on:
                }
                else if (missileX[i] < 641)
                {
                    //AW CRAP HERE COME DAT BOI!
                    gfx_TransparentSprite(missileIncoming_tiles[missile_icon_animate / 3], 281 + randInt(-1, 1), missileY[i] - 16 + randInt(-1, 1));
                }
                else if (missileX[i] < MISSILE_ORIGIN)
                {
                    //plenty of time to dodge (at the beginning at least)
                    gfx_TransparentSprite(missileWarning_tiles[missile_icon_animate / 2], 293, missileY[i] - 11);

                    //tracking on avatar
                    if (missileY[i] < (avatarY + 20))
                    {
                        missileY[i] += 2;
                    }
                    else if (missileY[i] > (avatarY + 21))
                    {
                        missileY[i] -= 2;
                    }
                }

                //missiles travel by 6 pixels each frame. It was surprisingly tedious to figure that out
                //from frame-by-frame reviewing of missiles; however, it's too slow on the calc.
                missileX[i] -= scrollSpeed + 8;
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
                    gfx_TransparentSprite(powering_tile_0, laserX - 19, laserY[i]);
                    gfx_TransparentSprite(powering_tile_0, 320 - laserX, laserY[i]);
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
                        if (laserLifetime[i] < 1)
                        {
                            ++deadLasers;
                        }
                        else if (laserLifetime[i] < 9)
                        {

                            //lasers running out of juice:
                            gfx_TransparentSprite(shutdown_tiles[laserAnimate / 3], 5, laserY[i] - 16);
                            gfx_TransparentSprite(shutdown_tiles_flipped[laserAnimate / 3], 285, laserY[i] - 16);

                            gfx_RLETSprite(laser_tiles[(laserAnimate / 3) + 1], 35, laserY[i]);
                        }
                        else if (laserLifetime[i] < 59)
                        {
                            //firing lasers:
                            gfx_TransparentSprite(firing_tiles[laserAnimate / 3], 5, laserY[i] - 11);
                            gfx_TransparentSprite(firing_tiles_flipped[laserAnimate / 3], 285, laserY[i] - 11);

                            gfx_RLETSprite(laser_tile_0, 35, laserY[i]);

                            //hitbox for damage. 5 pixel leeway above and below:
                            if ((health > 0) && Yspot(avatarY, 35, laserY[i], 10))
                            {
                                --health;
                            }

                            //makes sure the animations play correctly, I don't have time to fix math today:
                            if (laserLifetime[i] == 9)
                                laserAnimate = 0;
                        }
                        else if (laserLifetime[i] < 109)
                        {

                            //the simplest drawing code in this hot mess, for powering up:
                            gfx_SetColor(5);

                            gfx_Line(20, laserY[i] + 7, 300, laserY[i] + 7);

                            gfx_Circle(11, laserY[i] + 7, 7 + ((laserLifetime[i] - 50) / 4));
                            //gfx_Circle(11, laserY[i]+7, 6 + ((laserLifetime[i]-50)/4));

                            gfx_Circle(308, laserY[i] + 7, 7 + ((laserLifetime[i] - 50) / 4));
                            //gfx_Circle(308, laserY[i]+7, 8 + ((laserLifetime[i]-50)/4));
                        }

                        //update the laser's timer:
                        --laserLifetime[i];
                    }
                }

                if (laserLifetime[i] < 109)
                {
                    gfx_TransparentSprite(powering_tiles[(laserAnimate / 3) + 1], laserX - 19, laserY[i]);
                    gfx_TransparentSprite(powering_tiles_flipped[(laserAnimate / 3) + 1], 320 - laserX, laserY[i]);
                }
                else
                {
                    gfx_TransparentSprite(powering_tile_0, laserX - 19, laserY[i]);
                    gfx_TransparentSprite(powering_tiles_flipped[0], 320 - laserX, laserY[i]);
                }
            }
        }


   
        //gold coin color for money counter:
        gfx_SetTextFGColor(4);

        gfx_SetTextScale(1, 1);
        gfx_SetTextXY(10, 30);
        gfx_PrintInt(monies, 4);

        //distance and FPS counter are gray:
        gfx_SetTextFGColor(3);

        gfx_SetTextXY(280, 10);
        gfx_PrintInt(frameTime, 1);

        gfx_SetTextScale(2, 2);
        gfx_SetTextXY(10, 10);
        //these numbers are magic and just give good results
        gfx_PrintInt((distance += scrollSpeed) / 15, 4);
        

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
                break;
                //goto main menu
            }
            else if (menuSelect == 1)
            {
                goto GAMESTART;
            }
            //if menuSelect is 2 or anything else it just resumes; everything resets when the pause menu is accessed again.
        }

        //timer reset and "DA WORLDO" is over:
        timer_1_Counter = 0;

    } while (!(kb_Data[6] & kb_Clear) && (health > 0));

    if (health < 1)
    {
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

    //stop libraries, not doing so causes "interesting" results by messing up the color mode and scaling:
    gfx_End();
}

//CONGRATULATIONS, YOU FOUND THE END OF THE FILE! I HOPE PEEKING BEHIND THE CURTAIN WAS WORTH IT!
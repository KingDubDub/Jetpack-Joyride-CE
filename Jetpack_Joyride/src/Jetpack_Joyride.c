/*Jetpack Joyride CE

Jetpack Joyride for the TI-84 Plus CE calculators.

Made by King Dub Dub

I'm pretty sure you have the readme if you have this source code, but if you want
to mod this or something then get ready for some over-commented trash.

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

#include "sprites/gfx.h"
#include "coinShapes.c"

//max number of zappers:
#define MaxZappers 3

//mas number of missiles, will probably be replaced with a non-static value later:
#define MaxMissiles 1

//the most overused variable ever:
uint8_t i;

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name
//my child if I had the desire to marry and have children.
uint16_t avatarX;
uint8_t avatarY;

//avatar's sprite array and values for keeping track of animation frames:
gfx_sprite_t *avatar[4];
int8_t avatarAnimate = 1;
int8_t displacement = 3;

//exhaust/flame sprite array for jetpack flight animations:
gfx_sprite_t *exhaust[6];

//variable used for calculating fire animations:
uint8_t flightTime = 18;

//time it takes to complete the game loop, used to control the FPS; if it overflows
//then we have real problems:
uint8_t frameTime;

//variables for when jetpack is on or not for math calculations:
int8_t holdTime;

//speed of scrolling for avatar, obstacles, map, etc.
uint8_t scrollSpeed;

//measures timings for delays between spawning coins, obstacles, etc.:
int24_t spawnDelay = 100;

//used for a bad background scroll function that is actually the best for this scenario:
int24_t backgroundScroll;

//stores if Barry done got wasted or not:
uint8_t health;

//max monies at $4,294,967,295:
uint32_t monies;

//there's a limit to the distance you can fly in the original game, not sure I'll keep that or not...
uint32_t distance;

//for randomization values that need to be reused:
uint8_t randVar;
uint8_t randVar1;

uint8_t randObject;

//arrays for zapper coordinates:
uint24_t zapperY[MaxZappers];
uint16_t zapperX[MaxZappers];
//measured in beam units, 10x10 pixels:
uint8_t zapperLength[MaxZappers];
//used to draw zapper beam segments:
uint8_t beamCount;
//zapper animation count, all start at zero:
uint8_t zapperAnimate[MaxZappers];
gfx_sprite_t *zapper[6];

//sprite array for electrical flares around zapper nodes:
gfx_sprite_t *electric[8];

uint24_t coinX[MaxCoins];
uint8_t coinY[MaxCoins];
//coin formation variable to keep track of coin lists:
uint8_t coinFormation;
//values for keeping track of coin animation sprites, all start at zero:
uint8_t coinAnimate[MaxCoins];
gfx_sprite_t *coin[4];

uint16_t missileX[MaxMissiles];
uint8_t missileY[MaxMissiles];
//missile warning sprite array (the red exclamation marks):
gfx_sprite_t *warning[3];
//missile incoming sprite array (exclamation marks with emphasis):
gfx_sprite_t *incoming[2];
//keep track of animations for missiles:
uint8_t missileAnimate;
int8_t MAvalue = -1;



//clears all objects from gameplay:
void delObjects()
{
    for (i = 0; i < abbreviatedMax[coinFormation]; ++i)
    {
        coinX[i] = 2000;
        coinY[i] = 0;
    }

    for (i = 0; i < MaxZappers; ++i)
    {
        zapperLength[i] = 0;
        zapperX[i] = 2000;
        zapperY[i] = 0;
    }

    for (i = 0; i < MaxMissiles; ++i)
    {
        missileX[i] = 6001;
        missileY[i] = 0;
    }
}



void main(void)
{
    //make background sprite variables and a quick decompression slot:
    gfx_sprite_t *background, *nozzle, *sparkle, *beam, *missile, *decompressorVar;

    background = gfx_MallocSprite(192, 240);
    zx7_Decompress(background, background_compressed);

    //sparkle effect left by picked up coins:
    sparkle = gfx_MallocSprite(16, 15);
    zx7_Decompress(sparkle, sparkle_compressed);

    //jetpack nozzle that glows when releasing exhaust:
    nozzle = gfx_MallocSprite(nozzle_width, nozzle_height);
    zx7_Decompress(nozzle, nozzle_compressed);

    //zapper beam:
    beam = gfx_MallocSprite(10, 10);
    zx7_Decompress(beam, beam_compressed);

    //temporary missile sprite:
    missile = gfx_MallocSprite(46, 36);
    zx7_Decompress(missile, missile_compressed);

    for(i = 0; i < 4; ++i)
    {
        //decompressing avatar spritesheet:
        decompressorVar = gfx_MallocSprite(30, 40);
        zx7_Decompress(decompressorVar, avatarSheet_tiles_compressed[i]);
        avatar[i] = decompressorVar;
    }

    //jetpack exhaust:
    for(i = 0; i < 6; ++i)
    {
        decompressorVar = gfx_MallocSprite(11, 23);
        zx7_Decompress(decompressorVar, exhaustSheet_tiles_compressed[i]);
        exhaust[i] = decompressorVar;
    }

    for(i = 0; i < 4; ++i)
    {
        //decompressing coin spritesheet:
        decompressorVar = gfx_MallocSprite(12, 12);
        zx7_Decompress(decompressorVar, coinSheet_tiles_compressed[i]);
        coin[i] = decompressorVar;
    }

    for(i = 0; i < 3; ++i)
    {
        //zappers:
        zapper[i] = gfx_MallocSprite(18, 18);
        zapper[i+3] = gfx_MallocSprite(18, 18);
        zx7_Decompress(zapper[i], zapperSheet_tiles_compressed[i]);
        gfx_FlipSpriteX(zapper[i], zapper[i+3]);
    }

    for(i=0; i < 8; ++i)
    {
        //mallocing full electric array:
        electric[i] = gfx_MallocSprite(32, 32);
    }

    for(i = 0; i < 2; ++i)
    {
        //zapper lightning:
        zx7_Decompress(electric[i], electricSheet_tiles_compressed[i]);
        gfx_FlipSpriteX(electric[i], electric[i+2]);
        gfx_FlipSpriteY(electric[i], electric[i+4]);
        gfx_RotateSpriteHalf(electric[i], electric[i+6]);
    }

    //DON'T TOUCH THE MISSILE BITS, SOMETHING COMPILES FUNKY HERE AND I DON'T KNOW WHAT.
    for(i = 0; i < 2; ++i)
    {
        //missile incoming symbol:
        decompressorVar = gfx_MallocSprite(31, 31);
        zx7_Decompress(decompressorVar, missileIncoming_tiles_compressed[i]);
        incoming[i] = decompressorVar;
    }

    for(i = 0; i < 3; ++i)
    {
        //missile warning symbols:
        decompressorVar = gfx_MallocSprite(20, 21);
        zx7_Decompress(decompressorVar, missileWarning_tiles_compressed[i]);
        warning[i] = decompressorVar;
    }

    //initialize GFX libraries:
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);

    //start up a timer for FPS monitoring, do not move:
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    //best scan mode according to Mateo:
    kb_SetMode(MODE_3_CONTINUOUS);

    //all text printed is gray:
    gfx_SetTextFGColor(2);

    //when I first started using C, I asked some friends if there were GOTO statements.
    //They proved they were good friends, and told me "No, that's stupid". I'm glad they lied.
    GAMESTART:
    //But it's still sometimes okay.

    //reset variables for when a game starts:
    scrollSpeed = 6;
    avatarX = 24;
    avatarY = 185;
    holdTime = 0;
    flightTime = 18;
    health = 1;
    spawnDelay = 100;
    distance = 0;
    monies = 0;

    delObjects();

    // Loop until clear is pressed:
    do{
        //update keys, fixes bugs with update errors that can lead to softlocks:
        kb_Scan();

        if ((backgroundScroll - scrollSpeed) <= 0)
        {
            backgroundScroll += (192 - scrollSpeed);
        } else {
            backgroundScroll -= scrollSpeed;
        }

        //spawns stuff, SO much better than the debug methods I originally used:
        if (spawnDelay <= 0)
        {
            randObject = randInt(1,10);

            if (randObject == 1)
            {
                //sets coin coordinates from coordinate lists:
                randVar = randInt(30, 150);
                coinFormation = randInt(0, 5);
                for(i = 0; i < MaxCoins; ++i)
                {
                    coinX[i] = ctx[coinFormation][i] + 330;
                    coinY[i] = cty[coinFormation][i] + randVar;
                    coinAnimate[i] = 0;
                }

                spawnDelay = 500;

            } else if (randObject == 2) {

                //spawns missiles (and missile swarms if I implement them):
                for (i = 0; i < MaxMissiles; ++i)
                {
                    if (missileX[i] > 6000)
                    {
                        missileX[i] = 1466;
                        missileY[i] = 10 * randInt(2, 18);

                        i = MaxZappers;
                    }
                }

                //missiles have their own "delay" and don't need to reset spawnDelay.

            } else if (randObject <= 10) {

                //randomly generates zapper coordinates and lengths:
                for (i = 0; i < MaxZappers; ++i)
                {
                    if (zapperX[i] > 330)
                    {
                        zapperX[i] = 330;

                        zapperLength[i] = randInt(2, 4);
                        zapperY[i] = 10 * randInt(2, 19 - zapperLength[i]);

                        i = MaxZappers;
                    }
                }

                spawnDelay = 200;
            }
        } else {
            spawnDelay -= scrollSpeed;
        }

        //run controls until Barry gets wasted, then bounces his corpse around:
        if (health > 0)
        {
            if (kb_Data[1] & kb_2nd)
            {
                if((avatarY > 20) && (holdTime < 12))
                {
                    holdTime += 1;
                }

                if (flightTime > 7)
                {
                    flightTime = 0;
                }

                if (flightTime < 7)
                {
                    ++flightTime;
                }

            } else {

                if((avatarY < (185)) && (holdTime > -10))
                {
                    holdTime -= 1;
                }

                if (flightTime < 7)
                {
                    flightTime = 8;

                } else if (flightTime < 11) {

                    ++flightTime;

                } else {

                    flightTime = 18;
                }
            }

            //SAX got pretty mad about this part, turns out a linear equation can model a curve and
            //is actually better than a cubic function:
            avatarY -= (holdTime);

            //sees if Y-value rolled past zero or 198, figures out if it was going up
            //or down, and auto-corrects accordingly:
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
        } else {

            if (displacement == 9)
            {
                displacement = 3;
            }

            if((displacement < 1) || (displacement > 7))
            {
                avatarAnimate *= -1;
            }

            displacement += avatarAnimate;
        }

        //this is the best way I've found to draw the backgrounds, smaller and faster than a smart system:
        gfx_Sprite(background, backgroundScroll - 192, 0);
        gfx_Sprite(background, backgroundScroll, 0);
        gfx_Sprite(background, backgroundScroll + 192, 0);

        //bit that runs coin collision and movement:
        for(i = 0; i < abbreviatedMax[coinFormation]; ++i)
        {
            if (coinX[i] < 1000)
            {
                //collision detection and appropriate sprite drawing:
                if (coinX[i] < 330)
                {
                    if (gfx_CheckRectangleHotspot(avatarX+6,avatarY,18,40,coinX[i]-11,coinY[i]+1,10,10))
                    {
                        gfx_TransparentSprite(sparkle, coinX[i]-13, coinY[i]-1);

                        coinX[i] = 1020;
                        ++monies;
                    } else {
                        gfx_TransparentSprite(coin[(coinAnimate[i]/10)], (coinX[i]-12), coinY[i]);

                        if (coinAnimate[i] < 38)
                        {
                            coinAnimate[i] += 2;
                        } else {
                            coinAnimate[i] = 0;
                        }
                    }
                }

                coinX[i] -= scrollSpeed;
            }
        }

        //bit that calculates zappers 'n stuff:
        for (i = 0; i < MaxZappers; ++i)
        {
            //drawing the zapper beams:
            if (zapperX[i] < 1000)
            {
                for (beamCount = 0; beamCount < zapperLength[i]; ++beamCount)
                {
                    gfx_TransparentSprite(beam, zapperX[i]-14, zapperY[i]+16+(beamCount*10));
                }

                //draw zapper pairs and beams with distance of zapperLength between them:
                if (zapperX[i] < 336)
                {
                    randVar = randInt(0,1);
                    randVar1 = randInt(0,1);

                    gfx_TransparentSprite(electric[randVar+2+(randVar1*4)], zapperX[i]-25, zapperY[i]-7);
                    gfx_TransparentSprite(electric[randVar+(randVar1*4)], zapperX[i]-25,  zapperY[i]+7+(zapperLength[i]*10));

                    gfx_TransparentSprite(zapper[(zapperAnimate[i]/10)+3], zapperX[i]-18, zapperY[i]);
                    gfx_TransparentSprite(zapper[zapperAnimate[i]/10], zapperX[i]-18, zapperY[i]+14+(zapperLength[i]*10));

                    if (zapperAnimate[i] < 28)
                    {
                        zapperAnimate[i] += 2;
                    } else {
                        zapperAnimate[i] = 0;
                    }

                    //collision for zappers:
                    if (gfx_CheckRectangleHotspot(avatarX+6, zapperY[i]+2, 18, (zapperLength[i]*10)+30, zapperX[i]-14, avatarY, 10, 50))
                    {
                        --health;
                    }
                }

                zapperX[i] -= scrollSpeed;
            }
        }

        //bit that draws and calculates the missiles 'o death:
        for (i = 0; i < MaxMissiles; ++i)
        {
            if (missileX[i] < 6001)
            {
                if (missileX[i] < 366)
                {
                    gfx_TransparentSprite(missile, missileX[i]-46, missileY[i]-18);

                    if (gfx_CheckRectangleHotspot(missileX[i]-45, avatarY, 19, 40, avatarX+6, missileY[i]-5, 18, 12))
                    {
                        --health;
                    }

                } else if (missileX[i] < 641) {

                    //AW CRAP HERE COME DAT BOI!
                    gfx_TransparentSprite(incoming[missileAnimate/3], 281+randInt(-1,1), missileY[i]-16+randInt(-1,1));

                } else if (missileX[i] < 1467) {

                    //plenty of time to dodge (at the beginning at least)
                    gfx_TransparentSprite(warning[missileAnimate/2], 293, missileY[i]-11);

                    //tracking on avatar
                    if (missileY[i] < (avatarY + 20))
                    {
                        missileY[i] += 2;
                    } else if (missileY[i] > (avatarY + 21)) {
                        missileY[i] -= 2;
                    }
                }

                //make missileAnimate go up and down between 0 and 5 by toggling MAvalue:
                if ((missileAnimate < 1) || (missileAnimate > 4))
                {
                    MAvalue *= -1;
                }
                missileAnimate += MAvalue;

                //missiles travel by 6 pixels each frame. It was surprisingly tedious to figure that out
                //from frame-by-frame reviewing of missiles.
                missileX[i] -= scrollSpeed + 8;
            }
        }

        //draws the avatar after a few hundred lines of code:
        gfx_TransparentSprite_NoClip(avatar[displacement/3], avatarX, avatarY-abs((displacement/3)-1));

        //bit that draws exhaust when in flight:
        if (flightTime < 18)
        {
            gfx_TransparentSprite_NoClip(exhaust[flightTime/2], avatarX+randInt(1,3), avatarY+31);
            gfx_TransparentSprite_NoClip(nozzle, avatarX+4, avatarY+31);
        }

        //FPS counter:
        frameTime = (32768 / timer_1_Counter);

        if (frameTime > 25)
        {
            delay(40 - (1000/frameTime));
        }

        timer_1_Counter = 0;

        gfx_SetTextScale(1, 1);
        gfx_SetTextXY(280, 10);
        gfx_PrintInt(frameTime, 2);

        gfx_SetTextXY(10, 30);
        gfx_PrintInt(monies, 4);

        gfx_SetTextScale(2,2);
        gfx_SetTextXY(10, 10);
        gfx_PrintInt((distance += scrollSpeed)/15, 4);

        //gfx_BlitBuffer();
        gfx_SwapDraw();

    } while (!(kb_Data[6] & kb_Clear) && (health > 0));

    if (health < 1)
    {
        gfx_FillScreen(1);
        gfx_SetTextScale(4, 4);
        gfx_PrintStringXY("U is ded lol.", 15, 100);
        gfx_SwapDraw();

        while (kb_AnyKey());
        while (!kb_AnyKey());

        if (kb_Data[6] != kb_Clear)
        {
            goto GAMESTART;
        }
    }

    //erase the decompressed sprites, very important:
    free(background);
    free(nozzle);
    free(sparkle);
    free(beam);
    free(missile);
    free(decompressorVar);

    //stop libraries, not doing so causes "interesting" results
    gfx_End();
}

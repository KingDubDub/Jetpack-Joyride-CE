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
#include "coinShapes.h"

//maximum coins that can spawn, 27 is the maximum safe value currently:
#define extern MaxCoins 27

//max number of zappers:
#define MaxZappers 1

//buffer for copying and rotating zappers:
gfx_UninitedSprite(zappBuffer, 18, 18);
gfx_UninitedSprite(electricBuffer, 32, 32);

//the most overused variable ever:
uint8_t i;

//just got a remake:
uint8_t i_the_sequal;

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name
//my child if I had the desire to marry and have children.
uint16_t avatarX;
uint8_t avatarY;

//avatar's sprite array and values for keeping track of animation frames:
gfx_sprite_t *avatar[4];
int8_t avatarAnimate = 1;
int8_t displacement = 3;

//exhaust/flame sprite array for jetpack flight animations:
gfx_sprite_t *exhaust[4];

//variable used for calculating fire animations:
uint8_t flightTime;

//time it takes to complete the game loop, used to control the FPS; if it overflows
//then we have real problems:
uint8_t frameTime;

//variables for when jetpack is on or not for math calculations:
int8_t holdTime;

//speed of scrolling for avatar, obstacles, map, etc.
uint8_t scrollSpeed = 5;

//used for a bad background scroll function that is actually the best for this scenario:
int24_t backgroundScroll;

//stores if Barry done got wasted or not:
uint8_t health;

//max monies at $4,294,967,295:
uint32_t monies;

uint32_t distance;

//for randomization values that need to be reused:
uint8_t randVar;
uint8_t randVar1;

//arrays for zapper coordinates:
uint24_t zapperY[MaxZappers];
uint16_t zapperX[MaxZappers];
//measured in beam units, 10x10 pixels:
uint8_t zapperLength[MaxZappers];
//zapper animation count, all start at zero:
uint8_t zapperAnimate[MaxZappers];
gfx_sprite_t *zapper[3];

//sprite array for electrical flares around zapper nodes:
gfx_sprite_t *electric[2];

uint24_t coinX[MaxCoins];
uint8_t coinY[MaxCoins];
//values for keeping track of coin animation sprites, all start at zero:
uint8_t coinAnimate[MaxCoins];
gfx_sprite_t *coin[4];

//custom rectangle function: (x and y coords, size x, size y, and color)
//used for quick-'n-dirty drawing and debug purposes:
void rect(int16_t x, uint8_t y, int16_t x2, uint8_t y2, uint8_t color)
{
    gfx_SetColor(color);
    gfx_FillRectangle(x, y, x2, y2);
}

//sets coin coordinates from coordinate lists:
void spawnCoin()
{
    randVar = randInt(30, 160);
    randVar1 = randInt(0, 4);

    for(i = 0; i < MaxCoins; ++i)
    {
        coinX[i] = ctx[randVar1][i] + 330;
        coinY[i] = cty[randVar1][i] + randVar;
        coinAnimate[i] = 0;
    }
}

//randomly generates zapper coordinates:
void spawnZapper()
{
    randVar = randInt(2, 5);
    randVar1 = randInt(0, 200 - (randVar*10));

    for(i = 0; i < MaxZappers; ++i)
    {
        zapperX[i] = 330;
        zapperY[i] = randVar1;
        zapperLength[i] = randVar;
    }
}

void main(void)
{
    //make background sprite variables and a quick decompression slot:
    gfx_sprite_t *background, *nozzle, *beam, *decompressorVar;

    background = gfx_MallocSprite(192, 240);
    zx7_Decompress(background, background_compressed);

    //jetpack nozzle that glows when releasing exhaust:
    nozzle = gfx_MallocSprite(nozzle_width, nozzle_height);
    zx7_Decompress(nozzle, nozzle_compressed);

    //zapper beam:
    beam = gfx_MallocSprite(10, 10);
    zx7_Decompress(beam, beam_compressed);

    //decompressing avatar spritesheet:
    for(i = 0; i < 4; ++i)
    {
        decompressorVar = gfx_MallocSprite(30, 40);
        zx7_Decompress(decompressorVar, avatarSheet_tiles_compressed[i]);
        avatar[i] = decompressorVar;
    }

    //jetpack exhaust:
    for(i = 0; i < 3; ++i)
    {
        decompressorVar = gfx_MallocSprite(11, 20);
        zx7_Decompress(decompressorVar, exhaustSheet_tiles_compressed[i]);
        exhaust[i] = decompressorVar;
    }

    //decompressing coin spritesheet:
    for(i = 0; i < 4; ++i)
    {
        decompressorVar = gfx_MallocSprite(10, 10);
        zx7_Decompress(decompressorVar, coinSheet_tiles_compressed[i]);
        coin[i] = decompressorVar;
    }

    //zappers:
    for(i = 0; i < 3; ++i)
    {
        decompressorVar = gfx_MallocSprite(18, 18);
        zx7_Decompress(decompressorVar, zapperSheet_tiles_compressed[i]);
        zapper[i] = decompressorVar;
    }

    //zapper lightning:
    for(i = 0; i < 2; ++i)
    {
        decompressorVar = gfx_MallocSprite(32, 32);
        zx7_Decompress(decompressorVar, electricSheet_tiles_compressed[i]);
        electric[i] = decompressorVar;
    }

    //initialize GFX libraries:
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);

    //in case colors don't initialize correctly, use black for debugging:
    gfx_SetColor(1);

    //start up a timer for FPS monitoring, do not move:
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    //all text printed is gray:
    gfx_SetTextFGColor(2);

    GAMESTART:
    //I just couldn't resist.

    //reset variables for when a game starts:
    avatarX = 24;
    avatarY = 185;
    health = 1;
    holdTime, distance, monies = 0;

    spawnCoin();
    spawnZapper();

    // Loop until clear is pressed:
    do{
        //update keys, not actually necessary and causes lag but fixes bugs with update errors:
        kb_Scan();

        //this is the best way I've found to draw the backgrounds, smaller and faster than a smart system:
        gfx_Sprite(background, backgroundScroll - 192, 0);
        gfx_Sprite(background, backgroundScroll, 0);
        gfx_Sprite(background, backgroundScroll + 192, 0);

        if ((backgroundScroll - scrollSpeed) <= 0)
        {
            backgroundScroll += (192 - scrollSpeed);
        } else {
            backgroundScroll -= scrollSpeed;
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

                if (flightTime < 11)
                {
                    ++flightTime;
                }

            } else {

                if((avatarY < (185)) && (holdTime > -10))
                {
                    holdTime -= 1;
                }

                flightTime = 0;
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

        //make some coins with no regard for if they exist or not:
        if (randInt(0,200) == 0){spawnCoin();}

        //bit that runs coin collision and movement:
        for(i = 0; i < MaxCoins; ++i)
        {
            if (coinX[i] < 1000)
            {
                //collision detection:
                if (gfx_CheckRectangleHotspot(avatarX,avatarY,24,40,coinX[i]-9,coinY[i]+1,8,8))
                {
                    coinX[i] = 1020;
                    ++monies;
                }

                //draw coin when on screen and update animations:
                if (coinX[i] < 330) {

                    gfx_TransparentSprite(coin[(coinAnimate[i]/10)], (coinX[i]-10), coinY[i]);

                    if (coinAnimate[i] < 38)
                    {
                        coinAnimate[i] += 2;
                    } else {
                        coinAnimate[i] = 0;
                    }
                }

                coinX[i] -= scrollSpeed;
            }
        }

        //make a zapper just like we make coins, very VERY badly:
        if(randInt(0,100) == 0){spawnZapper();}

        //bit that calculates obstacles and zappers and stuff:
        for (i=0; i < MaxZappers; ++i)
        {
            //drawing the zapper beams:
            if (zapperX[i] < 1000)
            {
                for (i_the_sequal=0;i_the_sequal < zapperLength[i];++i_the_sequal)
                {
                    gfx_TransparentSprite(beam, zapperX[i]-14, zapperY[i]+16+(i_the_sequal*10));
                }

                //draw zapper pairs with distance of zapperLength between them:
                if (zapperX[i] < 336)
                {
                    randVar = randInt(0,1);

                    if (randInt(0,1) == 1)
                    {
                        gfx_TransparentSprite(gfx_FlipSpriteX(electric[randVar], electricBuffer), zapperX[i]-25, zapperY[i]-7);
                        gfx_TransparentSprite(gfx_FlipSpriteY(electric[randVar], electricBuffer), zapperX[i]-25,  zapperY[i]+7+(zapperLength[i]*10));
                    } else {
                        gfx_TransparentSprite(gfx_RotateSpriteHalf(electric[randVar], electricBuffer), zapperX[i]-25, zapperY[i]-7);
                        gfx_TransparentSprite(electric[randVar], zapperX[i]-25,  zapperY[i]+7+(zapperLength[i]*10));
                    }

                    gfx_TransparentSprite(gfx_FlipSpriteX(zapper[zapperAnimate[i]/10], zappBuffer), zapperX[i]-18, zapperY[i]);
                    gfx_TransparentSprite(zapper[zapperAnimate[i]/10], zapperX[i]-18, zapperY[i]+14+(zapperLength[i]*10));

                    if (zapperAnimate[i] < 28)
                    {
                        zapperAnimate[i] += 2;
                    } else {
                        zapperAnimate[i] = 0;
                    }
                }

                zapperX[i] -= scrollSpeed;

                //collision between to zappers:
                if (gfx_CheckRectangleHotspot(avatarX, zapperY[i]+2, 30, (zapperLength[i]*10)+30, zapperX[i]-14, avatarY, 10, 50))
                {
                    --health;
                }
            }
        }

        //draws the avatar after a few hundred lines of code:
        gfx_TransparentSprite_NoClip(avatar[displacement/3], avatarX, avatarY-abs((displacement/3)-1));

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

        //bit that draws exhaust when in flight:
        if (kb_Data[1] & kb_2nd)
        {
            gfx_TransparentSprite_NoClip(exhaust[flightTime/4], avatarX+randInt(1,3), avatarY+31);
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
        rect(0,0,320,240,1);
        gfx_SetTextScale(4, 4);
        gfx_PrintStringXY("U is ded lol.", 15, 100);
        gfx_SwapDraw();

        while (os_GetCSC());
        while (!os_GetCSC());

        if (kb_Data[6] != kb_Clear)
        {
            goto GAMESTART;
        }
    }

    //erase the decompressed sprites, very important:
    free(background);
    free(nozzle);
    free(beam);
    free(decompressorVar);

    //stop libraries, not doing so causes "interesting" results
    gfx_End();
}

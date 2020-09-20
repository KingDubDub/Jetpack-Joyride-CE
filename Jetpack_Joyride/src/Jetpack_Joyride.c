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

// Key variable:
kb_key_t key;

//buffer for copying and rotating zappers:
gfx_UninitedSprite(rotationBuffer, 18, 18);

//the most overused variable ever:
uint8_t i;

//just got a remake:
uint8_t i_the_sequal;

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name
//my child if I had the desire to marry and have children.
uint16_t BarryX;

uint8_t BarryY;

//time it takes to complete the game loop, used to control the FPS; if it overflows
//then we have real problems:
uint8_t frameTime;

//variables for when jetpack is on or not for math calculations:
int8_t holdTime;

//speed of scrolling for avatar, obstacles, map, etc.
uint8_t scrollSpeed = 5;

//used for a bad background scroll function that is actually the best for this scenario:
int16_t backgroundScroll;

//stores if Barry done got wasted or not:
uint8_t health;

//max monies at $4,294,967,295:
uint32_t monies;

//2D arrays for zapper objects, first is the downwards facing one, second is the upwards one:
uint8_t zapperY[MaxZappers] =
{
    70
};

uint16_t zapperX[MaxZappers] =
{
    320
};

//measured in beam units, 10x10 pixels:
uint8_t zapperLength[MaxZappers] =
{
    3
};

gfx_sprite_t *zapper[3];

uint24_t coinX[MaxCoins];

uint8_t coinY[MaxCoins];

//values for keeping track of coin animation sprites, all start at zero:
uint8_t coinAnimate[MaxCoins];
gfx_sprite_t *coin[4];

uint8_t LEGGanimate;
gfx_sprite_t *LEGGS[2];

//custom rectangle function: (x and y coords, size x, size y, and color)
//used for quick-'n-dirty drawing and debug purposes:
void rect(int16_t x, uint8_t y, int16_t x2, uint8_t y2, uint8_t color)
{
    gfx_SetColor(color);
    gfx_FillRectangle(x, y, x2, y2);
}


bool pointCollision(int16_t x1, uint8_t y1, int16_t x2, uint8_t y2, uint16_t px, uint8_t py)
{
    //this looks odd, but I feel like this runs faster, plus it puts emphasis on testing the Y first:
    if(px < x2)
    {
        if ((x1 < px) && (y1 < py) && (py < y2))
        {
            return true;
        }
    }
    return false;
}

//for randomization values that need to be reused:
uint8_t randVar;
uint8_t randVar1;

//sets coin coordinates from coordinate lists:
void spawnCoin()
{
    randVar = randInt(30, 160);
    randVar1 = randInt(0, 2);

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
    gfx_sprite_t *barry, *background, *beam, *decompressorVar;

    barry = gfx_MallocSprite(barry_width, barry_height);
    zx7_Decompress(barry, barry_compressed);

    background = gfx_MallocSprite(192, 240);
    zx7_Decompress(background, background_compressed);

    //zapper beam:
    beam = gfx_MallocSprite(10, 10);
    zx7_Decompress(beam, beam_compressed);

    //decompressing MAH BOI'S LEGGS:
    for(i = 0; i < 2; ++i)
    {
        decompressorVar = gfx_MallocSprite(28, 8);
        zx7_Decompress(decompressorVar, LEGGS_tiles_compressed[i]);
        LEGGS[i] = decompressorVar;
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

    //initialize GFX libraries:
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);

    //in case colors don't initialize correctly, use black for debugging:
    gfx_SetColor(1);

    //start up a timer for FPS monitoring, do not move:
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    GAMESTART:
    //I just couldn't resist.

    //reset variables for when a game starts:
    BarryX = 26;
    BarryY = 185;
    holdTime = 0;
    health = 1;
    monies = 0;

    spawnZapper();

    // Loop until clear is pressed:
    do{

        //this is the best way I've found to draw the backgrounds, smaller and faster than a smart system:
        gfx_Sprite(background, backgroundScroll - 192, 0);
        gfx_Sprite(background, backgroundScroll, 0);
        gfx_Sprite(background, backgroundScroll + 192, 0);

        if ((backgroundScroll - scrollSpeed) <= 0)
        {
            backgroundScroll = (192 - scrollSpeed);
        } else {
            backgroundScroll -= scrollSpeed;
        }


        //run controls until Barry gets wasted, then bounces his corpse around:
        if (health > 0)
        {
            if (kb_Data[1] & kb_2nd)
            {
                if((BarryY > 5) && (holdTime < 12))
                {
                    holdTime += 1;
                }
            } else {
                if((BarryY < (185)) && (holdTime > -10))
                {
                    holdTime -= 1;
                }
            }

            //SAX got pretty mad about this part, turns out a linear equation can model a curve and
            //is actually better than a cubic function:
            BarryY -= (holdTime);

            //sees if Y-value rolled past zero or 198, figures out if it was going up
            //or down, and auto-corrects accordingly:
            if ((BarryY > (186)) || (BarryY < 5))
            {
                if (holdTime > 0)
                {
                    BarryY = 5;
                } else {
                    BarryY = 185;
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
                if (pointCollision(BarryX-5,BarryY-5,BarryX+barry_width+5,BarryY+45,coinX[i]+5,coinY[i]+5))
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

        //make a zapper just like we make coins, very very badly:
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

                //draw zapper pairs:
                if (zapperX[i] < 336)
                {
                    gfx_TransparentSprite(gfx_FlipSpriteX(zapper[0], rotationBuffer), zapperX[i]-18, zapperY[i]);
                    gfx_TransparentSprite(zapper[0], zapperX[i]-18, zapperY[i]+14+(zapperLength[i]*10));
                }

                zapperX[i] -= scrollSpeed;

                //collision between to zappers, will change to one hitbox in the future:
                if (pointCollision(zapperX[i]-14-barry_width, zapperY[i]+2-barry_height, zapperX[i]-4, zapperY[i]+30+(zapperLength[i]*10), BarryX, BarryY))
                {
                    --health;
                }
            }
        }

        gfx_TransparentSprite_NoClip(barry, BarryX, BarryY);


        gfx_TransparentSprite_NoClip(LEGGS[LEGGanimate/10], BarryX, BarryY+32);

        if (LEGGanimate < 16)
        {
            LEGGanimate += 4;
        } else {
            LEGGanimate = 0;
        }

        if (BarryY < 185)
        {
            LEGGanimate = 16;
        }

        //FPS counter:
        frameTime = (32768 / timer_1_Counter);

        if (frameTime > 25)
        {
            delay(40-(1000/frameTime));
        }

        timer_1_Counter = 0;

        //prints the FPS without the lock (FPS will still be ~25)
        gfx_SetTextXY(280, 10);
        gfx_PrintInt(frameTime, 2);

        gfx_SetTextXY(10, 10);
        gfx_PrintInt(monies, 4);

        gfx_BlitBuffer();

    } while (!(kb_Data[6] & kb_Clear) && (health > 0));

    if (health < 1)
    {
        rect(0,0,320,240,1);
        gfx_SetTextScale(4, 4);
        gfx_PrintStringXY("U is ded lol.", 15, 100);
        gfx_SetTextScale(1, 1);
        gfx_BlitBuffer();

        while (os_GetCSC());
        while (!os_GetCSC());

        if (kb_Data[6] != kb_Clear)
        {
            goto GAMESTART;
        }
    }

    //erase the decompressed sprites, very important:
    free(background);
    free(beam);
    free(barry);
    free(decompressorVar);

    //stop libraries, not doing so causes "interesting" results
    gfx_End();
}

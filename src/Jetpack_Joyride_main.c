/*Jetpack Joyride CE

A Jetpack Joyride port for the TI-84 Plus CE calculators.

Made by King Dub Dub

I'm pretty sure you have the readme if you have this source code, but if you want
to mod this or something then get ready for some over-commented trash!

In case it wasn't clear, modding this should have my permission and credit to me,
but other than that you are obliged to have as much fun as will kill you!

*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <tice.h>
#include <keypadc.h>
#include <graphx.h>
#include <compression.h>
#include <fileioc.h>

//my little headers thingy that's full of data I neither touch much nor count towards total lines:
#include "headers.h"

//appvar version since I decided to add versions, it's my code and I'll do what I want:
const uint8_t APPVAR_VERSION = 4;

//HEY FUTURE ME, REMEMBER TO UPDATE THE VERSION WHEN WE ADD STUFF!
//SCREW YOU PAST-FUTURE ME, hopefully future-future me does better.

//we read APPVAR_VERSION to this var for testing later:
uint8_t saveIntegrity;

//speed of scrolling and time before incrementing it:
int8_t scrollSpeed;
uint16_t incrementDelay;

//measures timings for delays between spawning coins, obstacles, etc.:
int24_t spawnDelay;
int24_t missileDelay;

//used for a bad background scroll function that is actually the best for this scenario:
int24_t bg_scroll;

//arrays for storing background tileset pointer values to draw:
uint8_t bg_list[9];
uint8_t secondary_bg_list[9];

//all important game data, and in one clean struct pointer too:
game_data_t save_data;

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name my child if I had the desire to marry
//and have children.
avatar_t avatar;

//everything needed to keep track of the points and speed of the seperate jetpack when Barry dies:
jetpack_t jetpackEntity;

//and here's some obstacles and stuff:
coin_t coins;
zapper_t zappers;
missile_t missiles;
laser_t lasers;

//a simple framecount delay for the opening screen scrolling scene:
uint8_t opening_delay = 138;

//gfx_CheckRectangleHotspot but only the Y checks, since 4 C++ masters write better code than me:
// #define Yspot(master_y, master_height, test_y, test_height);

//Mmm smells like stolen code... adapted from StackOverflow, fills an array of a given size with a given value very quickly:
// #define Fill_Array(array, entries, value)

//takes an input sprite and pastes it into another sprite at given coordinates:
void CopyPasta(const gfx_sprite_t *spriteIn, gfx_sprite_t *spriteOut, uint24_t x, uint8_t y);

//a function for drawing buttons, will hopefully save on flash size and stack usage:
void draw_button(gfx_sprite_t *sprites[], char *text, uint8_t buttonSelect);

//resets the backgrounds starting from a given tile, no error checks or handling:
void Set_Background(const uint8_t start);

//saves the game data to the JPJRDAT appvar, creates one if it doesn't exist:
void save_state(void);

//a simple function based off of flawed but efficient code for getting sprite data out of appvars with one tileset:
void* GetTile_Ptr(void *ptr, uint8_t tile);

//Almost the exact same function as before but for pulling lists of single sprites out of an appvar without tilesets:
void* GetSprite_Ptr(void *ptr, uint8_t tile);

//Draws the title screen menu and all that cool stuff:
void TitleMenu(gfx_sprite_t *ceiling[], gfx_sprite_t *background[], gfx_sprite_t *floor[], gfx_sprite_t *menusprite);

//stuff for the death screen:
void ded_menu(gfx_sprite_t *menusprite)
{
    //make sure that whatever's on the screen is the same as what's in the buffer:
    gfx_BlitScreen();

    //temporary flipped menu sprite with hardcoded numbers to annoy people:
    gfx_sprite_t *flipped_menusprite = gfx_MallocSprite(8, 167);

    gfx_TransparentSprite_NoClip(menusprite, 160, 10);

    gfx_CopyRectangle(gfx_buffer, gfx_buffer, 167, 10, 168, 10, 132, 167);

    //this hearkens back to the early days of this project when I thought this actually worked and looked good...
    //I know know it only kinda works, and it looks cramped.
    gfx_TransparentSprite_NoClip(gfx_FlipSpriteY(menusprite, flipped_menusprite), 300, 10);

    gfx_SetTextFGColor(2); //white
    gfx_SetTextScale(1, 1);

    gfx_PrintStringXY("YOU FLEW", 200, 40);
    gfx_PrintStringXY("AND COLLECTED", 190, 100);
    gfx_PrintStringXY("COINS:", 170, 140);

    gfx_SetTextFGColor(4); //gold
    gfx_SetTextScale(3, 3);

    //draw coin count:
    gfx_SetTextXY(210, 130);
    gfx_PrintUInt(save_data.monies, 1);

    //draw distance:
    gfx_SetTextXY(190, 60);
    gfx_PrintUInt(save_data.distance / 15, 1);

    //add the meters symbol:
    gfx_SetTextScale(2, 2);
    gfx_PrintStringXY("m", gfx_GetTextX(), 68);

    gfx_SwapDraw();

    while (kb_AnyKey()) kb_Scan();
    while (!(kb_Data[1] & kb_2nd) && !(kb_Data[6] & kb_Clear)) kb_Scan();

    free(flipped_menusprite);

    //reset distance to flag that a game is no longer in progress:
    save_data.distance = 0;
}



//I'm perfectly aware that the main() is too long, go away.
int main(void)
{
    //set my crappy port of the Jetpackia font as the current font:
    gfx_SetFontData(jetpackia);
    gfx_SetFontSpacing(jetpackia_spacing);

    //a slot variable to hold any appvar locations I need since Mateo yeeted ti_CloseAll() out of the toolchain:
    ti_var_t tmp_slot;

    void *palette_ptr = ti_GetDataPtr(tmp_slot = ti_Open("JPJRPAL", "r"));
    ti_Close(tmp_slot);

    void *start_1_ptr = ti_GetDataPtr(tmp_slot = ti_Open("JPJRBG1", "r"));
    ti_Close(tmp_slot);

    void *start_2_ptr = ti_GetDataPtr(tmp_slot = ti_Open("JPJRBG2", "r"));
    ti_Close(tmp_slot);

    void *background_ptr = ti_GetDataPtr(tmp_slot = ti_Open("JPJRBG3", "r"));
    ti_Close(tmp_slot);

    void *background_extras_ptr = ti_GetDataPtr(tmp_slot = ti_Open("JPJRBG4", "r"));
    ti_Close(tmp_slot);

    void *menu_ptr = ti_GetDataPtr(tmp_slot = ti_Open("JPJRGFX1", "r"));
    ti_Close(tmp_slot);

    void *obstacles_ptr = ti_GetDataPtr(tmp_slot = ti_Open("JPJRGFX2", "r"));
    ti_Close(tmp_slot);

    void *avatar_ptr = ti_GetDataPtr(tmp_slot = ti_Open("BARRY", "r"));
    ti_Close(tmp_slot);

    //make sure that the appvar checks didn't return NULL:
    if( (!palette_ptr)           ||
        (!start_1_ptr)           ||
        (!start_2_ptr)           ||
        (!background_ptr)        ||
        (!background_extras_ptr) ||
        (!menu_ptr)              ||
        (!obstacles_ptr)         ||
        (!avatar_ptr)
    ) return 1;

    //Wow I hate how that looks.

    //pointers to the background sprites:
    gfx_sprite_t *ceiling_tiles[14];
    gfx_sprite_t *background_tiles[18];
    gfx_sprite_t *floor_tiles[14];

    //the start menu is also a part of the actual hallway, which is just me being lazy really:
    for(uint8_t i = 0; i < 8; ++i)
    {
        //we add one to the second input because all these appvar pointers go to single tilesets:
        background_tiles[i] = GetTile_Ptr(start_1_ptr, i + 1);
    }

    //set the pointers for the opening background where we bust through the wall:
    for(uint8_t i = 0; i < 4; ++i)
    {
        background_tiles[8 + i] = GetTile_Ptr(start_2_ptr, i + 1);
    }

    //move the background sprites directly into the RAM:
    for(uint8_t i = 0; i < 6; ++i)
    {
        background_tiles[12 + i] = gfx_MallocSprite(46, 160);
        zx7_Decompress(background_tiles[12 + i], GetTile_Ptr(background_ptr, i + 1));
    }

    //set the pointers for the ceiling sprites, some are reused with different backgrounds:
    for(uint8_t i = 0; i < 14; ++i)
    {
        ceiling_tiles[i] = GetTile_Ptr(background_extras_ptr, i + 1);
    }

    //and get the floor bits too:
    for(uint8_t i = 14; i < 28; ++i)
    {
        floor_tiles[i - 14] = GetTile_Ptr(background_extras_ptr, i + 1);
    }

    //used to save which offset in the obstacles appvar's LUT entries we need to use:
    uint8_t offsets_offset = 0;

    //sprites for menus and stuff:
    gfx_sprite_t *button_on_tiles[4];
    gfx_sprite_t *button_off_tiles[4];
    gfx_sprite_t *window;

    //now we get the sprites for the menus, here's the unpressed pause buttons:
    for(uint8_t i = 0; i < (sizeof(button_on_tiles) / 3); ++i)
    {
        //we leave the tiles parameter as-is since these sprites aren't stored as tilesets (which is awful and I will change that):
        button_on_tiles[i] = GetSprite_Ptr(menu_ptr, offsets_offset++);
    }

    //and the pressed buttons:
    for(uint8_t i = 0; i < (sizeof(button_off_tiles) / 3); ++i)
    {
        button_off_tiles[i] = GetSprite_Ptr(menu_ptr, offsets_offset++);
    }

    window = GetSprite_Ptr(menu_ptr, offsets_offset);

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
        coin_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //lookit the zapper beam being all weird and stuff:
    beam[0] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);

    //get the ding-dang zapper sprite pointers:
    for(uint8_t i = 0; i < (sizeof(zapper_tiles) / 3); ++i)
    {
        zapper_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //find the missile warning and incoming icon pointers:
    for(uint8_t i = 0; i < (sizeof(missileWarning_tiles) / 3); ++i)
    {
        missileWarning_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }
    for(uint8_t i = 0; i < (sizeof(missileIncoming_tiles) / 3); ++i)
    {
        missileIncoming_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //calculate pointers for missile sprites:
    for(uint8_t i = 0; i < (sizeof(missile_tiles) / 3); ++i)
    {
        missile_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //the laser node itself, but only the left side, we create flipped copies later:
    for(uint8_t i = 0; i < (sizeof(powering_tiles) / 3); ++i)
    {
        powering_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //pew pew pew
    for(uint8_t i = 0; i < (sizeof(firing_tiles) / 3); ++i)
    {
        firing_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //when the lasers power down:
    for(uint8_t i = 0; i < (sizeof(shutdown_tiles) / 3); ++i)
    {
        shutdown_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(laser_tiles) / 3); ++i)
    {
        laser_tiles[i] = GetSprite_Ptr(obstacles_ptr, offsets_offset++);
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
    jetpack = GetSprite_Ptr(avatar_ptr, offsets_offset++);

    nozzle = GetSprite_Ptr(avatar_ptr, offsets_offset++);

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(barry_run_tiles) / 3); ++i)
    {
        barry_run_tiles[i] = GetSprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(barry_hit_tiles) / 3); ++i)
    {
        barry_hit_tiles[i] = GetSprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(barry_ded_tiles) / 3); ++i)
    {
        barry_ded_tiles[i] = GetSprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image:
    for(uint8_t i = 0; i < (sizeof(exhaust_tiles) / 3); ++i)
    {
        exhaust_tiles[i] = GetSprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //and here's all the flipped sprites we'll put in the RAM:
    gfx_sprite_t *zapper_tiles_flipped[4];
    gfx_sprite_t *horizontal_zapper[4];
    gfx_sprite_t *horizontal_zapper_flipped[4];
    gfx_sprite_t *powering_tiles_flipped[4];
    gfx_sprite_t *firing_tiles_flipped[3];
    gfx_sprite_t *barryHit_resized;
    gfx_sprite_t *barryHit_rotated;
    gfx_sprite_t *jetpack_resized;
    gfx_sprite_t *jetpack_rotated;

    //flipped zapper sprites:
    for(uint8_t i = 0; i < 4; ++i)
    {
        zapper_tiles_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_FlipSpriteX(zapper_tiles[3 - i], zapper_tiles_flipped[i]);
    }

    //horizontal zapper sprites:
    for(uint8_t i = 0; i < 4; ++i)
    {
        horizontal_zapper[i] = gfx_MallocSprite(32, 32);
        horizontal_zapper_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_RotateSpriteC(zapper_tiles[i], horizontal_zapper[i]);
        gfx_RotateSpriteC(zapper_tiles_flipped[i], horizontal_zapper_flipped[i]);
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

    //best scan mode according to the angry lettuce man:
    kb_SetMode(MODE_3_CONTINUOUS);

    //initialize GFX libraries:
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(palette_ptr, 512, 0);
    gfx_SetTransparentColor(0);

    //read from appvar if it exists, fails if it returns 0
    ti_var_t savegame = ti_Open(DATA_APPVAR, "r");
    
    if(!savegame) save_state();

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

    //load all the appvar data as our "starting point" if distance isn't 0 and resume gameplay:
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

        ti_Read(&opening_delay, sizeof(opening_delay), 1, savegame);

        ti_Close(savegame);
    }
    else //make a fresh start and show the game menu:
    {
        //reset variables for when a game starts:
        scrollSpeed    = 6;
        bg_scroll      = 0;
        incrementDelay = 0;
        spawnDelay     = 512;
        
        save_data.distance = 0;
        save_data.health   = 1;
        save_data.monies   = 0;

        avatar.x                     = 24;
        avatar.y                     = 185;
        avatar.theta                 = 0;
        avatar.inputDuration         = 0;
        avatar.playerAnimationToggle = 1;
        avatar.playerAnimation       = 3;
        avatar.exhaustAnimation      = 18;
        avatar.corpseBounce          = 0;
        avatar.deathDelay            = 0;

        //if there's a bug, it's always because the animation values are funky:
        missiles.iconAnimate     = 0;
        missiles.animationToggle = -1;

        lasers.deactivated = MaxLasers;

        jetpackEntity.theta   = 0;
        jetpackEntity.bounce  = 0;
        jetpackEntity.h_accel = 2;

        //clear all objects from gameplay by moving their X-coords out of bounds:
        for(uint8_t i = 0; i < coin_max[coins.formation]; ++i) coins.x[i] = 2000;

        for(uint8_t i = 0; i < MAX_ZAPPERS; ++i) zappers.x[i] = ZAPPER_ORIGIN;

        for(uint8_t i = 0; i < MAX_MISSILES; ++i) missiles.x[i] = (MISSILE_ORIGIN + 10);

        lasers.x = 0;

        //set the backgrounds up for the opening scene:
        Set_Background(0);

        //if the opening delay isn't zero, AKA none, then we do the scrolling intro:
        if(opening_delay)
        {
            //quickly (lazily) draw the opening hallway tiles:
            for(uint8_t i = 0; i < 7; ++i)
            {
                gfx_Sprite(ceiling_tiles[secondary_bg_list[i]], i * 46, 0);
                gfx_Sprite(background_tiles[bg_list[i]],        i * 46, 40);
                gfx_Sprite(floor_tiles[secondary_bg_list[i]],   i * 46, 200);
            }

            //blit the fresh background:
            gfx_BlitBuffer();

            //opening menu function to make this mess more readable:
            TitleMenu(ceiling_tiles, background_tiles, floor_tiles, window);

            while(!(kb_Data[6] & kb_Clear) && opening_delay)
            {
                kb_Scan();

                //ceil() doesn't work with ints so here's some numbers I made up and some funky math:
                uint8_t deincrement = opening_delay/12 + (opening_delay % 12 != 0);

                if((opening_delay - deincrement) > 0)
                {
                    opening_delay -= deincrement;

                    if((bg_scroll + deincrement) < 46)
                    {
                        bg_scroll += deincrement;
                    }
                    else
                    {
                        bg_scroll = 0;

                        //a neat way to shift the opening tiles since they appear in order:
                        for(uint8_t i = 0; i < sizeof(bg_list); ++i)
                        {
                            ++bg_list[i];
                            ++secondary_bg_list[i];
                        }
                    }
                }
                else
                {
                    opening_delay = 0;
                    bg_scroll = 0;

                    break;
                }

                //what is this magical witchcraft and why didn't I use it earlier?!
                gfx_ShiftLeft(deincrement);

                gfx_Sprite(ceiling_tiles[secondary_bg_list[7]], (7 * 46) - bg_scroll, 0);
                gfx_Sprite(background_tiles[bg_list[7]],        (7 * 46) - bg_scroll, 40);
                gfx_Sprite(floor_tiles[secondary_bg_list[7]],   (7 * 46) - bg_scroll, 200);

                gfx_BlitBuffer();
            }
        }
        //make absolutely sure all the background tiles are set correctly:
        Set_Background(3);
    }

    //color to flash when hit by obstacles, it's a really cool effect and I'm surprised I thought of a way to do it:
    uint8_t deathColor;

    //time it takes to complete the game loop, used to control the FPS; if it overflows
    //then we have real speed problems:
    uint8_t FPS;

    //start up a timer for FPS monitoring, do not move:
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    //Loop until clear is pressed or Barry has been dead for a little while:
    while(!(kb_Data[6] & kb_Clear) && (avatar.deathDelay != 50))
    {
        //update keys, fixes bugs with update errors that can lead to softlocks:
        kb_Scan();

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

            if(!randObject && (missileDelay <= 0) && save_data.health)
            {
                //picks a random formation from the data arrays in the functions.c file:
                lasers.formation = randInt(0, (sizeof(laserMax) - 1));

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
                //after finishing the spawn checks, make sure missiles don't spawn when Barry is dead, I don't like it when they
                //spawn in and get frozen on screen.
                if(save_data.health)
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
        else if(lasers.x > 0)
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
                    gfx_TransparentSprite(zapper_tiles_flipped[zappers.animate/3], zappers.x[i] - 25, zappers.y[i] - 7);
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
                        else if(lasers.lifetime[i] <= 8)
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
                        else if(lasers.lifetime[i] < 64)
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
                            if(lasers.lifetime[i] == 9) lasers.animation = 0;
                        }
                        else if(lasers.lifetime[i] <= 120)
                        {
                            //the simplest drawing code in this hot mess, for powering up:
                            gfx_SetColor(5);

                            gfx_Line_NoClip(20, lasers.y[i] + 7, 300, lasers.y[i] + 7);

                            gfx_Circle_NoClip(11, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle_NoClip(11, lasers.y[i]+7, 6 + ((lasers.lifetime[i]-50)/4));

                            gfx_Circle_NoClip(308, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle_NoClip(308, lasers.y[i]+7, 8 + ((lasers.lifetime[i]-50)/4));
                        }

                        //I thought it was an issue with how I deactivated lasers, but it was really an int24 overflow:
                        if(lasers.lifetime[i] != 16777215) --lasers.lifetime[i];
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

        //draw the score 'n stuff unless we're on the last frame of Barry's death delay:
        if(avatar.deathDelay != 50)
        {
            //distance and FPS counter are grey:
            gfx_SetTextFGColor(2);

            gfx_SetTextScale(2, 2);
            gfx_SetTextXY(10, 10);
            //I pulled that number out of my butt and it gives good results, distance in pixels divided by 15:
            gfx_PrintInt(save_data.distance / 15, 4);

            gfx_SetTextScale(1, 1);

            //print a little meters symbol after the distance:
            gfx_PrintStringXY("M", gfx_GetTextX(), 18);

            gfx_PrintStringXY("BEST:", 10, 29); gfx_PrintInt(save_data.highscore, 1);

            gfx_SetTextXY(280, 10);
            gfx_PrintInt(FPS, 1);

            /*for(uint8_t i = 0; i < 7; ++i)
            {
                gfx_SetTextXY(260, 30 + (10 * i));
                gfx_PrintUInt(lasers.lifetime[i], 1);
            }*/

            //gold coin color for money counter:
            gfx_SetTextFGColor(4);

            gfx_SetTextXY(10, 41);
            gfx_PrintInt(save_data.monies, 3);
        }

        //speedy blitting, but works best when used a lot of cycles BEFORE any new drawing functions:
        gfx_SwapDraw();

        //afterwards, we check if Barry was hit, and if so then the screen will quickly flash a certain color:
        if(deathColor)
        {
            gfx_FillScreen(deathColor);
            
            //blit the color:
            gfx_SwapDraw();

            //we also set some variables that need to be updated once at time of death:
            jetpackEntity.y = avatar.y + 7;

            jetpackEntity.x = avatar.x;
            jetpackEntity.v_accel = avatar.inputDuration + 2;

            for(uint8_t i = 0; i < sizeof(lasers.y); ++i)
            {
                //either overflow the int24 instantly or put it in the last animation frames:
                if(lasers.lifetime[i] > 120)
                {
                    if(lasers.lifetime[i] < 16777215)
                    {
                        lasers.lifetime[i] = 16777215;
                        ++lasers.deactivated;
                    }
                }
                else if(lasers.lifetime[i] > 8)
                {
                    lasers.lifetime[i] = 8;
                }
            }

            lasers.animation = 0;
            
            delay(50);

            //reset the flag, comment this out if you need to off an epileptic cheese-themed supervillian:
            deathColor = 0;
        }

        //FPS counter data collection, time "stops" (being relevant) after this point:
        FPS = (32768 / timer_GetSafe(1, TIMER_UP));
        //the GetSafe() is to make Tari stop bothering me

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

            //strings used in pause menu:
            char pauseOptions[3][7] = {"QUIT", "RETRY", "RESUME"};

            //drawing the base color for all the menu stuff:
            gfx_FillScreen(6);

            //using that draw_button() function to write out the 3 menu options:
            draw_button(button_on_tiles, pauseOptions[0], 0);
            draw_button(button_on_tiles, pauseOptions[1], 1);
            draw_button(button_on_tiles, pauseOptions[2], 2);

            gfx_SetColor(2);

            //draw initial selector position:
            gfx_Rectangle(69, 32, 182, 52);

            //used by menuing functions to do "stuff:"
            debounced = false;
            int8_t menuSelect = 0;

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
                    draw_button(button_on_tiles, pauseOptions[menuSelect], menuSelect);

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
                    gfx_PrintStringXY(pauseOptions[menuSelect], 160 - gfx_GetStringWidth(pauseOptions[menuSelect])/2, 47 + (menuSelect * 60));
                }
                else
                {
                    if(debounced)
                    {
                        break;
                    }
                }

                gfx_BlitBuffer();

                //simple way to wait until none of the arrow keys are pressed:
                while(((kb_Data[7] & kb_Down) || (kb_Data[7] & kb_Up)) && !(kb_Data[6] & kb_Clear)) kb_Scan();
            }

            //imagine completing menuing checks when [clear] is pressed and leaving it in the code for 4 updates.
            if(!(kb_Data[6] & kb_Clear))
            {
                //quitting sets the opening delay to 138 along with some other values:
                if(!menuSelect)
                {
                    //set that menu delay variable to a positive value that I'll use for some funky math:
                    opening_delay = 138;

                    bg_scroll = 0;

                    //set this to flag that the game is over:
                    save_data.distance = 0;
                }
                else if(menuSelect == 1) //reset the game without the opening bit:
                {
                    opening_delay      = 0;
                    bg_scroll          = 0;
                    save_data.distance = 0;
                }
                //if menuSelect is 2 or anything else it just resumes; everything resets when the pause menu is accessed again.

                goto GAMESTART;
            }
        }

        //and with a timer reset "ZA WARUDO" is over (iS tHaT A jOJo rEfERenCe and also yes I looked it up):
        timer_1_Counter = 0;

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

            //if the last entry has been "dragged" through the next 2 list entries, make new sprite addresses;
            //note that I only check the background tiles since the timings are the same for all tile insertions:
            if(bg_list[7] == bg_list[8])
            {
                if(bg_list[8] < 11)
                {
                    //this just makes sure that we've done the opening 12 tiles before we start spawning the 16:
                    ++bg_list[8];
                    ++secondary_bg_list[8];
                }
                else if(bg_list[6] == bg_list[8])
                {
                    bg_list[7] = 12 + randInt(0, 2) * 2;
                    bg_list[8] = bg_list[7] + 1;

                    secondary_bg_list[7] =  12;
                    secondary_bg_list[8] = 13;
                }
            }
            //the problem is that all the tiles are only half of a single hallway "chunk" of 92 pixels.
        }
        else
        {
            bg_scroll += scrollSpeed;
        }
    }

    //if we didn't quit out of the game and Barry is clearly dead, pull up the death screen:
    if(!(kb_Data[6] & kb_Clear) && (save_data.health < 1))
    {
        //do all the graphical stuff and menuing until some decision is reached:
        ded_menu(window);

        //I still can't believe I used a goto label...
        goto GAMESTART;
    }
    
    //we only need distance and a few other vars to be saved, but I made this function AND I'M GONNA USE THE WHOLE FUNCTION.
    save_state();

    //stop libraries, not doing so causes "interesting" results by messing up the color mode and scaling:
    gfx_End();

    return 1;
}

//CONGRATION, YOU FOUND THE END OF THE FILE! I HOPE PEEKING BEHIND THE CURTAIN WAS WORTH YOUR SANITY!
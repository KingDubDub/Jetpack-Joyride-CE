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

#include "headers.h"


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
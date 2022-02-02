/*Jetpack Joyride CE

A Jetpack Joyride port for the TI-84 Plus CE calculators.

Made by King Dub Dub

I'm pretty sure you have the readme if you have this source code, but if you want
to mod this or something then get ready for some over-commented trash!

In case it wasn't clear, modding this should have my permission and credit to me,
but other than that you are obliged to have as much fun as will kill you!

NOTES TO SELF
    - Sprites are scaled down by 1:0.720720720
    - All in-game text must be capitalized since the font doesn't include lowercase
    - These chars are not implemented either: "`", "{", "|", "}", and "~"

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

//my little headers thingy that's full of data I neither touch much nor count towards total lines
#include "headers.h"



//I'm perfectly aware that the main() is too long, go away.
int main(void)
{
    //set my crappy port of the Jetpackia font as the current font
    gfx_SetFontData(jetpackia);
    gfx_SetFontSpacing(jetpackia_spacing);

    const void *palette_ptr = Get_Appvar_Ptr("JPJRPAL");

    const void *start_1_ptr = Get_Appvar_Ptr("JPJRBG1");

    const void *start_2_ptr = Get_Appvar_Ptr("JPJRBG2");

    const void *background_ptr = Get_Appvar_Ptr("JPJRBG3");

    const void *background_extras_ptr = Get_Appvar_Ptr("JPJRBG4");

    const void *menu_ptr = Get_Appvar_Ptr("JPJRGFX1");

    const void *obstacles_ptr = Get_Appvar_Ptr("JPJRGFX2");

    const void *avatar_ptr = Get_Appvar_Ptr("BARRY");

    //make sure that the appvar checks didn't return NULL
    if( (!palette_ptr)           ||
        (!start_1_ptr)           ||
        (!start_2_ptr)           ||
        (!background_ptr)        ||
        (!background_extras_ptr) ||
        (!menu_ptr)              ||
        (!obstacles_ptr)         ||
        (!avatar_ptr)            ) //Wow, I hate how this looks.
    {
        gfx_End();
        
        os_PutStrLine("Missing graphics appvars!");
        delay(2000);

        return 0;
    }

    //the start menu is also a part of the actual hallway, which is just me being lazy really
    for(uint8_t i = 0; i < 5; ++i)
    {
        //we add one to the second input because all these appvar pointers go to single tilesets
        background_tiles[i] = Get_Tile_Ptr(start_1_ptr, i + 1);
    }

    //set the pointers for the opening background where we bust through the wall
    for(uint8_t i = 0; i < 4; ++i)
    {
        background_tiles[5 + i] = Get_Tile_Ptr(start_2_ptr, i + 1);
    }

    //allocate precious RAM for the decompressed tile output
    for(uint8_t i = 0; i < 4; ++i)
    {
        background_tiles[9 + i] = gfx_MallocSprite(46, 160);
    }

    //decompress the background sprites directly into the RAM
    for(uint8_t i = 0; i < 4; ++i)
    {
        zx7_Decompress(background_tiles[9 + i], Get_Tile_Ptr(background_ptr, i + 1));
    }

    //oh boy now everything's out of order, but at least the code doesn't need to be completely overhauled
    background_tiles[13] = Get_Tile_Ptr(start_2_ptr, 5);
    //still might do it for peace of mind though, but I'll need to weigh the optimization benefits for sure...

    //set the pointers for the ceiling sprites, some are reused with different backgrounds
    for(uint8_t i = 0; i < 14; ++i)
    {
        ceiling_tiles[i] = Get_Tile_Ptr(background_extras_ptr, i + 1);
    }

    //and get the floor bits too
    for(uint8_t i = 14; i < 29; ++i)
    {
        floor_tiles[i - 14] = Get_Tile_Ptr(background_extras_ptr, i + 1);
    }

    //used to save which offset in the obstacles appvar's LUT entries we need to use
    uint8_t offsets_offset = 0;

    //now we get the sprites for the menus, here's the unpressed pause buttons
    for(uint8_t i = 0; i < (sizeof(button_on_tiles) / 3); ++i)
    {
        //we leave the tiles parameter as-is since these sprites aren't stored as tilesets (which is awful and I will change that)
        button_on_tiles[i] = Get_Sprite_Ptr(menu_ptr, offsets_offset++);
    }

    //and the pressed buttons
    for(uint8_t i = 0; i < (sizeof(button_off_tiles) / 3); ++i)
    {
        button_off_tiles[i] = Get_Sprite_Ptr(menu_ptr, offsets_offset++);
    }

    window = Get_Sprite_Ptr(menu_ptr, offsets_offset++);
    title  = Get_Sprite_Ptr(menu_ptr, offsets_offset++);

    offsets_offset = 0;

    //pointers to pickup and obstacle sprites 'n stuff, in the order that they're arranged in the appvar
    gfx_rletsprite_t *coin_tiles[5];
    gfx_sprite_t     *beam[2];
    gfx_sprite_t     *zapper_tiles[4];
    gfx_rletsprite_t *missile_warning_tiles[3];
    gfx_rletsprite_t *missile_incoming_tiles[2];
    gfx_rletsprite_t *missile_tiles[7];
    gfx_sprite_t     *powering_tiles[4];
    gfx_rletsprite_t *firing_tiles[3];
    gfx_rletsprite_t *shutdown_tiles[3];
    gfx_sprite_t     *laser_tiles[4];

    //locate coin sprite pointers
    for(uint8_t i = 0; i < (sizeof(coin_tiles) / 3); ++i)
    {
        coin_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //lookit the zapper beam being all weird and stuff
    beam[0] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);

    //get the ding-dang zapper sprite pointers
    for(uint8_t i = 0; i < (sizeof(zapper_tiles) / 3); ++i)
    {
        zapper_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //find the missile warning and incoming icon pointers
    for(uint8_t i = 0; i < (sizeof(missile_warning_tiles) / 3); ++i)
    {
        missile_warning_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }
    for(uint8_t i = 0; i < (sizeof(missile_incoming_tiles) / 3); ++i)
    {
        missile_incoming_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //calculate pointers for missile sprites
    for(uint8_t i = 0; i < (sizeof(missile_tiles) / 3); ++i)
    {
        missile_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //the laser node itself, but only the left side, we create flipped copies later
    for(uint8_t i = 0; i < (sizeof(powering_tiles) / 3); ++i)
    {
        powering_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //pew pew pew!
    for(uint8_t i = 0; i < (sizeof(firing_tiles) / 3); ++i)
    {
        firing_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //when the lasers power down
    for(uint8_t i = 0; i < (sizeof(shutdown_tiles) / 3); ++i)
    {
        shutdown_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image
    for(uint8_t i = 0; i < (sizeof(laser_tiles) / 3); ++i)
    {
        laser_tiles[i] = Get_Sprite_Ptr(obstacles_ptr, offsets_offset++);
    }

    //we start reading from a different appvar after this
    offsets_offset = 0;

    gfx_sprite_t     *jetpack;
    gfx_sprite_t     *nozzle;
    gfx_rletsprite_t *barry_run_tiles[4];
    gfx_sprite_t     *barry_hit_tiles[3];
    gfx_sprite_t     *barry_ded_tiles[3];
    gfx_rletsprite_t *exhaust_tiles[6];

    //Oh my gosh, a post-increment was actually useful for once. Huh.
    jetpack = Get_Sprite_Ptr(avatar_ptr, offsets_offset++);

    nozzle = Get_Sprite_Ptr(avatar_ptr, offsets_offset++);

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image
    for(uint8_t i = 0; i < (sizeof(barry_run_tiles) / 3); ++i)
    {
        barry_run_tiles[i] = Get_Sprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image
    for(uint8_t i = 0; i < (sizeof(barry_hit_tiles) / 3); ++i)
    {
        barry_hit_tiles[i] = Get_Sprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image
    for(uint8_t i = 0; i < (sizeof(barry_ded_tiles) / 3); ++i)
    {
        barry_ded_tiles[i] = Get_Sprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //the laser beam sprite is used as a color index for drawing the beams, it's only 1/250 of the total image
    for(uint8_t i = 0; i < (sizeof(exhaust_tiles) / 3); ++i)
    {
        exhaust_tiles[i] = Get_Sprite_Ptr(avatar_ptr, offsets_offset++);
    }

    //and here's all the flipped sprites we'll put in the RAM
    gfx_sprite_t     *zapper_tiles_flipped[4];
    gfx_sprite_t     *horizontal_zapper[4];
    gfx_sprite_t     *horizontal_zapper_flipped[4];
    gfx_sprite_t     *powering_tiles_flipped[4];
    gfx_rletsprite_t *firing_tiles_flipped[3];
    gfx_rletsprite_t *shutdown_tiles_flipped[3];

    //make space for the buffers, they need to be 36 pixels but the rotation functions tend to round up 1 pixel
    gfx_sprite_t *barryHit_resized = gfx_MallocSprite(37, 37);
    gfx_sprite_t *barryHit_rotated = gfx_MallocSprite(37, 37);

    //the jetpack sprite needs buffers as well, square 24x24 with extra pixels
    gfx_sprite_t *jetpack_resized = gfx_MallocSprite(25, 25);
    gfx_sprite_t *jetpack_rotated = gfx_MallocSprite(25, 25);

    //flipped zapper sprites
    for(uint8_t i = 0; i < 4; ++i)
    {
        zapper_tiles_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_FlipSpriteX(zapper_tiles[3 - i], zapper_tiles_flipped[i]);
    }

    //horizontal zapper sprites
    for(uint8_t i = 0; i < 4; ++i)
    {
        horizontal_zapper[i] = gfx_MallocSprite(32, 32);
        horizontal_zapper_flipped[i] = gfx_MallocSprite(32, 32);
        gfx_RotateSpriteC(zapper_tiles[i], horizontal_zapper[i]);
        gfx_RotateSpriteC(zapper_tiles_flipped[i], horizontal_zapper_flipped[i]);
    }

    //horizontal zapper beam
    beam[1] = gfx_MallocSprite(10, 10);
    gfx_RotateSpriteC(beam[0], beam[1]);

    //flipping laser powering up animations
    for(uint8_t i = 0; i < 4; ++i)
    {
        powering_tiles_flipped[i] = gfx_MallocSprite(19, 15);
        gfx_FlipSpriteY(powering_tiles[i], powering_tiles_flipped[i]);
    }

    //flipping laser sprites
    for(uint8_t i = 0; i < 3; ++i)
    {
        firing_tiles_flipped[i] = gfx_MallocRLETSprite(Get_Vertical_RLET_Size(firing_tiles[i]));
        Flip_RLETSpriteY(firing_tiles[i], firing_tiles_flipped[i]);
    }

    for(uint8_t i = 0; i < 3; ++i)
    {
        shutdown_tiles_flipped[i] = gfx_MallocRLETSprite(Get_Vertical_RLET_Size(shutdown_tiles[i]));
        Flip_RLETSpriteY(shutdown_tiles[i], shutdown_tiles_flipped[i]);
    }

    //clear the resizing buffer data since it isn't zero'd; the size of the data is (37*37)-1
    Fill_Array(barryHit_resized->data, 1368, 0);

    //clear the jetpack sprite data too, (25 * 25) - 1 = 624
    Fill_Array(jetpack_resized->data, 624, 0);

    //make a random damage sprite to start with
    Copy_Pasta(barry_hit_tiles[randInt(0, 2)], barryHit_resized, 6, 0);

    //prepare the jetpack
    Copy_Pasta(jetpack, jetpack_resized, 2, 0);

    //best scan mode according to the angry lettuce man
    kb_SetMode(MODE_3_CONTINUOUS);

    //initialize GFX libraries
    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(palette_ptr, 512, 0);
    gfx_SetTransparentColor(0);

    //read from appvar if it exists, fails if it returns 0
    ti_var_t savegame = ti_Open(DATA_APPVAR, "r");
    
    if(!savegame) Write_Savedata();

    //update all of our gameplay data from the save data
    ti_Read(&save_data, sizeof(save_data), 1, savegame);

    //check the appvar version for some future reason
    ti_Read(&save_integrity, 1, 1, savegame);

    //when I first started using C, I asked some friends if there were GOTO statements.
    //They proved they were good friends, and told me "No, that's stupid". I'm glad they lied.
    GAMESTART:
    //But it's still sometimes okay.

    //better random seed generation
    srand(rtc_Time());

    //load all the appvar data as our "starting point" if distance isn't 0 and resume gameplay
    if(save_data.distance)
    {
        //game environment variables
        ti_Read(&scroll_speed, sizeof(scroll_speed), 1, savegame);
        ti_Read(&increment_delay, sizeof(increment_delay), 1, savegame);
        ti_Read(&bg_scroll, sizeof(bg_scroll), 1, savegame);
        ti_Read(&bg_list, sizeof(bg_list), 1, savegame);
        ti_Read(&secondary_bg_list, sizeof(secondary_bg_list), 1, savegame);
        ti_Read(&spawn_delay, sizeof(spawn_delay), 1, savegame);

        //single pointer to avatar struct
        ti_Read(&avatar, sizeof(avatar), 1, savegame);

        //coin variables
        ti_Read(&coins, sizeof(coins), 1, savegame);

        //zapper variables
        ti_Read(&zappers, sizeof(zappers), 1, savegame);

        //missile variables
        ti_Read(&missiles, sizeof(missiles), 1, savegame);

        //laser variables
        ti_Read(&lasers, sizeof(lasers), 1, savegame);

        ti_Read(&opening_delay, sizeof(opening_delay), 1, savegame);

        //jetpack variables
        ti_Read(&jetpack_entity, sizeof(jetpack_entity), 1, savegame);

        //broken wall stuff
        ti_Write(&debris, sizeof(debris), 1, savegame);

        ti_Close(savegame);
    }
    else //make a fresh start and show the game menu
    {
        //decompress the title sprites to the special RAM pointers
        for(uint8_t i = 0; i < 3; ++i)
        {
            zx7_Decompress(background_tiles[9 + i], Get_Tile_Ptr(background_ptr, i + 4 + 1));
        }

        //reset variables for when a game starts
        scroll_speed    = START_SPEED;
        bg_scroll       = 0;
        increment_delay = 0;
        spawn_delay     = 512;
        
        save_data.distance = 0;
        save_data.health   = 1;
        save_data.monies   = 0;

        avatar.x                       = 24;
        avatar.y                       = FLOOR;
        avatar.theta                   = 0;
        avatar.input_duration          = 0;
        avatar.player_animation_toggle = 1;
        avatar.player_animation        = 3;
        avatar.exhaust_animation       = 18;
        avatar.corpse_bounce           = 0;
        avatar.death_delay             = 0;

        //if there's a bug, it's always because the animation values are funky
        missiles.icon_animate     = 0;
        missiles.animation_toggle = -1;

        lasers.deactivated = MAX_LASERS;

        jetpack_entity.theta   = 0;
        jetpack_entity.bounce  = 0;
        jetpack_entity.h_accel = 2;

        //clear all objects from gameplay by moving their X-coords out of bounds
        for(uint8_t i = 0; i < coin_max[coins.formation]; ++i) coins.x[i] = 2000;

        for(uint8_t i = 0; i < MAX_ZAPPERS; ++i) zappers.x[i] = ZAPPER_ORIGIN;

        for(uint8_t i = 0; i < MAX_MISSILES; ++i) missiles.x[i] = (MISSILE_ORIGIN + 10);

        lasers.x = 0;

        //set the backgrounds up for the opening scene
        Set_Background(0);

        //set the first floor tile to a normal wall in case it's been set to the broken tile
        floor_tiles[3] = Get_Tile_Ptr(background_extras_ptr, 17 + 1);

        //if the opening delay isn't zero, run the menu. Retrying skips this part
        if(opening_delay)
        {
            //opening menu function to make this mess more readable
            Title_Menu();

            int8_t title_pos = 50;

            while(!KEY_CLEAR && opening_delay)
            {
                kb_Scan();

                //ceil() doesn't work with ints so here's some numbers I made up and some funky math
                uint8_t deincrement = opening_delay/10 + (opening_delay % 10 != 0);

                if((opening_delay - deincrement) > 0)
                {
                    opening_delay -= deincrement;

                    if((bg_scroll + deincrement) < 46)
                    {
                        bg_scroll += deincrement;
                    }
                    else
                    {
                        //figure out what the scroll would be if it overflowed above 46
                        bg_scroll = bg_scroll + deincrement - 46;

                        //a neat way to shift the opening tiles since they appear in order
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

                //scroll the title menu tiles out of the way since they're static
                gfx_CopyRectangle(gfx_buffer, gfx_buffer, deincrement, 0, 0, 0, 138, 240);

                //we have to redraw the tiles under the menu though
                for(uint8_t i = 3; i < 8; ++i)
                {
                    //these are drawn all speedy-like without clipping calculations
                    gfx_Sprite(ceiling_tiles[secondary_bg_list[i]], (i * 46) - bg_scroll, 0);
                    gfx_Sprite(background_tiles[bg_list[i]], (i * 46) - bg_scroll, 40);
                    gfx_Sprite(floor_tiles[secondary_bg_list[i]], (i * 46) - bg_scroll, 200);
                }

                //draw the title scrawl as it ascends into the heavens
                gfx_RLETSprite(title, 160, title_pos -= deincrement);

                gfx_BlitBuffer();
            }
        }

        if(!KEY_CLEAR)
        {
            //make absolutely sure all the background tiles are set correctly without the menu
            Set_Background(0);

            //overwrite the bg_list entries for the non-existent title sprites with the opening hall ones
            for (uint8_t i = 0; i < sizeof(bg_list); ++i)
            {
                secondary_bg_list[i] = i + 3;
            }

            //make sure the first frame gets drawn when the game is retry'd rather than started from the menu
            Draw_Background();
            gfx_BlitBuffer();
        }

        //I wonder what the opening salaries are for programmers who make up numbers for projects...
        opening_delay = 200;

        //wait for little bit after the screen has settled for that wonderful build-up, then KABOOM
        while(opening_delay && !KEY_CLEAR)
        {
            kb_Scan();
            delay(1);

            --opening_delay;
        }

        //I would combine everything under one [clear] check, but then stuff gets weird when exitting the cutscene
        if(!KEY_CLEAR)
        {
            //draw the greyish flash for when the explosion happens
            Draw_DitheredMesh(3);

            gfx_SwapDraw();

            delay(50);

            //set the first tile to the broken wall
            bg_list[0] = 13;

            //set the lower wall tile sprite to the broken one
            floor_tiles[3] = floor_tiles[14];

            //normally there'd be a SwapDraw() here, but I let the main program loop do it instead here.

            //in fact, it adds a little delay to the flash, meaning that I'm using the... natural lag. (LAUGH AT THE MATH PUN)
        }
    }

    //decompress the normal hall sprites and overwrite the menu tiles
    for(uint8_t i = 0; i < 3; ++i)
    {
        zx7_Decompress(background_tiles[9 + i], Get_Tile_Ptr(background_ptr, i + 1));
    }

    //rotating Barry and the jetpack is way too slow, so I alternate between them every frame instead and keep track with
    //this little bool value
    bool barry_spin = true;

    //color to flash when hit by obstacles, it's a really cool effect and I'm surprised I thought of a way to do it
    uint8_t death_color;

    //time it takes to complete the game loop, used to control the FPS; if it overflows
    //then we have real speed problems
    uint8_t FPS;

    //start up a timer for FPS monitoring, do not move

    timer_Enable(1, TIMER_32K, TIMER_0INT, TIMER_UP);

    //I'm not sure how, but sometimes resuming the game kicks you out and death_delay goes over 50, which also
    //keeps the game from playing. This is a kinda-fix until I figure out what's wrong.
    if(save_data.health)
    {
        avatar.death_delay = 0;
    }

    //Loop until clear is pressed or Barry has been dead for a little while
    while(!KEY_CLEAR && ((avatar.death_delay != 50) || save_data.health))
    {
        //update keys, fixes bugs with update errors that can lead to softlocks
        kb_Scan();

        //missiles need an extra delay because they're just special like that
        if(missile_delay > 0)
        {
            missile_delay -= scroll_speed + 8;
        }

        //spawns stuff, SO much better than the debug methods I originally used, lasers act weird sometimes though
        if((spawn_delay < 1) && (lasers.x < 1))
        {
            uint8_t random_object = randInt(0, 30);
            //uint8_t random_object = randInt(0, 0);

            if(!random_object && (missile_delay <= 0) && save_data.health)
            {
                //picks a random formation from the data arrays in the functions.c file
                lasers.formation = randInt(0, (sizeof(formation_max_lasers) - 1));

                //give each laser their Y-axis
                for(uint8_t i = 0; i < formation_max_lasers[lasers.formation]; ++i)
                {
                    lasers.y[i] = laser_y[lasers.formation][i];
                    lasers.lifetime[i] = half_life[lasers.formation][i];
                }

                lasers.x = 1;

                lasers.deactivated = 0;

                spawn_delay = scroll_speed * 40;
            }
            else if((random_object < 4) && (missile_delay <= 0))
            {
                //sets coin coordinates from coordinate lists
                uint8_t rand_var = randInt(30, 150);

                coins.formation = randInt(0, (COIN_FORMATIONS - 1));

                for(uint8_t i = 0; i < MAX_COINS; ++i)
                {
                    coins.x[i] = (COIN_ORIGIN + coin_form_x[coins.formation][i]);
                    coins.y[i] = (rand_var + coin_form_y[coins.formation][i]);
                    coins.animation[i] = 0;
                }
                spawn_delay = 500;
            }
            else if(random_object < 10)
            {
                //after finishing the spawn checks, make sure missiles don't spawn when Barry is dead, I don't like it when they
                //spawn in and get frozen on screen.
                if(save_data.health)
                {
                    //spawns missiles (and missile swarms if I implement them)
                    for(uint8_t i = 0; i < MAX_MISSILES; ++i)
                    {
                        //if there's a missile offscreen, put it into play
                        if((missiles.x[i] > MISSILE_ORIGIN) || (missiles.x[i] < -54))
                        {
                            missiles.x[i] = MISSILE_ORIGIN;
                            missiles.y[i] = 10 * randInt(2, 18);

                            i = MAX_MISSILES;
                        }
                    }

                    //if there's a missile I can't spawn lasers or coins, so here's a special thing just for it
                    missile_delay = MISSILE_ORIGIN;
                }
            }
            else if(random_object < 30)
            {
                //randomly generates zapper coordinates and lengths
                for(uint8_t i = 0; i < MAX_ZAPPERS; ++i)
                {
                    //if the zapper isn't in play
                    if((zappers.x[i] >= ZAPPER_ORIGIN) || (zappers.x[i] < -100))
                    {
                        //put it in play
                        zappers.x[i] = ZAPPER_ORIGIN - 1;

                        //zapper length is always set the same way, regardless of orientation
                        zappers.length[i] = randInt(2, 4) * 10;

                        //if randInt returns anything that isn't zero, make a vertical zapper
                        if(randInt(0, 2))
                        {
                            zappers.orientation[i] = 0;

                            //vertical zappers can appear from y-20 to y-170
                            zappers.y[i] = 20 * randInt(1, 8 - (zappers.length[i] / 20));

                            //delay the next spawn
                            spawn_delay = randInt(140, 200);
                        }
                        else //if zero, then it's a horizontal zapper
                        {
                            zappers.orientation[i] = 1;

                            //horizontal zappers do be spawnin' everywhere tho
                            zappers.y[i] = (20 * randInt(1, 10)) - 5;

                            //delay the next spawping with respect for the elongation of the zapper
                            spawn_delay = zappers.length[i] + randInt(140, 200);
                        }

                        //if we made a zapper, stop making more
                        i = MAX_ZAPPERS;
                    }
                }
            }
        }
        else
        {
            spawn_delay -= scroll_speed;
        }

        //run controls and scrolling speed until Barry gets wasted, then bounces his corpse around
        if(save_data.health > 0)
        {
            //very small bit of code to increase speed with a decreasing frequency over time, max of 12
            if((increment_delay >= ((scroll_speed - 5) * 250)) && (scroll_speed < MAX_SPEED))
            {
                ++scroll_speed;
                increment_delay = 0;
            }
            else
            {
                ++increment_delay;
            }

            //when flying
            if(KEY_2ND)
            {
                //add to avatar.input_duration, which is added to avatar.y
                if(avatar.input_duration < 12)
                {
                    avatar.input_duration += 1;
                }

                //numbers for jetpack output (bullets or fire or whatever)
                if(avatar.exhaust_animation < 7)
                {
                    ++avatar.exhaust_animation;
                } else if(avatar.exhaust_animation > 7) {
                    avatar.exhaust_animation = 0;
                }

            //when falling (2nd isn't pressed)
            } else {
                //subtract from avatar.input_duration, added to avatar.y (it can be a negative number)
                if((avatar.y < (FLOOR)) && (avatar.input_duration > -12))
                {
                    avatar.input_duration -= 1;
                }

                //increases numbers for jetpack powering down animation
                if(avatar.exhaust_animation < 7)
                {
                    avatar.exhaust_animation = 8;
                }
                else if(avatar.exhaust_animation < 11)
                {
                    ++avatar.exhaust_animation;
                }
                else
                {
                    avatar.exhaust_animation = 18;
                }
            }

            //sees if Y-value rolled past the ceiling or floor values, figures out if it was going up
            //or down, and auto-corrects accordingly (FUTURE ME: IT'S OPTIMIZED ENOUGH PLEASE DON'T WASTE ANY MORE TIME)
            if(((avatar.y - avatar.input_duration) > FLOOR) || ((avatar.y - avatar.input_duration) < CEILING))
            {
                if(avatar.input_duration > 0)
                {
                    avatar.y = CEILING;
                } else {
                    avatar.y = FLOOR;
                }
                avatar.input_duration = 0;
            }
            else //if Y-value didn't roll past
            {
                //SAX got pretty mad about this part, turns out a linear equation can model a curve and is actually better
                //than a cubic function
                avatar.y -= avatar.input_duration;
            }

            //bit that runs avatar animations
            if(avatar.y < FLOOR)
            {
                //flying animation
                avatar.player_animation = 6;
            }
            else
            {
                //quickly reset sprite animation to the second running sprite
                if(avatar.player_animation == 6)
                {
                    avatar.player_animation = 2;
                }

                //toggle the animation frame order from first to last and vice versa
                if(avatar.player_animation < 1)
                {
                    avatar.player_animation_toggle = 1;
                }
                else if(avatar.player_animation > 4)
                {
                    avatar.player_animation_toggle = -1;
                }
                avatar.player_animation += avatar.player_animation_toggle;
            }
        }
        else //if "THE HEAVY IS DEAD!", calculate an inelastic collision on his corpse
        {   
            //when the crap hits the fan, or the Barry hits the floor I dunno I didn't read the manga
            if((avatar.y - avatar.input_duration) >= FLOOR)
            {
                avatar.y = FLOOR;

                //if scroll_speed isn't zero
                if(scroll_speed)
                {
                    //slow down, rotate, and count the bounces
                    --scroll_speed;
                    ++avatar.corpse_bounce;

                    avatar.input_duration *= -1;
                    avatar.input_duration -= 3;

                    if(avatar.input_duration < 3)
                    {
                        //if the acceleration drops beneath a certain point, consider Barry bounced
                        avatar.corpse_bounce = 3;
                    }
                    else
                    {
                        //empty the resized sprite
                        Fill_Array(barryHit_resized->data, 1369, 0);

                        //pick a new random sprite to spin around
                        Copy_Pasta(barry_hit_tiles[randInt(0, 2)], barryHit_resized, 6, 0);
                    }
                }
                else //increment a timer for waiting a bit before going to death menu
                {
                    ++avatar.death_delay;
                }
                
            }
            else if((avatar.y - avatar.input_duration) < CEILING) //if Barry hits the ceiling, flip his acceleration
            {
                avatar.y = CEILING;
                avatar.input_duration *= -1;
            }
            else
            {
                avatar.y -= avatar.input_duration;
                avatar.theta += BARRY_DEFLECTION;

                //downwards acceleration increase and cap, slightly faster than normal
                if(avatar.input_duration > -14)
                {
                    --avatar.input_duration;
                }

                //correct previous acceleration if Barry is well and truly ded
                if(avatar.corpse_bounce >= 3)
                {
                    avatar.input_duration = 0;
                    //this doesn't reset sometimes, still not sure why
                    avatar.y = FLOOR;
                }
            }

            //I'm keeping the jetpack rotation and control code seperate until I'm ready to hybridize the old code with it.

            //if it hits the floor, which is slighty lower for the jetpack in order to give a more 3D look
            if((jetpack_entity.y - jetpack_entity.v_accel) >= (FLOOR + 20))
            {
                jetpack_entity.y = FLOOR + 20;

                if(jetpack_entity.bounce < 3)
                {
                    ++jetpack_entity.bounce;

                    //stop the bouncing if the numbers are getting to small
                    if(jetpack_entity.v_accel < -6)
                    {
                        jetpack_entity.v_accel *= -1;
                        jetpack_entity.v_accel -= 3;
                    }
                    else
                    {
                        jetpack_entity.v_accel = 0;
                        jetpack_entity.bounce = 3;
                    }

                    //slow down the forward motion after hitting the ground when the screen stops moving
                    if(jetpack_entity.h_accel && !scroll_speed)
                    {
                        --jetpack_entity.h_accel;
                    }
                }
                else
                {
                    //equivalent to rotating 270 degrees clockwise
                    jetpack_entity.theta = 192;
                }
            }
            else if((jetpack_entity.y - jetpack_entity.v_accel) < CEILING) //if it hits the ceiling
            {
                jetpack_entity.y = CEILING;
                jetpack_entity.v_accel *= -1;
            }
            else
            {
                jetpack_entity.x += jetpack_entity.h_accel;
                jetpack_entity.y -= jetpack_entity.v_accel;

                //the jetpack rotates faster than Barry because I feel like it
                jetpack_entity.theta += JETPACK_DEFLECTION;

                if(jetpack_entity.v_accel > -12)
                {
                    --jetpack_entity.v_accel;
                }
            }

            //since rotating sprites is so slow, I'm doing it between drawing and buffer updates to increase speed
            if(barry_spin)
            {
                gfx_RotateSprite(barryHit_resized, barryHit_rotated, avatar.theta);
                barry_spin = false;
            }
            else
            {
                gfx_RotateSprite(jetpack_resized, jetpack_rotated, jetpack_entity.theta);
                barry_spin = true;
            }

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

        //control missile warning and incoming animation orders
        if(missiles.icon_animate < 1)
        {
            missiles.animation_toggle = 1;
        }
        else if(missiles.icon_animate > 4)
        {
            missiles.animation_toggle = -1;
        }
        missiles.icon_animate += missiles.animation_toggle;

        //animate missile sprites
        if(missiles.animation < 12)
        {
            ++missiles.animation;
        }
        else //but on the flip side, this else might be worse looking...
        {
            missiles.animation = 0;
        }

        //move lasers into play when needed
        if(lasers.deactivated < formation_max_lasers[lasers.formation])
        {
            spawn_delay = scroll_speed * 40;

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

        //I should've made this function many commits ago...
        Draw_Background();

        //draws avatar depending on health 'n stuff
        if(save_data.health > 0)
        {
            //draws the avatar, everything is layered over it
            gfx_RLETSprite_NoClip(barry_run_tiles[avatar.player_animation / 2], avatar.x, avatar.y);

            //bit that draws exhaust when in flight
            if(avatar.exhaust_animation < 18)
            {
                gfx_RLETSprite_NoClip(exhaust_tiles[avatar.exhaust_animation / 2], avatar.x + randInt(1, 3), avatar.y + 31);
                gfx_TransparentSprite_NoClip(nozzle, avatar.x + 4, avatar.y + 31);
            }
        }
        else
        {
            //if Barry is ded and his body hath struck the ground thrice, then the end times are upon us
            if(avatar.corpse_bounce >= 3)
            {
                //I hardcoded barry's corpse's Y-pos for effeciency
                gfx_TransparentSprite_NoClip(barry_ded_tiles[1], avatar.x, 210);
            }
            else
            {
                //we now draw that spinny sprite that's hopefully not going to suck up RAM like crazy
                gfx_TransparentSprite_NoClip(barryHit_rotated, avatar.x - 5, avatar.y);
            }

            //draw the jetpack seperate from everything else
            gfx_TransparentSprite_NoClip(jetpack_rotated, jetpack_entity.x, jetpack_entity.y);
        }

        //bit that runs coin collision and movement
        for(uint8_t i = 0; i < coin_max[coins.formation]; ++i)
        {
            //do things if the coin is less than the origin plus some buffer I made up on the spot
            if(coins.x[i] <= COIN_ORIGIN + 500)
            {
                //collision detection and appropriate sprite drawing
                if(gfx_CheckRectangleHotspot(avatar.x + 2, avatar.y, 18, 37, coins.x[i] - 11, coins.y[i] + 1, 10, 10))
                {
                    //I saved the coin poof as the last tile, it's only drawn when Barry gets a coin (duh)
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
                coins.x[i] -= scroll_speed;
            }
        }

        //bit that calculates zappers 'n stuff
        for(uint8_t i = 0; i < MAX_ZAPPERS; ++i)
        {
            if(zappers.x[i] != ZAPPER_ORIGIN)
            {
                //if orientation is zero, use data to draw vertical sprites
                if(!zappers.orientation[i])
                {
                    for(uint8_t j = 0; j < zappers.length[i]; j += 10)
                    {
                        gfx_TransparentSprite(beam[0], zappers.x[i] - 14, zappers.y[i] + 16 + j);
                    }

                    //draw zapper pairs and beams with distance of zapperLength between them
                    gfx_TransparentSprite(zapper_tiles_flipped[zappers.animate/3], zappers.x[i] - 25, zappers.y[i] - 7);
                    gfx_TransparentSprite(zapper_tiles[zappers.animate/3], zappers.x[i] - 25, zappers.y[i] + 7 + zappers.length[i]);

                    //collision for vertical zappers
                    if((save_data.health > 0) && (gfx_CheckRectangleHotspot(zappers.x[i] - 14, zappers.y[i] + 2, 10, zappers.length[i] + 30, avatar.x + 2, avatar.y, 18, 37)))
                    {
                        --save_data.health;

                        //screen flashes yellow
                        death_color = 4;
                    }
                }
                else //if the orientation is not zero
                {
                    //draw the horizontal beam, which WAS painful as hekk to do mathwise before I changed everything. Again.
                    for(uint8_t j = 0; j < zappers.length[i]; j += 10)
                    {
                        //horizontal zappers are why I had to change all my x-values to int24 instead of uint24
                        gfx_TransparentSprite(beam[1], zappers.x[i] + 32 + j, zappers.y[i] + 11);
                    }

                    //left 'n right emitter thingies
                    gfx_TransparentSprite(horizontal_zapper[zappers.animate / 3], zappers.x[i], zappers.y[i]);
                    gfx_TransparentSprite(horizontal_zapper_flipped[zappers.animate / 3], zappers.x[i] + 32 + zappers.length[i], zappers.y[i]);
                
                    //collision for horizontal zappers, a 68-88x10 rectangle
                    if((save_data.health > 0) && (gfx_CheckRectangleHotspot(zappers.x[i] + 8, zappers.y[i], zappers.length[i] + 48, 10, avatar.x, avatar.y, 20, 37)))
                    {
                        --save_data.health;

                         //screen flashes yellow
                        death_color = 4;
                    }
                }
                //all zappers move the same
                zappers.x[i] -= scroll_speed;
            }
        }

        //bit that draws and calculates the missiles 'o death
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
                        death_color = 5;
                    }

                    //I'm using the X-position as timer, meaning there's less warning as time goes on
                }
                else if(missiles.x[i] < 641)
                {
                    //AW CRAP HERE COME DAT BOI!
                    gfx_RLETSprite_NoClip(missile_incoming_tiles[missiles.icon_animate / 3], 281 + randInt(-1, 1), missiles.y[i] - 16 + randInt(-1, 1));
                }
                else if(missiles.x[i] < MISSILE_ORIGIN)
                {
                    //plenty of time to dodge (at the beginning at least)
                    gfx_RLETSprite_NoClip(missile_warning_tiles[missiles.icon_animate / 2], 293, missiles.y[i] - 11);

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
                missiles.x[i] -= scroll_speed + 8;
            }
        }

        //bit for lasers, lots of moving parts
        for(uint8_t i = 0; i < formation_max_lasers[lasers.formation]; ++i)
        {
            if(lasers.x > 0)
            {
                uint8_t animation_index = lasers.animation / 3;

                //reset the laser animations
                if((lasers.x < 20) && (lasers.deactivated < formation_max_lasers[lasers.formation]))
                {
                    lasers.animation = 0;

                    //draw an inactive laser
                    gfx_TransparentSprite(powering_tiles[0], lasers.x - 19, lasers.y[i]);
                    gfx_TransparentSprite(powering_tiles[0], 320 - lasers.x, lasers.y[i]);
                }
                else
                {
                    //if they are finished, finish the animation sequence and hide them again
                    if(lasers.deactivated >= formation_max_lasers[lasers.formation])
                    {
                        lasers.animation = 0;

                        //if they aren't dead and have moved, run through their animations and attacks
                    }
                    else
                    {
                        //check to see if the half-life ended
                        if(lasers.lifetime[i] < 1)
                        {
                            ++lasers.deactivated;
                        }
                        else if(lasers.lifetime[i] <= 8)
                        {
                            //lasers running out of juice
                            gfx_RLETSprite_NoClip(shutdown_tiles[animation_index], 5, lasers.y[i] - 16);
                            gfx_RLETSprite_NoClip(shutdown_tiles_flipped[animation_index], 285, lasers.y[i] - 16);

                            //drawing the laser beam line by line since that's faster and smaller than using a sprite
                            for(uint8_t j = 0; j < 15; ++j)
                            {
                                if(laser_tiles[animation_index]->data[j])
                                {
                                    gfx_SetColor(laser_tiles[animation_index]->data[j]);
                                    gfx_HorizLine_NoClip(35, lasers.y[i] + j, 250);
                                }
                            }
                        }
                        else if(lasers.lifetime[i] < 64)
                        {
                            //firing lasers
                            gfx_RLETSprite_NoClip(firing_tiles[animation_index], 5, lasers.y[i] - 11);
                            gfx_RLETSprite_NoClip(firing_tiles_flipped[animation_index], 285, lasers.y[i] - 11);

                            for(uint8_t j = 0; j < 15; ++j)
                            {
                                if(laser_tiles[3]->data[j])
                                {
                                    gfx_SetColor(laser_tiles[3]->data[j]);
                                    gfx_HorizLine_NoClip(35, lasers.y[i] + j, 250);
                                }
                            }

                            //hitbox for damage. 3 pixel leeway above and below
                            if((save_data.health > 0) && Yspot(avatar.y, 34, lasers.y[i], 12))
                            {
                                --save_data.health;

                                //again, the screen flashes red
                                death_color = 5;
                            }

                            //makes sure the animations play correctly, I don't have time to fix math today
                            if(lasers.lifetime[i] == 9) lasers.animation = 0;
                        }
                        else if(lasers.lifetime[i] <= 120)
                        {
                            //the simplest drawing code in this hot mess, for powering up
                            gfx_SetColor(5);

                            gfx_Line_NoClip(20, lasers.y[i] + 7, 300, lasers.y[i] + 7);

                            gfx_Circle_NoClip(11, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle_NoClip(11, lasers.y[i]+7, 6 + ((lasers.lifetime[i]-50)/4));

                            gfx_Circle_NoClip(308, lasers.y[i] + 7, 7 + ((lasers.lifetime[i] - 50) / 4));
                            //gfx_Circle_NoClip(308, lasers.y[i]+7, 8 + ((lasers.lifetime[i]-50)/4));
                        }

                        //I thought there was an issue with how I deactivated lasers, but it was really an int24 overflow
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

        //sets a new highscore
        if(save_data.highscore < ((save_data.distance += scroll_speed) / 15))
        {
            save_data.highscore = (save_data.distance / 15);
        }

        //draw the score 'n stuff unless we're on the last frame of Barry's corpse bouncing
        if((avatar.death_delay != 50) || save_data.health)
        {
            //distance and FPS counter are grey
            gfx_SetTextFGColor(2);

            gfx_SetTextScale(2, 2);
            gfx_SetTextXY(10, 5);
            //I pulled that number out of my butt and it gives good results, distance in pixels divided by 15
            gfx_PrintUInt(save_data.distance / 15, 4);

            gfx_SetTextScale(1, 1);

            //print a little meters symbol after the distance
            gfx_PrintStringXY("M", gfx_GetTextX(), 13);

            gfx_PrintStringXY("BEST:", 10, 24); gfx_PrintInt(save_data.highscore, 1);

            gfx_SetTextXY(280, 10);
            gfx_PrintUInt(FPS, 1);

            /*for(uint8_t i = 0; i < 3; ++i)
            {
                gfx_SetTextXY(260, 30 + (10 * i));
                gfx_PrintUInt(Get_RLET_Size(shutdown_tiles[i]), 1);
            }*/

            //gold coin color for money counter
            gfx_SetTextFGColor(4);

            gfx_SetTextXY(10, 36);
            gfx_PrintUInt(save_data.monies, 3);
        }

        //speedy blitting, but works best when used a lot of cycles BEFORE any new drawing functions
        gfx_SwapDraw();

        //afterwards, we check if Barry was hit, and if so then the screen will quickly flash a certain color
        if(death_color)
        {
            //does the neat transparency effect for taking damage
            Draw_DitheredMesh(death_color);
            
            //blit the color
            gfx_SwapDraw();

            //we also set some variables that need to be updated once at time of death
            jetpack_entity.y = avatar.y + 7;

            jetpack_entity.x = avatar.x;
            jetpack_entity.v_accel = avatar.input_duration + 2;

            for(uint8_t i = 0; i < sizeof(lasers.y); ++i)
            {
                //either overflow the int24 instantly or put it in the last animation frames
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

            //reset the flag, comment this out if you need to off an epileptic cheese-themed supervillian
            death_color = 0;
        }

        //FPS counter data collection, time "stops" (being relevant) after this point
        FPS = (32768 / timer_GetSafe(1, TIMER_UP));
        //the GetSafe() is to make Tari stop bothering me

        //time is frozen for a delay, and I wanna make a Jo-Jo reference but I don't speak Japanese
        if(FPS > MAX_FPS)
        {
            delay(40 - (1000 / FPS));
        }

        //pause menu controls and drawing, can be accessed as long as in main loop
        if(KEY_DEL)
        {
            int8_t choice = Pause_Menu();

            //imagine completing menuing checks when [clear] is pressed and leaving it in the code for 4 commits.
            if(!KEY_CLEAR)
            {
                //quitting sets the opening delay to 138 along with some other values
                if(!choice)
                {
                    //set that menu delay variable to a positive value that I'll use for some funky math
                    opening_delay = 138;

                    bg_scroll = 0;

                    //set this to flag that the game is over
                    save_data.distance = 0;
                }
                else if(choice == 1) //reset the game without the opening bit
                {
                    opening_delay      = 0;
                    bg_scroll          = 0;
                    save_data.distance = 0;
                }
                //if choice is 2 or anything else it just resumes; everything resets when the pause menu is accessed again.

                goto GAMESTART;
            }
        }

        //and with a timer reset "ZA WARUDO" is over (iS tHaT A jOJo rEfERenCe and also yes I looked it up)
        timer_1_Counter = 0;

        //controls bg_scroll, 46 is for background sprite width
        if((bg_scroll + scroll_speed) >= 46)
        {
            //seamlessly reset the position and increment it
            bg_scroll -= (46 - scroll_speed);

            //move the background tileset lists
            for(uint8_t i = 0; i < 8; ++i)
            {
                bg_list[i] = bg_list[i + 1];
                secondary_bg_list[i] = secondary_bg_list[i + 1];
            }

            //if the last entry has been "dragged" through the next 2 list entries, make new sprite addresses;
            //note that I only check the background tiles since the timings are the same for all tile insertions
            if(bg_list[7] == bg_list[8])
            {
                if(bg_list[8] < 8)
                {
                    //this just makes sure that we've done the opening 12 tiles before we start spawning the 16
                    ++bg_list[8];
                    ++secondary_bg_list[8];
                }
                else if(bg_list[6] == bg_list[8])
                {
                    bg_list[7] = 9 + randInt(0, 1) * 2;
                    bg_list[8] = bg_list[7] + 1;

                    secondary_bg_list[7] =  12;
                    secondary_bg_list[8] = 13;
                }
            }
            //the problem is that all the tiles are only half (46 px) of a single hallway "chunk" of 92 pixels.
        }
        else
        {
            bg_scroll += scroll_speed;
        }
    }

    //if we didn't quit out of the game and Barry is clearly dead, pull up the death screen
    if(!KEY_CLEAR && ((avatar.death_delay >= 50) && !save_data.health))
    {
        //do all the graphical stuff and menuing until some decision is reached
        uint8_t choice = Ded_Menu();

        //quitting sets the opening delay to 138 along with some other values
        if(!choice)
        {
            //reset the game without the opening bit and start it immediately
            opening_delay      = 0;
            bg_scroll          = 0;
            save_data.distance = 0;
        }
        else if(choice) //quitting and clearing do the same thing here since the variables need to be reset for the next game
        {
            //set that menu delay variable to a positive value that I'll use for some funky math
            opening_delay = 138;

            bg_scroll = 0;

            //set this to flag that the game is over and the menu should come up
            save_data.distance = 0;
        }

        //only go to the start if clear wasn't pressed
        if(choice != 2) goto GAMESTART;        
    }
    
    //we only need distance and a few other vars to be saved, but I made this function AND I'M GONNA USE THE WHOLE FUNCTION.
    Write_Savedata();

    //stop libraries, not doing so causes "interesting" results by messing up the color mode and scaling
    gfx_End();

    return 1;
}

//CONGRATION, YOU FOUND THE END OF THE FILE! I HOPE PEEKING BEHIND THE CURTAIN WAS WORTH YOUR SANITY!
/* Jetpack Joyride CE - King Dub Dub

A Jetpack Joyride port for the TI-84 Plus CE calculators.

A rewrite of the original Jetpack Joyride port I made in highschool when I was stupid and had
a "make it work and be fast" mentality and no care for my sanity or coding etiquette. This should
run better/be a better example of what you can do with the power of pointer witchcraft.

NOTES
    - Sprites are scaled down to a 1:0.720720720
    - All in-game text must be capitalized since the font doesn't include lowercase
    - These chars are not implemented in the font: ` { | } ~
*/

#include <stddef.h>
#include <string.h>
#include <math.h>

#include <tice.h>
#include <ti/getcsc.h>
#include <sys/timers.h>
#include <time.h>
#include <compression.h>
#include <keypadc.h>
#include <graphx.h>

#include "headers.h"



// These are for background sprite memory regions that will never be unloaded during gameplay.
// The compiler refuses let me put these in a different file than where their arrays are declared.
// Because of this they have to be here and visually disgusting code must be left in.
gfx_TempSprite(bg_tile_4, 46, 160);
gfx_TempSprite(bg_tile_5, 46, 160);
gfx_TempSprite(bg_tile_6, 46, 160);
gfx_TempSprite(bg_tile_7, 46, 160);

gfx_TempSprite(ceiling_tile_4, 46, 160);
gfx_TempSprite(ceiling_tile_5, 46, 160);

gfx_TempSprite(floor_tile_4, 46, 160);
gfx_TempSprite(floor_tile_5, 46, 160);

gfx_TempSprite(button0, 8, 30);
gfx_TempSprite(button1, 8, 30);
gfx_TempSprite(button2, 8, 30);
gfx_TempSprite(button3, 8, 30);

gfx_TempSprite(select_button0, 8, 26);
gfx_TempSprite(select_button1, 8, 26);
gfx_TempSprite(select_button2, 8, 26);
gfx_TempSprite(select_button3, 8, 26);

int main(void)
{
    gfx_SetFontData(jetpackia);
    gfx_SetFontSpacing(jetpackia_spacing);

    const void *palette_ptr    = Get_Appvar_Ptr("JPJRPAL");

    const void *background_ptr = Get_Appvar_Ptr("JPJRBG");

    const void *graphics_ptr   = Get_Appvar_Ptr("JPJRGFX");

    if(!palette_ptr || !background_ptr || !graphics_ptr)
    {
        // Using gfx_End means we don't need to compile os_ClrHome and use 13 extra bytes just to
        // clear the screen.
        gfx_End();
        
        os_PutStrLine("Missing graphics appvars!");
        delay(2000);

        return 0;
    }

    player.x             = 20;
    player.y             = FLOOR - BARRY_HEIGHT;
    player.dy            = 0;
    player.theta         = 0;
    player.frame         = 3;
    player.frame_toggle  = 1;
    player.exhaust_frame = 0;
    player.bounces       = 2;

    gfx_sprite_t *barry[4];
    gfx_rletsprite_t *exhaust[6];

    // This is an array of pointers to various objects with an ID byte that determines what
    // graphics and operations need to be done. Each entity has their own type but all have the
    // same first byte for ID, so I can pass a mix of them to functions for pointer witchcraft.
    void    *entities[MAX_ENTITIES] = {0};
    uint8_t  entity_count = 0;

    // Allocate RAM for player sprites and decompress immediately
    for(uint8_t i = 0; i < sizeof(barry) / sizeof(barry[0]); ++i)
    {
        barry[i] = gfx_MallocSprite(27, 37);

        zx0_Decompress(barry[i], Get_Tile_Ptr(graphics_ptr, 1 + i));
    }

    for(uint8_t i = 0; i < sizeof(exhaust) / sizeof(exhaust[0]); ++i)
    {
        exhaust[i] = Get_Tile_Ptr(graphics_ptr, 5 + i);
    }

    // The vertical edges of the popup menu screen
    window         = gfx_MallocSprite(8, 167);
    window_flipped = gfx_MallocSprite(8, 167);

    zx0_Decompress(window, Get_Tile_Ptr(graphics_ptr, 11));
    gfx_FlipSpriteY(window, window_flipped);

    // I've declared these externally for function reasons, and here we assign their temp sprites.
    // It looks bad, but this is a C limitation and I can't find a better way to shrink/clean it up.
    button[0] = button0; button[1] = button1; button[2] = button2; button[3] = button3;
    select_button[0] = select_button0; select_button[1] = select_button1; select_button[2] = select_button2; select_button[3] = select_button3;

    for(uint8_t i = 0; i < sizeof(button) / sizeof(button[0]); ++i)
    {
        zx0_Decompress(button[i],        Get_Tile_Ptr(graphics_ptr, 12 + i));
        zx0_Decompress(select_button[i], Get_Tile_Ptr(graphics_ptr, 16 + i));
    }

    // Get the coin sparkle/pickup effect and animation sprites
    for(uint8_t i = 0; i < sizeof(coin_sprite) / sizeof(coin_sprite[0]); ++i)
    {
        coin_sprite[i] = gfx_MallocSprite(16, 15);

        zx0_Decompress(coin_sprite[i], Get_Tile_Ptr(graphics_ptr, 20 + i));
    }

    // Speeds up kb_Scan() updates but uses interrupts to poll constantly
    kb_SetMode(MODE_3_CONTINUOUS);

    // Start graphics and draw to a buffer instead of directly to the screen
    gfx_Begin();
    gfx_SetDrawBuffer();

    // Assuming we use all 256 color entries with 16 bit color, that's 512 bytes for the palette.
    gfx_SetPalette(palette_ptr, 256 * 2, 0);

    gfx_SetTextFGColor(WHITE);

    uint8_t scroll;
    uint8_t pause_action = 0;

    int24_t spawn_delay;

    while(!KEY_CLEAR && pause_action != 2)
    {
        pause_action = 0;

        // I would break this down so it's more visible, but I think you get the gist. There's an
        // entry for every background sprite with only 8 pointing to memory addresses since only 8
        // are on screen at once, and when one goes offscreen I decompress a new sprite to it and
        // shuffle the address to entry of the normal tileset. If I decompressed everything at once
        // I'd fill the RAM 2x over.
        gfx_sprite_t *background[16] = {0, 0, 0, 0, bg_tile_4,      bg_tile_5,      bg_tile_6, bg_tile_7, 0, 0, 0, 0, 0, 0, 0, 0};
        gfx_sprite_t *ceiling[14]    = {0, 0, 0, 0, ceiling_tile_4, ceiling_tile_5, 0,         0,         0, 0, 0, 0, 0, 0};
        gfx_sprite_t *floor[14]      = {0, 0, 0, 0, floor_tile_4,   floor_tile_5,   0,         0,         0, 0, 0, 0, 0, 0};
        // Note that only some sprites get heap-allocated tiles, these will always be used and will
        // become the sprites for the main hallway once the "leapfrogging" stops after the starting
        // area is unloaded. It looks gross, but this is what's required of me to save memory.

        uint8_t bg_tiles[8]       = {0, 1, 2, 3, 4, 5, 6, 7};
        uint8_t bg_extra_tiles[8] = {0, 1, 2, 3, 4, 5, 6, 7};

        uint8_t tile_offset;

        for(uint8_t i = 0; i < 4; ++i) // Temporary background wall tile sprites I'll free up later
        {
            background[i] = gfx_MallocSprite(46, 160);
        }

        for(uint8_t i = 0; i < 8; ++i) // Background walls
        {
            zx0_Decompress(background[i], Get_Tileset_Tile_Ptr(background_ptr, 0, i));
        }

        for(uint8_t i = 0; i < 8; ++i) // Ceiling and floor background sprites
        {
            if(i != 4 && i != 5)
            {
                ceiling[i] = gfx_MallocSprite(46, 40);
                floor[i]   = gfx_MallocSprite(46, 40);
            }

            zx0_Decompress(ceiling[i], Get_Tileset_Tile_Ptr(background_ptr, 1, i));
            zx0_Decompress(floor[i], Get_Tileset_Tile_Ptr(background_ptr, 2, i));
        }

        savedata.monies   = 0;
        savedata.distance = 0;
        savedata.health   = 1;
        
        // Opening menu section, only loads if a valid save isn't found
        if(true)
        {
            int24_t countdown = 138;

            scroll = 0;
            tile_offset = 0;

            spawn_delay = 300;

            // The title sprite is too big and would prevent me from loading stuff so I read it
            // directly from the appvar in the flash memory. It's slower than RAM but not a big
            // deal for cutscene.
            gfx_rletsprite_t *title = Get_Tile_Ptr(graphics_ptr, 0);

            int24_t title_y = 50 * 2;

            uint8_t selector_y = 5;

            gfx_SetTextScale(1, 1);

            while(!KEY_CLEAR && countdown)
            {
                if(countdown > 2)
                {
                    countdown -= scroll;
                } else {
                    countdown = 0;
                }

                tile_offset += scroll;

                for(uint8_t i = 0; i < sizeof(bg_tiles); ++i)
                {
                    gfx_Sprite(ceiling[bg_extra_tiles[i]], (i * 46) - tile_offset, 0);
                    gfx_Sprite(background[bg_tiles[i]],    (i * 46) - tile_offset, 40);
                    gfx_Sprite(floor[bg_extra_tiles[i]],   (i * 46) - tile_offset, 200);
                }

                gfx_RLETSprite(title, 160, (title_y -= (2 * scroll)) / 2);

                gfx_BlitBuffer();

                // Scroll the tiles to the left and shift each one in the tile list when they've
                // traveled their width.
                if(tile_offset >= 46)
                {
                    countdown += tile_offset - 46;
                    
                    tile_offset = 0;

                    // Here we swap upcoming empty sprite pointers for old ones that will be
                    // overwritten with new sprite data as they show up. This way I don't have to
                    // allocate memory for every background tile, just the 8 that will be on screen.
                    ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] + 1] = ceiling[bg_extra_tiles[0]];
                    zx0_Decompress(ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] + 1], Get_Tileset_Tile_Ptr(background_ptr, 1, bg_extra_tiles[sizeof(bg_extra_tiles) - 1] + 1));

                    // We'll also want to wipe that old pointer for making some checks easier in the future
                    ceiling[bg_extra_tiles[0]] = 0;

                    background[bg_tiles[sizeof(bg_tiles) - 1] + 1] = background[bg_tiles[0]];
                    zx0_Decompress(background[bg_tiles[sizeof(bg_tiles) - 1] + 1], Get_Tileset_Tile_Ptr(background_ptr, 0, bg_tiles[sizeof(bg_extra_tiles) - 1] + 1));

                    background[bg_tiles[0]] = 0;

                    floor[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] + 1] = floor[bg_extra_tiles[0]];
                    zx0_Decompress(floor[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] + 1], Get_Tileset_Tile_Ptr(background_ptr, 2, bg_extra_tiles[sizeof(bg_extra_tiles) - 1] + 1));

                    floor[bg_extra_tiles[0]] = 0;

                    for(uint8_t i = 0; i < sizeof(bg_tiles) - 1; ++i)
                    {
                        bg_tiles[i]       = bg_tiles[i + 1];
                        bg_extra_tiles[i] = bg_extra_tiles[i + 1];
                    }

                    ++bg_tiles[sizeof(bg_tiles) - 1];
                    ++bg_extra_tiles[sizeof(bg_tiles) - 1];
                }

                // Key debouncing my beloved (it's funny because it sucks)
                bool debounced = false;

                // This is where the actual menuing code goes
                while(!(KEY_2ND && (selector_y == 5)) && !scroll && !KEY_CLEAR)
                {
                    kb_Scan();

                    if(KEY_UP)
                    {
                        selector_y -= 35;
                    } else if(KEY_DOWN) {
                        selector_y += 35;
                    }

                    if(selector_y == 226)
                    {
                        selector_y = 110;
                    } else if(selector_y == 145) {
                        selector_y = 5;
                    }

                    for(uint8_t i = 0; i < 3; ++i)
                    {
                        gfx_Sprite_NoClip(ceiling[i],    46 * i, 0);
                        gfx_Sprite_NoClip(background[i], 46 * i, 40);
                    }

                    gfx_SetColor(WHITE);
                    gfx_Rectangle_NoClip(36, selector_y, 98, 31);

                    gfx_BlitBuffer();

                    while(KEY_2ND && !debounced) kb_Scan();

                    debounced = true;

                    // Wait until all arrow keys are released
                    while(kb_Data[7]) kb_Scan();

                    if(KEY_2ND) // Here's where the submenuing happens
                    {
                        debounced = false;

                        switch(selector_y + KEY_CLEAR)
                        {
                            case 40: // shop stuff
                                
                            break;
                            
                            case 75: // settings
                                
                            break;
                            
                            case 110: // How it's made + the power of friendship or whatever
                                
                                Draw_StretchedSprite(window, window_flipped, 33, 33, 238);

                                gfx_BlitBuffer();

                                uint8_t start_line = 0;

                                do {
                                    kb_Scan();

                                    if(KEY_UP && (start_line != 0))
                                    {
                                        start_line -= 2;
                                    } else if(KEY_DOWN && (start_line != 18)) {
                                        start_line +=2;
                                    }

                                    for(uint8_t i = 0; i < 12; ++i)
                                    {
                                        gfx_PrintStringXY(about_txt[start_line + i], 42, 60 + i*12);
                                    }

                                    gfx_BlitRectangle(gfx_buffer, 38, 58, 244, 137);

                                    gfx_CopyRectangle(gfx_buffer, gfx_buffer, 38, 58, 39, 58, 243, 137);

                                    while((KEY_2ND && !debounced) || kb_Data[7]) kb_Scan();

                                    debounced = true;
                                } while(!KEY_CLEAR && !KEY_2ND);

                                while(KEY_CLEAR) kb_Scan();
                            break;
                        }

                        break;

                        debounced = false;
                    }
                }

                if(selector_y == 5)
                {
                    // The +1 makes sure that even when the math rounds down scroll won't be zero,
                    // and this looks more natural than the visible change using an if() statement
                    // makes when correcting it. Fun math things.
                    scroll = countdown / 10 + 1;
                }
            }

            if(!KEY_CLEAR)
            {
                // Overwrite the wall tiles with their broken equivelents, few will notice but I
                // think it's an important detail.
                zx0_Decompress(background[3], Get_Tileset_Tile_Ptr(background_ptr, 0, 16));
                zx0_Decompress(floor[3], Get_Tileset_Tile_Ptr(background_ptr, 2, 14));

                delay(500);

                Draw_Dithering(GREY);
                gfx_SwapDraw();
                delay(80);
            }

            scroll = START_SPEED;
        }

        // Keeps track of the time difference between each loop for FPS locking. If this overflows
        // then I've made a huge breakthrough, or badly broken the game.
        uint8_t FPS;

        // Start the real-time clock for FPS tracking
        timer_Enable(1, TIMER_32K, TIMER_0INT, TIMER_UP);

        // Let the game begin.
        while(!KEY_CLEAR && pause_action == 0)
        {
            kb_Scan();

            gfx_Sprite(ceiling[bg_extra_tiles[0]], 0 - tile_offset, 0);
            gfx_Sprite(background[bg_tiles[0]],    0 - tile_offset, 40);
            gfx_Sprite(floor[bg_extra_tiles[0]],   0 - tile_offset, 200);

            // Draw and update the position of the background tiles
            for(uint8_t i = 1; i < sizeof(bg_tiles) - 2; ++i)
            {
                gfx_Sprite_NoClip(ceiling[bg_extra_tiles[i]], (i * 46) - tile_offset, 0);
                gfx_Sprite_NoClip(background[bg_tiles[i]],    (i * 46) - tile_offset, 40);
                gfx_Sprite_NoClip(floor[bg_extra_tiles[i]],   (i * 46) - tile_offset, 200);
            }

            gfx_Sprite(ceiling[bg_extra_tiles[6]], 276 - tile_offset, 0);
            gfx_Sprite(background[bg_tiles[6]],    276 - tile_offset, 40);
            gfx_Sprite(floor[bg_extra_tiles[6]],   276 - tile_offset, 200);

            gfx_Sprite(ceiling[bg_extra_tiles[7]], 322 - tile_offset, 0);
            gfx_Sprite(background[bg_tiles[7]],    322 - tile_offset, 40);
            gfx_Sprite(floor[bg_extra_tiles[7]],   322 - tile_offset, 200);

            tile_offset += scroll;

            savedata.distance += scroll;

            // Scroll the tiles to the left and shift each one in the tile list when they've
            // traveled their width.
            if(tile_offset >= 46)
            {
                tile_offset = tile_offset - 46;

                // If the sprite that's going offscreen is one of the malloc'd ones then free and
                // mark as null.
                if((7 < bg_tiles[0]) && (bg_tiles[0] < 12))
                {
                    free(background[bg_tiles[0]]);
                }
                
                if((5 < bg_extra_tiles[0]) && (bg_extra_tiles[0] < 12))
                {
                    free(ceiling[bg_extra_tiles[0]]);
                    free(floor[bg_extra_tiles[0]]);
                }

                for(uint8_t i = 0; i < sizeof(bg_tiles) - 1; ++i)
                {
                    bg_tiles[i]       = bg_tiles[i + 1];
                    bg_extra_tiles[i] = bg_extra_tiles[i + 1];
                }

                if(bg_tiles[sizeof(bg_tiles) - 1] >= 12)
                {
                    if((bg_tiles[sizeof(bg_tiles) - 1] == 12) || (bg_tiles[sizeof(bg_tiles) - 1] == 14))
                    {
                        ++bg_tiles[sizeof(bg_tiles) - 1];
                    } else {
                        bg_tiles[sizeof(bg_tiles) - 1] = 12 + randInt(0, 1) * 2;
                    }

                    if(bg_extra_tiles[sizeof(bg_extra_tiles) - 1] == 12)
                    {
                        bg_extra_tiles[sizeof(bg_extra_tiles) - 1] = 13;
                    } else {
                        bg_extra_tiles[sizeof(bg_extra_tiles) - 1] = 12;
                    }
                } else {
                    ++bg_tiles[sizeof(bg_tiles) - 1];
                    ++bg_extra_tiles[sizeof(bg_tiles) - 1];
                }

                // Make sure the upcoming tile has a sprite, if not then we need to decompress it
                // with the code from earlier during the intro:
                if(!background[bg_tiles[sizeof(bg_tiles) - 1]])
                {
                    background[bg_tiles[sizeof(bg_tiles) - 1]] = background[bg_tiles[sizeof(bg_tiles) - 1] - 8];
                    zx0_Decompress(background[bg_tiles[sizeof(bg_tiles) - 1]], Get_Tileset_Tile_Ptr(background_ptr, 0, bg_tiles[sizeof(bg_tiles) - 1]));
                }

                if(!ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]])
                {
                    ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]] = ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] - 8];
                    zx0_Decompress(ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]], Get_Tileset_Tile_Ptr(background_ptr, 1, bg_extra_tiles[sizeof(bg_extra_tiles) - 1]));

                    floor[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]] = floor[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] - 8];
                    zx0_Decompress(floor[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]], Get_Tileset_Tile_Ptr(background_ptr, 2, bg_extra_tiles[sizeof(bg_extra_tiles) - 1]));
                }
            }

            // Draw the player sprite
            gfx_TransparentSprite(barry[player.frame / 2], player.x, player.y);

            for(uint8_t i = 0; i < entity_count; ++i)
            {
                Entity_Drawing[ *(uint8_t*)entities[i] ](entities[i]);
            }

            player.frame += player.frame_toggle;

            if((player.frame == 0) || (player.frame == 5))
            {
                player.frame_toggle = -player.frame_toggle;
            }

            if(KEY_2ND && (player.exhaust_frame > 7))
            {
                player.exhaust_frame = 0;
            }

            if(player.frame > 5)
            {
                if(KEY_2ND && player.exhaust_frame == 12)
                {
                    player.exhaust_frame = 0;
                }

                if(player.exhaust_frame < 12)
                {
                    gfx_RLETSprite_NoClip(exhaust[player.exhaust_frame / 2], player.x + randInt(0, 1), player.y + 31);

                    ++player.exhaust_frame;
                }

                if(player.exhaust_frame == 7 && KEY_2ND)
                {
                    player.exhaust_frame = 4;
                }
            }

            if(savedata.health)
            {
                gfx_SetTextFGColor(WHITE);
    
                gfx_SetTextScale(2, 2);
                gfx_SetTextXY(5, 5);
                
                gfx_PrintUInt(savedata.distance / 16, 4);
    
                gfx_SetTextScale(1, 1);

                gfx_PrintStringXY("M", gfx_GetTextX() + 1, 13);
    
                gfx_PrintStringXY("BEST:", 5, 24); gfx_PrintInt(savedata.highscore, 1);
    
                gfx_SetTextFGColor(GOLD);
    
                gfx_SetTextXY(5, 36);
                gfx_PrintUInt(savedata.monies, 3);
            }

            gfx_SetTextXY(290, 10);
            gfx_PrintUInt(FPS, 1);

            /*
            for(uint8_t i = 0; i < 18; ++i) // le funni debug printer
            {
                gfx_SetTextXY(290, 20 + i*10);
                gfx_PrintUInt(entities[i], 1);
            } */

            // Swap the screen and buffer memory regions, faster than copying from buffer -> screen
            gfx_SwapDraw();

            // Barry's controls vs the power of gravity
            if(abs(player.dy) < PLAYER_MAX_SPEED)
            {
                if(KEY_2ND)
                {
                    player.dy -= 1;
                } else {
                    player.dy += 1;
                }
            }

            player.y += player.dy;

            if(player.y >= FLOOR - BARRY_HEIGHT)
            {
                player.y  = FLOOR - BARRY_HEIGHT;
                player.dy = 0;

                if(player.frame == 7)
                {
                    player.frame        = 2;
                    player.frame_toggle = 1;
                }
            } else {
                player.frame = 6;
            }
            
            if(player.y <= CEILING)
            {
                player.y  = CEILING;
                player.dy = 0;
            }

            if(spawn_delay < 0)
            {
                entity_count += Spawn_Stuff(entities, COIN_ID);

                spawn_delay = 600;
            }

            spawn_delay -= scroll;

            // This runs through all entities and does the math for them based on their respective
            // types, which is used as the index for their function in the array.
            for(int8_t i = 0; i < entity_count; ++i)
            {
                if(*(uint8_t*)entities[i] == 0)
                {
                    free(entities[i]);

                    --entity_count;

                    for(uint8_t j = i; j < 30 - 1; ++j)
                    {
                        entities[j] = entities[j + 1];
                    }

                    entities[29] = 0;

                    --i;
                } else {
                    Entity_Action[ *(uint8_t*)entities[i] ](entities[i], scroll);
                }
            }

            // Update the FPS counter, has to use GetSafe since the value changes during reading
            FPS = (CLOCKS_PER_SEC / timer_GetSafe(1, TIMER_UP)); 

            // Delay the game loop to keep it capped it to 24 FPS
            while(timer_GetSafe(1, TIMER_UP) < (CLOCKS_PER_SEC / MAX_FPS))
            {
                // Some kind of memory/entity garbage collecting would work well here or maybe a
                // preemptive decompression system to make sure things are ready in time? zx0 makes
                // so many unattainable goals possible that I'll have to rethink everything. Then
                // again, that's why I'm rewriting from scratch... ah well.
            }

            // Pause menu stuff needs to be done between the FPS calculations
            if(KEY_DEL)
            {   
                pause_action = Do_Pause();
            }

            // Reset the clock safely by shutting it down before zeroing
            timer_Set(1, 0);
        }

        // Update the highscores and monies, would be annoying to lose a primo run after all
        if(savedata.highscore < (savedata.distance / 16))
        {
            savedata.highscore = savedata.distance / 16;
        }
        savedata.college_fund += savedata.monies;

        // This runs through and frees the background sprites in case the game was quit/restarted
        // during the opening. I need to look into optimizing this since it's quite large.
        while(bg_tiles[0] < 12)
        {
            if((7 < bg_tiles[0]) && (bg_tiles[0] < 12))
            {
                free(background[bg_tiles[0]]);
            }
            
            if((5 < bg_extra_tiles[0]) && (bg_extra_tiles[0] < 12))
            {
                free(ceiling[bg_extra_tiles[0]]);
                free(floor[bg_extra_tiles[0]]);
            }

            for(uint8_t i = 0; i < sizeof(bg_tiles) - 1; ++i)
            {
                bg_tiles[i]       = bg_tiles[i + 1];
                bg_extra_tiles[i] = bg_extra_tiles[i + 1];
            }

            ++bg_tiles[sizeof(bg_tiles) - 1];
            ++bg_extra_tiles[sizeof(bg_tiles) - 1];


            if(!background[bg_tiles[sizeof(bg_tiles) - 1]])
            {
                background[bg_tiles[sizeof(bg_tiles) - 1]] = background[bg_tiles[sizeof(bg_tiles) - 1] - 8];
            }

            if(!ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]])
            {
                ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]] = ceiling[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] - 8];

                floor[bg_extra_tiles[sizeof(bg_extra_tiles) - 1]] = floor[bg_extra_tiles[sizeof(bg_extra_tiles) - 1] - 8];
            }
        }

        // Free and wipe any entities left on screen
        for(int8_t i = 0; i < entity_count; ++i)
        {
            if(entities[i])
            {
                free(entities[i]);

                --entity_count;

                for(uint8_t j = i; j < 30 - 1; ++j)
                {
                    entities[j] = entities[j + 1];
                }

                entities[29] = 0;

                --i;
            }
        }
    }

    // Time to free up memory, the thing that kept crashing on either rev L or M calcs originally
    for(uint8_t i = 0; i < sizeof(barry) / sizeof(gfx_sprite_t*); i++)
    {
        free(barry[i]);
    }

    // Stop graphics, aka reset the screen to 16-bit color mode and clean up the buffers for TI-OS
    gfx_End();

    return 1;
}

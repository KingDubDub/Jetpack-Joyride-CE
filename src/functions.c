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

/* --- Globals -- */

//we read APPVAR_VERSION to this var for testing later
uint8_t save_integrity;

//speed of scrolling and time before incrementing it
int8_t scroll_speed;
uint16_t increment_delay;

//measures timings for delays between spawning coins, obstacles, etc.
int24_t spawn_delay;
int24_t missile_delay;

//used for a bad background scroll function that is actually the best for this scenario
int24_t bg_scroll;

//arrays for storing background tileset pointer values to draw
uint8_t bg_list[9];
uint8_t secondary_bg_list[9];

//all important game data, and in one clean struct pointer too
game_data_t save_data;

//the Jetpack Joyride guy's name is Barry Steakfries, which is what I would name my child if I had the desire to marry
//and have children. I'll call him avatar internally so the code is easier to steal :)
avatar_t avatar;

//everything needed to keep track of the points and speed of the seperate jetpack when Barry dies
physics_obj_t jetpack_entity;

//and here's some obstacles and stuff
coin_t coins;
zapper_t zappers;
missile_t missiles;
laser_t lasers;

//debris for the opening scene when the wall explodes
physics_obj_t debris[4];

//a simple framecount delay for the opening screen scrolling scene
uint8_t opening_delay = 138;

//globalized sprite pointers, uses less RAM than managing a bunch of function parameters
gfx_sprite_t     *ceiling_tiles[14];
gfx_sprite_t     *background_tiles[14];
gfx_sprite_t     *floor_tiles[15];
gfx_sprite_t     *button_on_tiles[4];
gfx_sprite_t     *button_off_tiles[4];
gfx_sprite_t     *window;
gfx_rletsprite_t *title;

//takes an input sprite and pastes it into another sprite at given coordinates
void Copy_Pasta(const gfx_sprite_t *sprite_in, gfx_sprite_t *sprite_out, const uint24_t x, const uint8_t y)
{
    const uint24_t width_in = sprite_in->width;
    const uint24_t sprite_in_size = sprite_in->height * width_in;
    const uint24_t width_out = sprite_out->width;
    uint24_t start_write = (width_out * y) + x;

    //write out input sprite row by row into the output sprite
    for(uint24_t j = 0; j < sprite_in_size; j += width_in)
    {
        //copy the row of the input to the position needed in the output
        memcpy(&sprite_out->data[start_write], &sprite_in->data[j], width_in);

        //add the output sprite's width to move to the next row plus the given X that was added at the start
        start_write += width_out;
    }
}

//draws the hallway background
void Draw_Background(void)
{
    //draw the first and last background tiles with clipping since they go offscreen
    gfx_Sprite(ceiling_tiles[secondary_bg_list[0]], 0 - bg_scroll, 0);
    gfx_Sprite(background_tiles[bg_list[0]],        0 - bg_scroll, 40);
    gfx_Sprite(floor_tiles[secondary_bg_list[0]],   0 - bg_scroll, 200);
    
    gfx_Sprite(ceiling_tiles[secondary_bg_list[6]], 276 - bg_scroll, 0);
    gfx_Sprite(background_tiles[bg_list[6]],        276 - bg_scroll, 40);
    gfx_Sprite(floor_tiles[secondary_bg_list[6]],   276 - bg_scroll, 200);

    gfx_Sprite(ceiling_tiles[secondary_bg_list[7]], 322 - bg_scroll, 0);
    gfx_Sprite(background_tiles[bg_list[7]],        322 - bg_scroll, 40);
    gfx_Sprite(floor_tiles[secondary_bg_list[7]],   322 - bg_scroll, 200);

    //this is the best way I've found to draw the backgrounds, smaller and faster than a smart system
    for(uint8_t i = 1; i < 6; ++i)
    {
        //these are drawn all speedy-like without clipping calculations
        gfx_Sprite_NoClip(ceiling_tiles[secondary_bg_list[i]], (i * 46) - bg_scroll, 0);
        gfx_Sprite_NoClip(background_tiles[bg_list[i]], (i * 46) - bg_scroll, 40);
        gfx_Sprite_NoClip(floor_tiles[secondary_bg_list[i]], (i * 46) - bg_scroll, 200);
    }
}

//draws buttons, optimized for program size, not speed, with given button tileset, coords, and width
void Draw_Button(const gfx_sprite_t *first_tile, const gfx_sprite_t *last_tile, const uint16_t x, const uint8_t y, const uint8_t width)
{
    //draw the first button tile (green, yellow, or red in my case)
    gfx_TransparentSprite_NoClip(first_tile, x, y);

    uint16_t middle_start = x + first_tile->width;

    //copy the last column of pixels from the first tile across the given width
    gfx_CopyRectangle(gfx_buffer, gfx_buffer, middle_start - 1, y, middle_start, y, width + 1, first_tile->height);

    //draw the last tile for the button tilesets
    gfx_TransparentSprite_NoClip(last_tile, middle_start + width, y);
}

//a function for drawing the pause menu, saves on stack usage
void Draw_Pause_Buttons(gfx_sprite_t *sprites[], const char *text, uint8_t button_select)
{
    Draw_Button(sprites[button_select], sprites[3], 94, 71 + (button_select * 41), 116);

    //words 'n stuff
    gfx_SetTextFGColor(2);
    gfx_SetTextScale(2, 2);

    //pretty much all the letters are odd numbers of pixels wide or tall, so that sucks
    gfx_PrintStringXY(text, 160 - gfx_GetStringWidth(text)/2, 78 + (button_select * 41));
}

void Draw_MenuWindow(void)
{
    //temporary flipped menu sprite with hardcoded numbers to annoy people
    gfx_sprite_t *flipped_window = gfx_MallocSprite(8, 167);

    gfx_FlipSpriteY(window, flipped_window);

    //I mean, it works and saves space, so why not use it?
    Draw_Button(window, flipped_window, 33, 33, 238);

    //almost forgot this part, that would've been bad
    free(flipped_window);
}

//draws a mesh of horizontal and vertical lines for a pseudo-transparent effect
void Draw_DitheredMesh(uint8_t palette_color)
{
    gfx_SetColor(palette_color);

    //draw the dithered mesh for the janky transparency effect
    for(uint24_t i = randInt(0, 1); i < 320; i += 2)
    {
        gfx_HorizLine(0, i, 320);
        gfx_VertLine(i, 0, 240);
    }
}

//set the background tiles starting at a given point
void Set_Background(const int8_t start)
{
    for(uint8_t i = 0; i < + 9; ++i)
    {
        bg_list[i] = start + i;
        secondary_bg_list[i] = start + i;
    }
}

//this is purely for streamlining, it should only used twice
void Write_Savedata(void)
{
    //create a new appvar, erases the old one and writes the variable data to it
    ti_var_t savegame = ti_Open(DATA_APPVAR, "w");

    //game save data and stats
    ti_Write(&save_data, sizeof(save_data), 1, savegame);

    //this exists to silence a compiler warning that I can't get to shut up
    const uint8_t ver = APPVAR_VERSION;

    //version of the appvar, for future use with updater fix programs and hopefully help with debugging
    ti_Write(&ver, 1, 1, savegame);

    //game environment variables
    ti_Write(&scroll_speed, sizeof(scroll_speed), 1, savegame);
    ti_Write(&increment_delay, sizeof(increment_delay), 1, savegame);
    ti_Write(&bg_scroll, sizeof(bg_scroll), 1, savegame);
    ti_Write(&bg_list, sizeof(bg_list), 1, savegame);
    ti_Write(&secondary_bg_list, sizeof(secondary_bg_list), 1, savegame);
    ti_Write(&spawn_delay, sizeof(spawn_delay), 1, savegame);

    //single pointer to avatar struct
    ti_Write(&avatar, sizeof(avatar), 1, savegame);

    //coin variables
    ti_Write(&coins, sizeof(coins), 1, savegame);

    //zapper variables
    ti_Write(&zappers, sizeof(zappers), 1, savegame);

    //missile variables
    ti_Write(&missiles, sizeof(missiles), 1, savegame);

    //laser variables
    ti_Write(&lasers, sizeof(lasers), 1, savegame);

    ti_Write(&opening_delay, sizeof(opening_delay), 1, savegame);

    //jetpack variables
    ti_Write(&jetpack_entity, sizeof(jetpack_entity), 1, savegame);

    //broken wall stuff
    ti_Write(&debris, sizeof(debris), 1, savegame);

    //uncomment this in the NEAR FINAL release, right now it should a temporary save until I iron out the bugs
    //ti_SetArchiveStatus(true, savegame);

    ti_Close(savegame);
}

//A quick-'n-dirty function for finding an appvar pointer in read mode
void* Get_Appvar_Ptr(const char *appvar)
{
    ti_var_t tmp_slot = ti_Open(appvar, "r");

    void *ptr = ti_GetDataPtr(tmp_slot);
 
    ti_Close(tmp_slot);

    return ptr;
}

//A function for returning the pointers of tileset sprites in appvars with 2-byte LookUp Table (LUT) entries,
//this only works if the appvar only has single images loaded into it, but you can use a single tileset if you
//add 1 to your tiles input, note that the sprite type (normal or RLET) doesn't matter
void* Get_Tile_Ptr(const void *ptr, const uint8_t tile)
{
    // pointer to data + offset of LUT data from start + stored offset at LUT entry "tile"
    return (void*)(ptr + *((uint16_t*)ptr + 1) + *((uint16_t*)ptr + tile + 1));
}

//The exact same thing as the above, but we don't add an extra offset to the tileset LUT; meaning this only works
//for lists of sprites in an appvar (no tilesets). Again, type doesn't matter
void* Get_Sprite_Ptr(const void *ptr, uint8_t tile)
{
    // pointer to data + stored offset at LUT entry "tile"
    return (void*)(ptr + *((uint16_t*)ptr + tile + 1));
}

//Gets the data size of an RLET-encoded sprite, good for allocating memory for flipped copies
uint24_t Get_RLET_Size(const gfx_rletsprite_t *src)
{
    //optimizing for speed here
    uint24_t width  = src->width;
    uint24_t height = src->height;

    uint24_t data_offset = 0;

    for(uint8_t y = 0; y < height; ++y)
    {
        //figure out the row size
        for(uint8_t x = 0; x < width;)
        {
            //grab the transparent run length and increment the size afterwards
            x += src->data[data_offset++];

            //make sure the last transparent run wasn't the whole line
            if(x < width)
            {
                //the next byte is number of non-transparent colors
                x += src->data[data_offset];

                //the next byte is the number of non-transparent colors, add that plus 1 for the next run
                data_offset += (src->data[data_offset] + 1);
            }
        }
    }

    //add the two width and height values
    return data_offset + 2;
}

//calculates what the size of an RLET-encoded sprite would be after flipping it across the Y-axis
uint24_t Get_Vertical_RLET_Size(gfx_rletsprite_t *src)
{
    uint24_t data_offset = 0;

    //the range of possible dest size differences range from -255 to 255
    int24_t dest_difference = 0;

    for(uint8_t y = 0; y < src->height; ++y)
    {
        uint24_t row_start = data_offset;

        //figure out the row size
        for(uint8_t x = 0; x < src->width;)
        {
            x += src->data[data_offset++];

            //make sure the last transparent run wasn't the whole line
            if(x < src->width)
            {
                //the next byte is number of non-transparent colors
                x += src->data[data_offset];

                //the next byte is the number of non-transparent colors, add that plus 1 for the next run
                data_offset += (src->data[data_offset] + 1);

                if((x >= src->width) && (src->data[row_start]))
                {
                    //if the line ends in a solid run but starts with a transparent run, we need to increase the ouput row size
                    //so we can append zero to the start of the dest data
                    ++dest_difference;
                }
            }
            else if(!src->data[row_start])
            {
                //if the line ended with a transparent run but started without one, the dest row needs to be shrunk by one byte
                //since the zero will be replaced with the last transparent run and the data cannot end in zero
                --dest_difference;
            }
        }
    }

    //add the width and height variables size to the data size
    return data_offset + dest_difference + 2;
}

//Takes a RLET-encoded sprite and flips it across the X-axis
void Flip_RLETSpriteX(const gfx_rletsprite_t *src, gfx_rletsprite_t *dest, const uint24_t size)
{
    dest->width  = src->width;
    dest->height = src-> height;

    uint24_t data_offset = 0;

    //the src size has the first 2 bytes for height and width, we only need the data
    uint24_t data_size = size - 2;

    for(uint8_t y = 0; y < src->height; ++y)
    {
        const uint8_t *src_ptr  = &src->data[data_offset];

        //reset the line size (in bytes, not pixels)
        uint8_t row_size = 0;

        //figure out the row size
        for(uint8_t x = 0; x < src->width;)
        {
            //grab the transparent run length and increment the row size afterwards
            x += src_ptr[row_size++];

            //make sure the last transparent run wasn't the whole line
            if(x < src->width)
            {
                //the next byte is number of non-transparent colors
                x += src_ptr[row_size];

                //the next byte is the number of non-transparent colors, add that plus 1 for the next run
                row_size += (src_ptr[row_size] + 1);
            }
        }

        //copy the first row of the src to what will be the last row of the dest
        memcpy(&dest->data[data_size - row_size - data_offset], &src->data[data_offset], row_size);

        data_offset += row_size;
    }
}

//Takes a RLET-encoded sprite and flips it across the Y-axis
void Flip_RLETSpriteY(const gfx_rletsprite_t *src, gfx_rletsprite_t *dest)
{
    dest->width  = src->width;
    dest->height = src-> height;

    uint24_t src_data_offset  = 0;
    uint24_t dest_data_offset = 0;

    for(uint8_t y = 0; y < src->height; ++y)
    {
        uint8_t src_row_size  = 0;
        uint8_t dest_row_size = 0;

        //Why is the compiler like this? Why is this the right way? At least it saves 9 bytes...
        uint8_t *dest_ptr = &dest->data[dest_data_offset];

        //...and this one saves 48 bytes. Commandz is a wise block, I should've tried this earlier.
        const uint8_t *src_ptr  = &src->data[src_data_offset];

        //due to how the data encodes, we need to track if the ouput row size needs to be different than the src size
        bool funky_flag = 0;

        //figure out the row size
        for(uint8_t x = 0; x < src->width;)
        {
            x += src_ptr[src_row_size++];

            //make sure the last transparent run wasn't the whole line
            if(x < src->width)
            {
                //the next byte is number of non-transparent colors
                x += src_ptr[src_row_size];

                //the next byte is the number of non-transparent colors, add that plus 1 for the next run
                src_row_size += (src_ptr[src_row_size] + 1);

                if((x >= src->width) && (src_ptr[0]))
                {
                    //if the line ends in a solid run but starts with a transparent run, we need to increase the ouput row size
                    //so we can append zero to the start of the dest data
                    dest_row_size = src_row_size + 1;

                    funky_flag = true;
                }
            }
            else if(!src_ptr[0])
            {
                //if the line ended with a transparent run but started without one, the dest row needs to be shrunk by one byte
                //since the zero will be replaced with the last transparent run and the data cannot end in zero
                dest_row_size = src_row_size - 1;

                funky_flag = true;
            }
        }

        if(!funky_flag)
        {
            dest_row_size += src_row_size;
        }

        //Oh boy, I sure do love abusing bool types!
        for(int24_t i = 0; i < (src_row_size - funky_flag);)
        {
            uint8_t solid_length;

            //if the first transparent run isn't zero
            if(src_ptr[0])
            {
                dest_ptr[dest_row_size - 1 - i] = src_ptr[i];

                ++i;

                //ensure the whole line wasn't transparent
                if(i < (src_row_size - funky_flag))
                {
                    solid_length = src_ptr[i];
    
                    //set the transparent run length
                    dest_ptr[dest_row_size - 1 - solid_length - i] = solid_length;

                    for(uint24_t j = 0; j < solid_length; ++j)
                    {
                        dest_ptr[dest_row_size - solid_length - i + j] = src_ptr[i + solid_length - j];
                    }

                    //add the length of the copied data to the size count
                    i += solid_length + 1;

                    //if the src row ends with a solid run, we have to tack a zero-transparent run at the start of the dest
                    if(i >= (src_row_size - funky_flag))
                    {
                        dest_ptr[0] = 0;
                    }
                }
            }
            else //else things get weird because we have to read ahead of the src to skip the zero
            {
                solid_length = src_ptr[i + 1];

                dest_ptr[dest_row_size - 1 - solid_length - i] = solid_length;

                for(uint24_t j = 0; j < solid_length; ++j)
                {
                    dest_ptr[dest_row_size - solid_length - i + j] = src_ptr[i + solid_length + 1 - j];
                }

                i += src_ptr[i + 1] + 1;

                //ensure the whole line wasn't solid
                if(i < (src_row_size - funky_flag))
                {
                    dest_ptr[dest_row_size - 1 - i] = src_ptr[i + 1];

                    ++i;

                    //if everything is normal and it ends with a solid run, we need to apend a zero to the dest
                    if((!funky_flag) && (i >= src_row_size))
                    {
                        dest_ptr[0] = 0;
                    }
                }
            }
        }

        src_data_offset  += src_row_size;
        dest_data_offset += dest_row_size;
    }
}

//Draws the opening tiles and the button selector for choosing what to do, handles all menus and popups at the
//beginning, I have to have pointers to the sprites because they're local variables
void Title_Menu(void)
{
    uint8_t selectorY = 5;

    //the selector rectangle is white, so the color has to be 2; why I chose 2 as white is unknown to me
    gfx_SetColor(2);

    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(2);

    //main menuing loop
    do{
        //graphics and input handling for main menu
        do{
            kb_Scan();

            //write the normal hall entries in, but skip the first three
            for (uint8_t i = 3; i < sizeof(bg_list); ++i)
            {
                bg_list[i] = i - 3;
            }
            
            //go back and write the decompressed hall tile index
            for(uint8_t i = 0; i < 3; ++i)
            {
                bg_list[i] = 9 + i;
            }

            Draw_Background();

            if((KEY_DOWN) && !KEY_UP)
            {
                if(selectorY == 110)
                {
                    selectorY = 5;
                } else {
                    selectorY += 35;
                }
            }
            else if((KEY_UP) && !KEY_DOWN)
            {
                if(selectorY == 5)
                {
                    selectorY = 110;
                } else {
                    selectorY -= 35;
                }
            }

            gfx_Rectangle_NoClip(36, selectorY, 98, 31);

            gfx_RLETSprite_NoClip(title, 160, 50);

            gfx_BlitBuffer();

            //wait until none of the arrow keys are pressed
            while((KEY_DOWN || KEY_UP) && !KEY_CLEAR) kb_Scan();
        }
        while(!(KEY_2ND || KEY_ENTER) && !KEY_CLEAR);

        //make sure the 2nd and enter keys aren't still pressed
        while((KEY_2ND || KEY_ENTER) && !KEY_CLEAR) kb_Scan();

        //figures out what to do based on selection with my favorite evaluation technique, and that [clear] check
        //is a big brain play where the key being pressed adds one to selectorY and keeps it from matching anything
        switch(selectorY + (KEY_CLEAR))
        {
            case 40: break; //shop

            case 75: //settings

                /*
                do{
                    kb_Scan();

                    Draw_MenuWindow();

                    gfx_BlitBuffer();

                    //make sure those keys are released
                    while(((KEY_DOWN) || (KEY_UP)) && !KEY_CLEAR) kb_Scan();
                }
                while(!KEY_CLEAR);

                while(KEY_CLEAR) kb_Scan();
                */
            
            break;

            case 110: //about page

                //where to start my rambling monologue about coding and friends and motivation and crap
                ;uint8_t txt_start = 0;
                //the compiler wants me to put a semicolon before the uint, I know not why.

                do{
                    kb_Scan();

                    Draw_MenuWindow();

                    //adjust which twelve lines of text show up based on up and down inputs
                    if((KEY_DOWN) && !KEY_UP && (txt_start < ((sizeof(about_txt) / 3) - 12)))
                    {
                        txt_start += 2;
                    }
                    if((KEY_UP) && !KEY_DOWN && (txt_start > 0))
                    {
                        txt_start -= 2;
                    }

                    for(uint8_t i = 0; i < 12; ++i)
                    {
                        gfx_PrintStringXY(about_txt[i + txt_start], 38, 60 + i*12);
                    }

                    //cover the bit of bottom text that clips out of the window
                    gfx_CopyRectangle(gfx_buffer, gfx_buffer, 38, 195, 39, 195, 240, 5);

                    gfx_BlitBuffer();

                    //make sure those keys are released
                    while(((KEY_DOWN) || (KEY_UP)) && !KEY_CLEAR) kb_Scan();
                }
                while(!KEY_CLEAR);

                while(KEY_CLEAR) kb_Scan();  

            break;
        }
    }
    while(!KEY_CLEAR && (selectorY != 5));
}

int8_t Pause_Menu()
{
    //used by menuing functions to do "stuff"
    int8_t menu_select = 0;
    bool debounced = false;

    //strings used in pause menu
    char pause_options[3][7] = {"QUIT", "RETRY", "RESUME"};

    //make sure the buffer has the contents of the screen
    gfx_BlitScreen();

    gfx_SetColor(1); //black

    Draw_DitheredMesh(1);

    Draw_MenuWindow();

    Draw_Pause_Buttons(button_on_tiles, pause_options[0], 0);
    Draw_Pause_Buttons(button_on_tiles, pause_options[1], 1);
    Draw_Pause_Buttons(button_on_tiles, pause_options[2], 2);

    gfx_SetColor(2);

    //draw initial selector position
    gfx_Rectangle(93, 70, 134, 32);

    //menu control loop
    while (!KEY_CLEAR)
    {
        kb_Scan();

        if((KEY_DOWN) || (KEY_UP))
        {
            debounced = false;

            //erase the old selection rectangle as soon as any key updates are made that change the selection
            gfx_SetColor(6); //grey-blue
            gfx_Rectangle(93, 70 + (menu_select * 41), 134, 32);

            //re-draw the unpressed form of whatever button is being moved on from
            Draw_Pause_Buttons(button_on_tiles, pause_options[menu_select], menu_select);

            //if down is pressed, add to menu select and correct overflow when necessary
            if(KEY_DOWN)
            {
                if(menu_select > 1)
                {
                    menu_select = 0;
                } else {
                    ++menu_select;
                }
            }

            //and if up is pressed, subtract from menu select and correct overflow
            if(KEY_UP)
            {
                if(menu_select < 1)
                {
                    menu_select = 2;
                } else {
                    --menu_select;
                }
            }

            //draw the new selection rectangle
            gfx_SetColor(2); //white
            gfx_Rectangle(93, 70 + (menu_select * 41), 134, 32);
        }
        else if((KEY_2ND) || (KEY_ENTER))
        {
            debounced = true;

            gfx_SetColor(6); //grey-blue

            //edges of larger buttons need to be hidden for smaller buttons
            gfx_FillRectangle(93, 70 + (menu_select * 41), 134, 32);

            Draw_Button(button_off_tiles[menu_select], button_off_tiles[3], 96, 73 + (menu_select * 41), 112);

            //smol selector
            gfx_SetColor(2); //white
            gfx_Rectangle(95, 72 + (menu_select * 41), 130, 28);

            //button text
            gfx_PrintStringXY(pause_options[menu_select], 160 - gfx_GetStringWidth(pause_options[menu_select])/2, 78 + (menu_select * 41));
        }
        else
        {
            if(debounced)
            {
                break;
            }
        }

        gfx_BlitBuffer();

        //simple way to wait until none of the arrow keys are pressed
        while(((KEY_DOWN) || (KEY_UP)) && !KEY_CLEAR) kb_Scan();
    }

    return menu_select;
};

//stuff for the death screen and runs the menu for returning to the title or retrying
uint8_t Ded_Menu(void)
{
    //make sure that whatever's on the screen is the same as what's in the buffer
    gfx_BlitScreen();

    gfx_SetTextScale(1, 1);

    //there isn't any way to measure the pixel width of a integer with the gfx libs and sprintf is really bad,
    //but we can measure the pixel displacement so I can just draw it out and then cover it up with the menu
    gfx_SetTextXY(146, 30);
    gfx_PrintUInt(save_data.distance / 15, 1);

    //store the length of the distance number-string (with 3x scale) plus the length of the M minus three transparent pixels
    uint16_t distance_width = 3 * (gfx_GetTextX() - 146) + 16 - 3;

    gfx_SetTextXY(146, 30);
    gfx_PrintUInt(save_data.monies, 1);

    //store the lengths of the money number-string (at 2x scale) minus two transparent pixels
    uint16_t money_width = 2 * (gfx_GetTextX() - 146) - 2;

    gfx_sprite_t *flipped_window = gfx_MallocSprite(8, 167);

    Draw_Button(window, gfx_FlipSpriteY(window, flipped_window), 146, 10, 132);

    free(flipped_window);

    //Why no, this code is beautiful and not janky at all.

    gfx_SetTextFGColor(2); //white

    //I just took the current christmas date and multiplied it's values: 25 * 12 * 2021 = 606300
    //If there's a funny Cemetech number out there then I need to use that instead; I doubt anyone
    //will ever find this, and if they do I doubt they'll talk about it.
    if(randInt(0, 606300) || ((save_data.distance / 15) >= save_data.highscore))
    {
        gfx_SetTextScale(1, 1);

        gfx_PrintStringXY("YOU FLEW", 191, 40);
        gfx_PrintStringXY("AND COLLECTED", 173, 100);
        gfx_PrintStringXY("COINS:", 154, 134);

        gfx_SetTextFGColor(4); //gold
        gfx_SetTextScale(3, 3);

        //draw distance centered on the menu box
        gfx_SetTextXY(217 - distance_width/2, 60);
        gfx_PrintUInt(save_data.distance / 15, 1);

        //add the meters symbol
        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY("M", gfx_GetTextX(), 68);

        //draw coin count
        gfx_SetTextXY(218 - money_width/2, 130);
        gfx_PrintUInt(save_data.monies, 1);
    }
    else
    {
        //note that this doesn't show up if you set a new high-score, that'd be annoying

        gfx_SetColor(1); //black
        gfx_FillRectangle_NoClip(150, 34, 140, 139);

        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY("U IS DED.", 168, 55);

        gfx_SetTextScale(5, 5);
        gfx_PrintStringXY("LOL.", 155, 100);

        //OH LOOK ANOTHER FUNNY HAHA JOJOKE
        gfx_SetTextScale(1, 1);
        gfx_SetTextXY(216, 153);
        gfx_PrintChar(15);
    }

    gfx_SetTextScale(2, 2);
    gfx_SetTextFGColor(2); //white

    //draw the retry and quit buttons
    Draw_Button(button_on_tiles[2], button_on_tiles[3], 132, 180, 70);
    gfx_PrintStringXY("RETRY", 142, 187);

    Draw_Button(button_on_tiles[0], button_on_tiles[3], 222, 180, 70);
    gfx_PrintStringXY("QUIT", 243, 187);

    //draw the initial selector rectangle
    gfx_SetColor(2); //white
    gfx_Rectangle_NoClip(132, 180, 76, 30);

    gfx_BlitBuffer();

    while(kb_AnyKey()) kb_Scan();

    //oh joy, I get to do debouncing checks again
    uint8_t selector_x           = 132;
    bool    previous_2nd_state   = false;
    bool    previous_arrow_state = false;

    //run until [2nd] is pressed and not the arrow keys or until [clear] is pressed
    do
    {
        kb_Scan();

        //update the previous button states for debouncing
        if(KEY_2ND) previous_2nd_state = true;

        if(kb_Data[7])
        {
            previous_2nd_state = false;
        }

        //hitting any of the arrow keys will toggle the selector, but it has to be released to do it again
        if(kb_Data[7] && !previous_arrow_state)
        {
            Draw_Button(button_on_tiles[(222 - selector_x) / 45], button_on_tiles[3], selector_x, 180, 70);

            if(selector_x == 132)
            {
                selector_x = 222;
            } else {
                selector_x = 132;
            }

            previous_arrow_state = true;
        }
        else if(!kb_Data[7])
        {
            previous_arrow_state = false;
        }
        
        if(previous_2nd_state)
        {
            Draw_Button(button_off_tiles[(222 - selector_x) / 45], button_off_tiles[3], selector_x + 2, 182, 66);
            gfx_Rectangle_NoClip(selector_x + 1, 181, 84, 28);
        }
        else
        {
            Draw_Button(button_on_tiles[(222 - selector_x) / 45],  button_on_tiles[3],  selector_x,     180, 70);
            gfx_Rectangle_NoClip(selector_x, 180, 86, 30);
        }

        gfx_PrintStringXY("RETRY", 142, 187);
        gfx_PrintStringXY("QUIT", 243, 187);

        gfx_BlitBuffer();
    }
    while(!(previous_2nd_state && !KEY_2ND && !kb_Data[7]) && !KEY_CLEAR);

    if(KEY_CLEAR)
    {
        //exit game
        return 2;
    }
    else
    {
        //go to menu or 
        return ((selector_x - 130) / 92);
    }
}
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>

#include "headers.h"



// Get the pointer to an appvar in read mode
void* Get_Appvar_Ptr(const char *appvar)
{
    ti_var_t tmp_slot = ti_Open(appvar, "r");

    void *ptr = ti_GetDataPtr(tmp_slot);
 
    ti_Close(tmp_slot);

    return ptr;
}

// Returns the pointer to a sprite in an appvar with 2-byte LUT entries given the pointer and zero-
// indexed sprite index (this is based on the order of the images in your convimg.yaml)
void* Get_Tile_Ptr(const void *ptr, const uint8_t tile)
{
    // pointer to data + offset of LUT data from start + stored offset at LUT entry "tile"
    return (void*)(ptr + *((uint16_t*)ptr + tile + 1));
}

/* For posterity (me) let's discuss how LookUp Table entries are stored for tileset appvars:
The first value is the total number of LUT entries, which are all either 2 or 3 bytes depending on
your convimg settings. The next set will be an ascending list with the offet in *bytes* of each
tileset in single bytes irregardless of convimg. After this will be a number of ascending lists of
tile offsets in a tileset that should be added to their corresponding tileset offset. The equation
for finding the offset is: appvar pointer + tileset offset + tile offset. If an appvar has multiple
tilesets and I want to pull the 5th tile from the 2nd tileset, then I'd add the 2nd value in the
first list and the 5th value in the 3rd list to the pointer of the appvar itself. This is annoying
to sort out but can be done by finding the points where a value is less than the previous to find
each list of offsets.                                                                            */

// Returns the pointer to a tile of an appvar given the appvar tile, the tileset in the list, and
// the tile itself. Only works with 2-byte LUT entries. Tileset and tile values are zero-indexed.
void* Get_Tileset_Tile_Ptr(const void *ptr, const uint8_t tileset, const uint8_t tile)
{
    uint24_t tile_index;

    for(uint8_t i = 1, x = 0; x <= tileset; ++i)
    {
        if( *((uint16_t*)ptr + i) >= *((uint16_t*)ptr + 1 + i))
        {
            ++x;
            tile_index = i + 1;
        }
    }

    return (void*)(ptr + *((uint16_t*)ptr + 1 + tileset) + *((uint16_t*)ptr + tile_index + tile));
}

// A simple function for drawing the left and right edges of a sprite and "stretches" the left
// sprite between the sides. Saves a lot of memory and is used for menus and buttons of varying
// widths. Takes the left and right edge sprites, the top left coordinates, and the width of the
// space between them.
void Draw_StretchedSprite(const gfx_sprite_t *left_spr, const gfx_sprite_t *right_spr, const uint16_t x, const uint8_t y, const uint8_t width)
{
    gfx_TransparentSprite_NoClip(left_spr, x, y);

    // Copy the last column of the left sprite across the width, exploits how writes to a column
    // before reading that same column to copy to the next space.
    gfx_CopyRectangle(gfx_buffer, gfx_buffer, x + left_spr->width - 1, y, x + left_spr->width, y, width + 1, left_spr->height);

    gfx_TransparentSprite_NoClip(right_spr, x + left_spr->width + width, y);
}

gfx_sprite_t *window;
gfx_sprite_t *window_flipped;

// Create a mesh of colored lines for a poor-man's transparency filter.
void Draw_Dithering(const uint8_t palette_entry)
{
    gfx_SetColor(palette_entry);

    for(uint24_t i = 0; i < 320; i += 2)
    {
        gfx_HorizLine(0, i, 320);
        gfx_VertLine( i, 0, 240);
    }
}

// Draws a button using the Stretched_Sprite routine and puts centered text over it
void Draw_Button(const gfx_sprite_t *left_spr, const gfx_sprite_t *right_spr, const uint16_t x, const uint8_t y, const uint8_t width, const char *text)
{
    Draw_StretchedSprite(left_spr, right_spr, x, y, width);

    gfx_PrintStringXY(text, x + left_spr->width + (width - gfx_GetStringWidth(text)) / 2, y + left_spr->height / 2 - 8);
}

gfx_sprite_t *button[4];
gfx_sprite_t *select_button[4];

// Runs the basic menuing for the pause screen, assumes the window has already been drawn
uint8_t Do_Pause(void)
{
    // Because of swapdraw we're drawing to the last frame and not the one onscreen, so
    // we need to copy it to the buffer real quick before throwing up the menu.
    gfx_BlitScreen();

    Draw_Dithering(BLACK);

    Draw_StretchedSprite(window, window_flipped, 53, 33, 198);

    gfx_SetTextFGColor(2);
    gfx_SetTextScale(2, 2);

    char text[3][7] = {"RESUME", "RETRY", "QUIT"};

    for(uint8_t i = 0; i < 3; ++i)
    {
        Draw_StretchedSprite(button[i], button[3], 94, 71 + (i * 41), 116);

        gfx_PrintStringXY(text[i], 160 - gfx_GetStringWidth(text[i]) / 2, 78 + (i * 41));
    }

    gfx_SetColor(WHITE);
    gfx_Rectangle_NoClip(93, 70, 134, 32);

    gfx_BlitBuffer();

    while(KEY_DEL) kb_Scan();

    uint8_t action_select = 0;
    bool debounced = false;

    while(!KEY_DEL && !KEY_CLEAR && (KEY_2ND || !debounced))
    {
        Draw_Button(button[action_select], button[3], 94, 71 + (action_select * 41), 116, text[action_select]);
        gfx_SetColor(NAVY);
        gfx_Rectangle_NoClip(93, 70 + (action_select * 41), 134, 32);

        if(KEY_UP)
        {
            if(action_select == 0)
            {
                action_select = 2;
            } else {
                --action_select;
            }
        }
        
        if(KEY_DOWN)
        {
            if(action_select == 2)
            {
                action_select = 0;
            } else {
                ++action_select;
            }
        }

        if(!KEY_2ND)
        {
            Draw_Button(button[action_select], button[3], 94, 71 + (action_select * 41), 116, text[action_select]);

            gfx_SetColor(WHITE);
            gfx_Rectangle_NoClip(93, 70 + (action_select * 41), 134, 32);
        } else {

            gfx_SetColor(NAVY);
            gfx_Rectangle_NoClip(94, 71 + (action_select * 41), 132, 30);
            gfx_SetColor(WHITE);
            gfx_Rectangle_NoClip(95, 72 + (action_select * 41), 130, 28);

            Draw_Button(select_button[action_select], select_button[3], 96, 73 + (action_select * 41), 112, text[action_select]);
            
            debounced = true;
        }

        gfx_BlitBuffer();

        while(kb_Data[7]) kb_Scan();

        kb_Scan();
    }

    while(KEY_DEL) kb_Scan();

    return action_select;
}
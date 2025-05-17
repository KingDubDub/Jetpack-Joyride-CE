
// Prevent multiple accidental includes
#ifndef HEADERS_H
#define HEADERS_H

#include <graphx.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* --- Defines --- */

    #define KEY_2ND   (kb_Data[1] & kb_2nd)
    #define KEY_DEL   (kb_Data[1] & kb_Del)
    #define KEY_ENTER (kb_Data[6] & kb_Enter)
    #define KEY_CLEAR (kb_Data[6] & kb_Clear)
    #define KEY_DOWN  (kb_Data[7] & kb_Down)
    #define KEY_LEFT  (kb_Data[7] * kb_Left)
    #define KEY_RIGHT (kb_Data[7] & kb_Right)
    #define KEY_UP    (kb_Data[7] & kb_Up)

    #define TRANSPARENT 0 // RGB: (255, 0,   128)
    #define BLACK       1 // RGB: (0,   0,   0)
    #define WHITE       2 // RGB: (255, 255, 255)
    #define GREY        3 // RGB: (200, 200, 200)
    #define GOLD        4 // RGB: (40,  48,  73)
    #define RED         5 // RGB: (249, 0,   0)
    #define NAVY        6 // RGB: (40,  48,  73)

    #define MAX_FPS 24

    #define START_SPEED      6
    #define MAX_SPEED        12
    #define PLAYER_MAX_SPEED 12

    #define CEILING 20
    #define FLOOR   222

    #define BARRY_WIDTH  22
    #define BARRY_HEIGHT 36

    // These are entity type IDs for function array magic, I'll elaborate later
    #define NULL_ID    NULL // Redundant much?
    #define BARRY_ID   1
    #define PHYSOBJ_ID 2
    #define ZAPPER_ID  3
    #define MISSILE_ID 4
    #define LASER_ID   5
    #define COIN_ID    6

    /* --- Types --- */
    
    typedef struct
    {
        uint8_t id;
        int24_t x;
        uint8_t y;
        int8_t  dy;
        uint8_t theta;
        uint8_t frame;
        int8_t  frame_toggle;
        uint8_t exhaust_frame;
        uint8_t bounces;
    }
    avatar_t;

    /* --- Externs --- */

    extern const unsigned char jetpackia[];
    extern const uint8_t jetpackia_spacing[];

    extern gfx_sprite_t *window;
    extern gfx_sprite_t *window_flipped;

    extern gfx_sprite_t *button[4];
    extern gfx_sprite_t *select_button[4];

    extern const char *about_txt[];

    /* --- Functions --- */

    void*    Get_Appvar_Ptr(const char *appvar);

    void*    Get_Tile_Ptr(const void *ptr, const uint8_t tile);

    void*    Get_Tileset_Tile_Ptr(const void *ptr, const uint8_t tileset, const uint8_t tile);

    void     Draw_StretchedSprite(const gfx_sprite_t *left_spr, const gfx_sprite_t *right_spr, const uint16_t x, const uint8_t y, const uint8_t width);

    void     Draw_Dithering(const uint8_t palette_entry);

    void     Draw_Button(const gfx_sprite_t *left_spr, const gfx_sprite_t *right_spr, const uint16_t x, const uint8_t y, const uint8_t width, const char *text);

    uint8_t  Do_Pause(void);

#ifdef __cplusplus
}
#endif

#endif
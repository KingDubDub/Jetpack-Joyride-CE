//prevent multiple accidental includes:
#ifndef HEADERS_H
#define HEADERS_H

#include "gfx/jetpackia.h"

#include "constants.h"

//compatibility with C++ compilers, covers all functions in the block:
#ifdef __cplusplus
extern "C" {
#endif

    //all the externs from the main program (they don't need to be clean, who looks in here anyway?):
    extern game_data_t save_data;
    extern uint8_t     save_integrity;
    extern int8_t      scroll_speed;
    extern uint16_t    increment_delay;
    extern int24_t     spawn_delay;
    extern int24_t     missile_delay;
    extern int24_t     bg_scroll;
    extern uint8_t     bg_list[9];
    extern uint8_t     secondary_bg_list[9];
    extern avatar_t    avatar;
    extern jetpack_t   jetpack_entity;
    extern coin_t      coins;
    extern zapper_t    zappers;
    extern missile_t   missiles;
    extern laser_t     lasers;
    extern uint8_t     opening_delay;

    //gfx_CheckRectangleHotspot but only the Y checks, since 4 C++ masters write better code than me:
    #define Yspot(master_y, master_height, test_y, test_height) \
        (((test_y) < ((master_y) + (master_height))) && \
        (((test_y) + (test_height)) > (master_y)))

    //Mmm smells like stolen code... adapted from StackOverflow, fills an array of a given size with a given value very quickly:
    #define Fill_Array(array, entries, value) \
    do{ \
        size_t n = entries; \
        for(size_t i = 0; i < n; ++i) (array)[i] = value; \
    } while (0);
    //NOTE: initial tests saw smaller size than memset and equivalent speed. Further testing will be done when I feel like it.

    void  copy_pasta(const gfx_sprite_t *spriteIn, gfx_sprite_t *spriteOut, uint24_t x, uint8_t y);

    void  draw_button(gfx_sprite_t *sprites[], char *text, uint8_t buttonSelect);

    void  set_background(const uint8_t start);

    void  save_state(void);

    void* get_appvar_ptr(const char *appvar);

    void* get_tile_ptr(const void *ptr, uint8_t tile);

    void* get_sprite_ptr(const void *ptr, uint8_t tile);

    void  title_menu(gfx_sprite_t *ceiling[], gfx_sprite_t *background[], gfx_sprite_t *floor[], gfx_sprite_t *menusprite);

    void  ded_menu(gfx_sprite_t *menusprite);

#ifdef __cplusplus
}
#endif

#endif
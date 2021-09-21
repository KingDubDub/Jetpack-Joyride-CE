
//prevent multiple accidental includes:
#ifndef HEADERS_H
#define HEADERS_H

#include "gfx/jetpackia.h"

#include "constants.h"

//compatibility with C++ compilers, covers all functions in the block:
#ifdef __cplusplus
extern "C" {
#endif

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

    void CopyPasta(const gfx_sprite_t *spriteIn, gfx_sprite_t *spriteOut, uint24_t x, uint8_t y);

    void draw_button(gfx_sprite_t *sprites[], char *text, uint8_t buttonSelect);

    void Set_Background(const uint8_t start);

    void save_state(void);

    void* GetTile_Ptr(void *ptr, uint8_t tile);

    void* GetSprite_Ptr(void *ptr, uint8_t tile);

    void TitleMenu(gfx_sprite_t *ceiling[], gfx_sprite_t *background[], gfx_sprite_t *floor[], gfx_sprite_t *menusprite);

    void ded_menu(gfx_sprite_t *menusprite);

#ifdef __cplusplus
}
#endif

#endif
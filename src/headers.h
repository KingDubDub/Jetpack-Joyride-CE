
//prevent multiple accidental includes:
#ifndef HEADERS_H
#define HEADERS_H

#include <graphx.h>

//my crappy custom font that might be good, but I'm unsatisfied with it's current form:
#include "gfx/jetpackia.h"

//compatibility with C++ compilers, covers all functions in the block:
#ifdef __cplusplus
extern "C" {
#endif

    /* --- Defines --- */

    //just some little macros to speed up debugging:
    #define KEY_2ND   (kb_Data[1] & kb_2nd)
    #define KEY_DEL   (kb_Data[1] & kb_Del)
    #define KEY_ENTER (kb_Data[6] & kb_Enter)
    #define KEY_CLEAR (kb_Data[6] & kb_Clear)
    #define KEY_DOWN  (kb_Data[7] & kb_Down)
    #define KEY_LEFT  (kb_Data[7] * kb_Left)
    #define KEY_RIGHT (kb_Data[7] & kb_Right)
    #define KEY_UP    (kb_Data[7] & kb_Up)

    #define DATA_APPVAR    "JTPKDAT"
    #define APPVAR_VERSION 4

    //HEY FUTURE ME, REMEMBER TO UPDATE THE VERSION WHEN WE ADD STUFF!
    //SCREW YOU PAST-FUTURE ME, hopefully future-future me does better.
    //No, we definitely did not.

    #define START_SPEED 6
    #define MAX_SPEED   12

    #define CEILING 20
    #define FLOOR   185

    //the starting X-coords for various obstacles and things:
    #define COIN_ORIGIN    330
    #define MAX_COINS 30
    #define COIN_FORMATIONS 6

    #define ZAPPER_ORIGIN  352
    #define MAX_ZAPPERS  3

    #define MISSILE_ORIGIN 1466
    #define MAX_MISSILES 1

    #define MAX_LASERS   7
    #define LASER_FORMATIONS 5

    //rotation speeds:
    #define BARRY_DEFLECTION   5
    #define JETPACK_DEFLECTION 5

    /* --- Types --- */

    typedef struct
    {
        uint8_t health;       //the number of hits Barry can take (increased with vehicles and shields)
        uint32_t monies;      //money collected in the run
        uint32_t college_fund; //total money collected from all runs, can be used to purchase stuff (when can I add this?)
        uint32_t distance;    //distance travelled in the run, measured in pixels
        uint32_t highscore;   //highest distance travelled in all runs
    }
    game_data_t;

    //all the data used to control Barry's position, acceleration, etc. are consolidated under one pointer.
    //x and y positions, time 2nd is pressed/released to fly/fall, animation frame, animation cycling control,
    //and jetpack animation frame:
    typedef struct
    {
        int24_t x;
        uint8_t y;
        uint8_t theta; //in calculator land, there're only 256 degrees, not 360 oogabooga degrees.
        int8_t  input_duration;
        uint8_t player_animation;
        uint8_t player_animation_toggle;
        uint8_t exhaust_animation;
        uint8_t corpse_bounce;
        uint8_t death_delay;
    }
    avatar_t; //looks gross, but it saves 309 bytes in read/write calls to the appvar. It's staying.

    typedef struct
    {
        int24_t x;
        uint8_t y;
        int8_t  h_accel; //horizontal acceleration
        int8_t  v_accel; //vertical acceleration
        uint8_t theta;
        uint8_t bounce;
    }
    jetpack_t;

    //hey looky, a coin struct type for easy use, WHY DIDN'T I MAKE THIS EARLIER?
    //stores x and y positions, along with the coin's animation frame:
    typedef struct
    {
        int24_t x[30];
        uint8_t y[30];
        uint8_t animation[30];

        //coin formation variable to keep track of coin list shapes:
        uint8_t formation;
    }
    coin_t;

    //again, WHY did I not make these before... I think there was a reason...
    //laser x and y positions, length, and animation frame:
    typedef struct
    {
        int24_t x[MAX_ZAPPERS];
        uint8_t y[MAX_ZAPPERS];
        int8_t length[MAX_ZAPPERS];

        //zapper type, vertical, horizontal, or positve/negative diagonal:
        int8_t orientation[MAX_ZAPPERS];

        //zapper sprite animation frame:
        int8_t animate;
    }
    zapper_t;

    //I believe the ZDS toolchain broke them and doubled the program size, praise God I upgraded to LLVM.
    //missile x and y positions:
    typedef struct
    {
        int24_t x[MAX_MISSILES];
        uint8_t y[MAX_MISSILES];

        //keep track of animations for missiles:
        int8_t icon_animate;
        int8_t animation;
        int8_t animation_toggle;
    }
    missile_t;

    //the point is, updates are good; ergo, Windows is exciting
    //y position (x position is shared among all lasers), laser's time to function:
    typedef struct
    {
        uint8_t y[7];
        uint24_t lifetime[7];

        //all lasers have a universal X that doesn't move very far:
        int8_t x;
        //laser animation sprite:
        int8_t animation;
        //which laser formation is being used:
        uint8_t formation;
        //keep track of how many lasers have fired already:
        uint8_t deactivated;
    }
    laser_t;

    /* --- Externs --- */

    extern       game_data_t save_data;
    extern       uint8_t     save_integrity;
    extern       int8_t      scroll_speed;
    extern       uint16_t    increment_delay;
    extern       int24_t     spawn_delay;
    extern       int24_t     missile_delay;
    extern       int24_t     bg_scroll;
    extern       uint8_t     bg_list[9];
    extern       uint8_t     secondary_bg_list[9];
    extern       avatar_t    avatar;
    extern       jetpack_t   jetpack_entity;
    extern       coin_t      coins;
    extern       zapper_t    zappers;
    extern       missile_t   missiles;
    extern       laser_t     lasers;
    extern       uint8_t     opening_delay;
    extern const uint8_t     coin_max[];
    extern const uint24_t    coin_form_x[COIN_FORMATIONS][MAX_COINS];
    extern const uint8_t     coin_form_y[COIN_FORMATIONS][MAX_COINS];
    extern const uint8_t     formation_max_lasers[LASER_FORMATIONS];
    extern const uint8_t     laser_y[LASER_FORMATIONS][MAX_LASERS];
    extern const uint8_t     half_life[LASER_FORMATIONS][MAX_LASERS];
    extern const char        *about_txt[36];

    //pointers to the background sprites:
    extern gfx_sprite_t *ceiling_tiles[14];
    extern gfx_sprite_t *background_tiles[18];
    extern gfx_sprite_t *floor_tiles[14];

    //globalizing these saves about ~200 bytes, so there is a good reason for this:
    extern gfx_sprite_t     *button_on_tiles[4];
    extern gfx_sprite_t     *button_off_tiles[4];
    extern gfx_sprite_t     *window;
    extern gfx_rletsprite_t *title;

    /* --- Functions --- */

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

    void  draw_background(void);

    void  draw_button(const gfx_sprite_t *first_tile, const gfx_sprite_t *last_tile, const uint16_t x, const uint8_t y, const uint8_t width);

    void  draw_pause_buttons(gfx_sprite_t *sprites[], const char *text, uint8_t buttonSelect);

    void  set_background(const uint8_t start);

    void  save_state(void);

    void* get_appvar_ptr(const char *appvar);

    void* get_tile_ptr(const void *ptr, const uint8_t tile);

    void* get_sprite_ptr(const void *ptr, uint8_t tile);

    void  title_menu(void);

    uint8_t  ded_menu(void);

#ifdef __cplusplus
}
#endif

#endif
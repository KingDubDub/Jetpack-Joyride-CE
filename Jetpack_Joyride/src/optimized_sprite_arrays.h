#include "sprites/missile_0.c"
#include "sprites/missile_1.c"
#include "sprites/missile_2.c"
#include "sprites/missile_3.c"
#include "sprites/missile_4.c"
#include "sprites/missile_5.c"
#include "sprites/missile_6.c"

extern unsigned char missile_0_data[1682];
extern unsigned char missile_1_data[1817];
extern unsigned char missile_2_data[1707];
extern unsigned char missile_3_data[1460];
extern unsigned char missile_4_data[1460];
extern unsigned char missile_5_data[1325];
extern unsigned char missile_6_data[1452];

unsigned char *missile_sprites_data[7] =
{
    missile_0_data,
    missile_1_data,
    missile_2_data,
    missile_3_data,
    missile_4_data,
    missile_5_data,
    missile_6_data
};

#define missile_tiles ((gfx_sprite_t**)missile_sprites_data)

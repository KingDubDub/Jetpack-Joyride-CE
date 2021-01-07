/*

    Custom Sprite Tileset Array Header

Saves ~2600 bytes alone on just the missiles, others save a few hundred (exact value has yet to be calculated).
This is a dramatic enough cut that it looses ~60 bytes compressed even with a .28% compression drop.

Every byte is precious. This is not a waste of time.

*/

//custom exhaust array:
extern unsigned char exhaust0_data[145];
extern unsigned char exhaust1_data[189];
extern unsigned char exhaust2_data[211];
extern unsigned char exhaust3_data[222];
extern unsigned char exhaust4_data[255];
extern unsigned char exhaust5_data[232];


unsigned char *exhaust_tiles_data[6] =
{
    exhaust0_data,
    exhaust1_data,
    exhaust2_data,
    exhaust3_data,
    exhaust4_data,
    exhaust5_data,
};

#define exhaust_tiles_optimized ((gfx_sprite_t**)exhaust_tiles_data)

//custom coin tiles array:
extern unsigned char coin0_data[146];
extern unsigned char coin1_data[134];
extern unsigned char coin2_data[98];
extern unsigned char coin3_data[134];

unsigned char *coin_tiles_data[4] =
{
    coin0_data,
    coin1_data,
    coin2_data,
    coin3_data
};

#define coin_tiles_optimized ((gfx_sprite_t**)coin_tiles_data)

//custom missile tilesets:
extern unsigned char missile0_data[1682];
extern unsigned char missile1_data[1817];
extern unsigned char missile2_data[1707];
extern unsigned char missile3_data[1460];
extern unsigned char missile4_data[1460];
extern unsigned char missile5_data[1325];
extern unsigned char missile6_data[1452];

unsigned char *missile_tiles_data[7] =
{
    missile0_data,
    missile1_data,
    missile2_data,
    missile3_data,
    missile4_data,
    missile5_data,
    missile6_data
};

#define missile_tiles_optimized ((gfx_sprite_t**)missile_tiles_data)
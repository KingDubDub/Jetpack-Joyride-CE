/*

    Custom Sprite Tileset Array Header

Saves ~2600 bytes alone on just the missiles, others save a few hundred (exact value has yet to be calculated).
This is a dramatic enough cut that it looses ~60 bytes compressed even with a .28% compression drop.

Every byte is precious. This is not a waste of time.

*/

//custom barry sprite array:
extern unsigned char barry0_data[];
extern unsigned char barry1_data[];
extern unsigned char barry2_data[];
extern unsigned char barry3_data[];

unsigned char *barryRun_array_data[4] =
{
    barry0_data,
    barry1_data,
    barry2_data,
    barry3_data
};

#define barryRun_array ((gfx_sprite_t**)barryRun_array_data)

//custom array for when Barry is hit:
extern unsigned char barryHit0_data[];
extern unsigned char barryHit1_data[];
extern unsigned char barryHit2_data[];

unsigned char *barryHit_array_data[3] =
{
    barryHit0_data,
    barryHit1_data,
    barryHit2_data
};

#define barryHit_array_optimized ((gfx_sprite_t**)barryHit_array_data)

//Barry's dead corpse that slides on the ground:
extern unsigned char barryDed0_data[];
extern unsigned char barryDed1_data[];
extern unsigned char barryDed2_data[];

unsigned char *barryDed_array_data[3] =
{
    barryDed0_data,
    barryDed1_data,
    barryDed2_data
};

#define barryDed_array_optimized ((gfx_sprite_t**)barryDed_array_data)

//custom exhaust array:
extern unsigned char exhaust0_data[];
extern unsigned char exhaust1_data[];
extern unsigned char exhaust2_data[];
extern unsigned char exhaust3_data[];
extern unsigned char exhaust4_data[];
extern unsigned char exhaust5_data[];


unsigned char *exhaust_array_data[6] =
{
    exhaust0_data,
    exhaust1_data,
    exhaust2_data,
    exhaust3_data,
    exhaust4_data,
    exhaust5_data
};

#define exhaust_array_optimized ((gfx_sprite_t**)exhaust_array_data)

//custom coin tiles array:
extern unsigned char coin0_data[];
extern unsigned char coin1_data[];
extern unsigned char coin2_data[];
extern unsigned char coin3_data[];

unsigned char *coin_array_data[4] =
{
    coin0_data,
    coin1_data,
    coin2_data,
    coin3_data
};

#define coin_array_optimized ((gfx_sprite_t**)coin_array_data)

//this is (currently) unecessary but I'm making zapppers like this for some later ideas:
extern unsigned char zapper0_data[];
extern unsigned char zapper1_data[];
extern unsigned char zapper2_data[];
extern unsigned char zapper3_data[];

unsigned char *zapper_array_data[] =
{
    zapper0_data,
    zapper1_data,
    zapper2_data,
    zapper3_data
};

#define zapper_array_optimized ((gfx_sprite_t**)zapper_array_data)

//custom missile tilesets:
extern unsigned char missile0_data[];
extern unsigned char missile1_data[];
extern unsigned char missile2_data[];
extern unsigned char missile3_data[];
extern unsigned char missile4_data[];
extern unsigned char missile5_data[];
extern unsigned char missile6_data[];

unsigned char *missile_data_data[7] =
{
    missile0_data,
    missile1_data,
    missile2_data,
    missile3_data,
    missile4_data,
    missile5_data,
    missile6_data
};

#define missile_tiles ((gfx_rletsprite_t**)missile_data_data)
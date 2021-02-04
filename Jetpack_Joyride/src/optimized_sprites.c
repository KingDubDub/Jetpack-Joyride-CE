/*

    Custom Sprite Tileset Array Header

Saves ~2600 bytes alone on just the missiles, others save a few hundred (exact value has yet to be calculated).
This is a dramatic enough cut that it looses ~60 bytes compressed even with a .28% compression drop.

Every byte is precious. This is not a waste of time.

*/

//custom barry sprite array:
extern unsigned char barry0_data[974];
extern unsigned char barry1_data[1001];
extern unsigned char barry2_data[902];
extern unsigned char barry3_data[816];

unsigned char *barryRun_array_data[4] =
{
    barry0_data,
    barry1_data,
    barry2_data,
    barry3_data
};

#define barryRun_array ((gfx_sprite_t**)barryRun_array_data)

//custom array for when Barry is hit:
extern unsigned char barryHit0_data[842];
extern unsigned char barryHit1_data[784];
extern unsigned char barryHit2_data[722];

unsigned char *barryHit_array_data[3] =
{
    barryHit0_data,
    barryHit1_data,
    barryHit2_data
};

#define barryHit_array_optimized ((gfx_sprite_t**)barryHit_array_data)

//Barry's dead corpse that slides on the ground:
extern unsigned char barryDed0_data[665];
extern unsigned char barryDed1_data[631];
extern unsigned char barryDed2_data[597];

unsigned char *barryDed_array_data[3] =
{
    barryDed0_data,
    barryDed1_data,
    barryDed2_data
};

#define barryDed_array_optimized ((gfx_sprite_t**)barryDed_array_data)

//custom exhaust array:
extern unsigned char exhaust0_data[145];
extern unsigned char exhaust1_data[189];
extern unsigned char exhaust2_data[211];
extern unsigned char exhaust3_data[222];
extern unsigned char exhaust4_data[255];
extern unsigned char exhaust5_data[232];


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
extern unsigned char coin0_data[146];
extern unsigned char coin1_data[134];
extern unsigned char coin2_data[98];
extern unsigned char coin3_data[134];

unsigned char *coin_array_data[4] =
{
    coin0_data,
    coin1_data,
    coin2_data,
    coin3_data
};

#define coin_array_optimized ((gfx_sprite_t**)coin_array_data)

//this is (currently) unecessary but I'm making zapppers like this for some later ideas:
extern unsigned char zapper0_data[1026];
extern unsigned char zapper1_data[1026];
extern unsigned char zapper2_data[1026];
extern unsigned char zapper3_data[1026];

unsigned char *zapper_array_data[] =
{
    zapper0_data,
    zapper1_data,
    zapper2_data,
    zapper3_data
};

#define zapper_array_optimized ((gfx_sprite_t**)zapper_array_data)

//custom missile tilesets:
extern unsigned char missile0_data[1682];
extern unsigned char missile1_data[1817];
extern unsigned char missile2_data[1707];
extern unsigned char missile3_data[1460];
extern unsigned char missile4_data[1460];
extern unsigned char missile5_data[1325];
extern unsigned char missile6_data[1452];

unsigned char *missile_array_data[7] =
{
    missile0_data,
    missile1_data,
    missile2_data,
    missile3_data,
    missile4_data,
    missile5_data,
    missile6_data
};

#define missile_array_optimized ((gfx_sprite_t**)missile_array_data)
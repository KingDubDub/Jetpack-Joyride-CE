#ifndef coin_include_file
#define coin_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char coin_tile_0_data[146];
#define coin_tile_0 ((gfx_sprite_t*)coin_tile_0_data)
extern unsigned char coin_tile_1_data[146];
#define coin_tile_1 ((gfx_sprite_t*)coin_tile_1_data)
extern unsigned char coin_tile_2_data[146];
#define coin_tile_2 ((gfx_sprite_t*)coin_tile_2_data)
extern unsigned char coin_tile_3_data[146];
#define coin_tile_3 ((gfx_sprite_t*)coin_tile_3_data)
#define coin_num_tiles 4
extern unsigned char *coin_tiles_data[4];
#define coin_tiles ((gfx_sprite_t**)coin_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

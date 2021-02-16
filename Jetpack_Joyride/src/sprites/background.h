#ifndef background_include_file
#define background_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char background_tile_0_data[22082];
#define background_tile_0 ((gfx_sprite_t*)background_tile_0_data)
extern unsigned char background_tile_1_data[22082];
#define background_tile_1 ((gfx_sprite_t*)background_tile_1_data)
extern unsigned char background_tile_2_data[22082];
#define background_tile_2 ((gfx_sprite_t*)background_tile_2_data)
#define background_num_tiles 3
extern unsigned char *background_tiles_data[3];
#define background_tiles ((gfx_sprite_t**)background_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

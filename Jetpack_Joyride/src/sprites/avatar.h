#ifndef avatar_include_file
#define avatar_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char avatar_tile_0_data[1202];
#define avatar_tile_0 ((gfx_sprite_t*)avatar_tile_0_data)
extern unsigned char avatar_tile_1_data[1202];
#define avatar_tile_1 ((gfx_sprite_t*)avatar_tile_1_data)
extern unsigned char avatar_tile_2_data[1202];
#define avatar_tile_2 ((gfx_sprite_t*)avatar_tile_2_data)
extern unsigned char avatar_tile_3_data[1202];
#define avatar_tile_3 ((gfx_sprite_t*)avatar_tile_3_data)
#define avatar_num_tiles 4
extern unsigned char *avatar_tiles_data[4];
#define avatar_tiles ((gfx_sprite_t**)avatar_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

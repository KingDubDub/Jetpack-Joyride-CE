#ifndef shutdown_include_file
#define shutdown_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char shutdown_tile_0_data[1322];
#define shutdown_tile_0 ((gfx_sprite_t*)shutdown_tile_0_data)
extern unsigned char shutdown_tile_1_data[1322];
#define shutdown_tile_1 ((gfx_sprite_t*)shutdown_tile_1_data)
extern unsigned char shutdown_tile_2_data[1322];
#define shutdown_tile_2 ((gfx_sprite_t*)shutdown_tile_2_data)
#define shutdown_num_tiles 3
extern unsigned char *shutdown_tiles_data[3];
#define shutdown_tiles ((gfx_sprite_t**)shutdown_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

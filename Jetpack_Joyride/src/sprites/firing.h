#ifndef firing_include_file
#define firing_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char firing_tile_0_data[1112];
#define firing_tile_0 ((gfx_sprite_t*)firing_tile_0_data)
extern unsigned char firing_tile_1_data[1112];
#define firing_tile_1 ((gfx_sprite_t*)firing_tile_1_data)
extern unsigned char firing_tile_2_data[1112];
#define firing_tile_2 ((gfx_sprite_t*)firing_tile_2_data)
#define firing_num_tiles 3
extern unsigned char *firing_tiles_data[3];
#define firing_tiles ((gfx_sprite_t**)firing_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

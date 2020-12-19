#ifndef electric_include_file
#define electric_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char electric_tile_0_data[1026];
#define electric_tile_0 ((gfx_sprite_t*)electric_tile_0_data)
extern unsigned char electric_tile_1_data[1026];
#define electric_tile_1 ((gfx_sprite_t*)electric_tile_1_data)
#define electric_num_tiles 2
extern unsigned char *electric_tiles_data[2];
#define electric_tiles ((gfx_sprite_t**)electric_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

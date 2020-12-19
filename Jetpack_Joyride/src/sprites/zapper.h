#ifndef zapper_include_file
#define zapper_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char zapper_tile_0_data[326];
#define zapper_tile_0 ((gfx_sprite_t*)zapper_tile_0_data)
extern unsigned char zapper_tile_1_data[326];
#define zapper_tile_1 ((gfx_sprite_t*)zapper_tile_1_data)
extern unsigned char zapper_tile_2_data[326];
#define zapper_tile_2 ((gfx_sprite_t*)zapper_tile_2_data)
#define zapper_num_tiles 3
extern unsigned char *zapper_tiles_data[3];
#define zapper_tiles ((gfx_sprite_t**)zapper_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

#ifndef powering_include_file
#define powering_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char powering_tile_0_data[287];
#define powering_tile_0 ((gfx_sprite_t*)powering_tile_0_data)
extern unsigned char powering_tile_1_data[287];
#define powering_tile_1 ((gfx_sprite_t*)powering_tile_1_data)
extern unsigned char powering_tile_2_data[287];
#define powering_tile_2 ((gfx_sprite_t*)powering_tile_2_data)
extern unsigned char powering_tile_3_data[287];
#define powering_tile_3 ((gfx_sprite_t*)powering_tile_3_data)
#define powering_num_tiles 4
extern unsigned char *powering_tiles_data[4];
#define powering_tiles ((gfx_sprite_t**)powering_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

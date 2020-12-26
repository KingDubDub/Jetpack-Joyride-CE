#ifndef laser_include_file
#define laser_include_file

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char laser_tile_0_data[3782];
#define laser_tile_0 ((gfx_rletsprite_t*)laser_tile_0_data)
extern unsigned char laser_tile_1_data[3782];
#define laser_tile_1 ((gfx_rletsprite_t*)laser_tile_1_data)
extern unsigned char laser_tile_2_data[3782];
#define laser_tile_2 ((gfx_rletsprite_t*)laser_tile_2_data)
extern unsigned char laser_tile_3_data[1021];
#define laser_tile_3 ((gfx_rletsprite_t*)laser_tile_3_data)
#define laser_num_tiles 4
extern unsigned char *laser_tiles_data[4];
#define laser_tiles ((gfx_rletsprite_t**)laser_tiles_data)

#ifdef __cplusplus
}
#endif

#endif

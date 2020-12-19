#ifndef laser_include_file
#define laser_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define laser_width 25
#define laser_height 15
#define laser_size 377
#define laser ((gfx_sprite_t*)laser_data)
extern unsigned char laser_data[377];

#ifdef __cplusplus
}
#endif

#endif

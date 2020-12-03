#ifndef JTPKGFX2_appvar_include_file
#define JTPKGFX2_appvar_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define singleObjects_palette_offset 0
#define jetpack_width 19
#define jetpack_height 27
#define jetpack_compressed JTPKGFX2_appvar[0]
#define nozzle_width 7
#define nozzle_height 2
#define nozzle_compressed JTPKGFX2_appvar[1]
#define sparkle_width 16
#define sparkle_height 15
#define sparkle_compressed JTPKGFX2_appvar[2]
#define beam_width 10
#define beam_height 10
#define beam_compressed JTPKGFX2_appvar[3]
#define laser_width 25
#define laser_height 15
#define laser_compressed JTPKGFX2_appvar[4]
#define JTPKGFX2_entries_num 5
extern unsigned char *JTPKGFX2_appvar[5];
unsigned char JTPKGFX2_init(void);

#ifdef __cplusplus
}
#endif

#endif

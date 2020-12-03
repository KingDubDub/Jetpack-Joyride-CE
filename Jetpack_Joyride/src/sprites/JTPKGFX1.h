#ifndef JTPKGFX1_appvar_include_file
#define JTPKGFX1_appvar_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define hallway_palette_offset 0
#define background_width 192
#define background_height 240
#define background_compressed JTPKGFX1_appvar[0]
#define JTPKGFX1_entries_num 1
extern unsigned char *JTPKGFX1_appvar[1];
unsigned char JTPKGFX1_init(void);

#ifdef __cplusplus
}
#endif

#endif

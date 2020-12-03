#ifndef JTPKPAL_appvar_include_file
#define JTPKPAL_appvar_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define sizeof_jetpack_palette 512
#define jetpack_palette (JTPKPAL_appvar[0])
#define JTPKPAL_entries_num 1
extern unsigned char *JTPKPAL_appvar[1];
unsigned char JTPKPAL_init(void);

#ifdef __cplusplus
}
#endif

#endif

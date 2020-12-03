#ifndef JTPKGFX3_appvar_include_file
#define JTPKGFX3_appvar_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define avatar_palette_offset 0
#define avatarSheet_tile_width 30
#define avatarSheet_tile_height 40
#define avatarSheet_compressed JTPKGFX3_appvar[0]
#define avatarSheet_tiles_num 4
extern unsigned char *avatarSheet_tiles_compressed[4];
#define avatarSheet_tile_0_compressed avatarSheet_tiles_compressed[0]
#define avatarSheet_tile_1_compressed avatarSheet_tiles_compressed[1]
#define avatarSheet_tile_2_compressed avatarSheet_tiles_compressed[2]
#define avatarSheet_tile_3_compressed avatarSheet_tiles_compressed[3]
#define exhaust_palette_offset 0
#define exhaustSheet_tile_width 11
#define exhaustSheet_tile_height 23
#define exhaustSheet_compressed JTPKGFX3_appvar[1]
#define exhaustSheet_tiles_num 6
extern unsigned char *exhaustSheet_tiles_compressed[6];
#define exhaustSheet_tile_0_compressed exhaustSheet_tiles_compressed[0]
#define exhaustSheet_tile_1_compressed exhaustSheet_tiles_compressed[1]
#define exhaustSheet_tile_2_compressed exhaustSheet_tiles_compressed[2]
#define exhaustSheet_tile_3_compressed exhaustSheet_tiles_compressed[3]
#define exhaustSheet_tile_4_compressed exhaustSheet_tiles_compressed[4]
#define exhaustSheet_tile_5_compressed exhaustSheet_tiles_compressed[5]
#define coin_palette_offset 0
#define coinSheet_tile_width 12
#define coinSheet_tile_height 12
#define coinSheet_compressed JTPKGFX3_appvar[2]
#define coinSheet_tiles_num 4
extern unsigned char *coinSheet_tiles_compressed[4];
#define coinSheet_tile_0_compressed coinSheet_tiles_compressed[0]
#define coinSheet_tile_1_compressed coinSheet_tiles_compressed[1]
#define coinSheet_tile_2_compressed coinSheet_tiles_compressed[2]
#define coinSheet_tile_3_compressed coinSheet_tiles_compressed[3]
#define zapper_palette_offset 0
#define zapperSheet_tile_width 18
#define zapperSheet_tile_height 18
#define zapperSheet_compressed JTPKGFX3_appvar[3]
#define zapperSheet_tiles_num 3
extern unsigned char *zapperSheet_tiles_compressed[3];
#define zapperSheet_tile_0_compressed zapperSheet_tiles_compressed[0]
#define zapperSheet_tile_1_compressed zapperSheet_tiles_compressed[1]
#define zapperSheet_tile_2_compressed zapperSheet_tiles_compressed[2]
#define electricStuff_palette_offset 0
#define electricSheet_tile_width 32
#define electricSheet_tile_height 32
#define electricSheet_compressed JTPKGFX3_appvar[4]
#define electricSheet_tiles_num 2
extern unsigned char *electricSheet_tiles_compressed[2];
#define electricSheet_tile_0_compressed electricSheet_tiles_compressed[0]
#define electricSheet_tile_1_compressed electricSheet_tiles_compressed[1]
#define missileWarning_palette_offset 0
#define missileWarning_tile_width 20
#define missileWarning_tile_height 21
#define missileWarning_compressed JTPKGFX3_appvar[5]
#define missileWarning_tiles_num 3
extern unsigned char *missileWarning_tiles_compressed[3];
#define missileWarning_tile_0_compressed missileWarning_tiles_compressed[0]
#define missileWarning_tile_1_compressed missileWarning_tiles_compressed[1]
#define missileWarning_tile_2_compressed missileWarning_tiles_compressed[2]
#define missileIncoming_palette_offset 0
#define missileIncoming_tile_width 31
#define missileIncoming_tile_height 31
#define missileIncoming_compressed JTPKGFX3_appvar[6]
#define missileIncoming_tiles_num 2
extern unsigned char *missileIncoming_tiles_compressed[2];
#define missileIncoming_tile_0_compressed missileIncoming_tiles_compressed[0]
#define missileIncoming_tile_1_compressed missileIncoming_tiles_compressed[1]
#define missile_palette_offset 0
#define missile_width 46
#define missile_height 36
#define missile_compressed JTPKGFX3_appvar[7]
#define laser_powering_palette_offset 0
#define powering_tile_width 19
#define powering_tile_height 15
#define powering_compressed JTPKGFX3_appvar[8]
#define powering_tiles_num 4
extern unsigned char *powering_tiles_compressed[4];
#define powering_tile_0_compressed powering_tiles_compressed[0]
#define powering_tile_1_compressed powering_tiles_compressed[1]
#define powering_tile_2_compressed powering_tiles_compressed[2]
#define powering_tile_3_compressed powering_tiles_compressed[3]
#define laser_firing_palette_offset 0
#define firing_tile_width 30
#define firing_tile_height 37
#define firing_compressed JTPKGFX3_appvar[9]
#define firing_tiles_num 3
extern unsigned char *firing_tiles_compressed[3];
#define firing_tile_0_compressed firing_tiles_compressed[0]
#define firing_tile_1_compressed firing_tiles_compressed[1]
#define firing_tile_2_compressed firing_tiles_compressed[2]
#define laser_shutdown_palette_offset 0
#define laserShutDown_tile_width 30
#define laserShutDown_tile_height 44
#define laserShutDown_compressed JTPKGFX3_appvar[10]
#define laserShutDown_tiles_num 3
extern unsigned char *laserShutDown_tiles_compressed[3];
#define laserShutDown_tile_0_compressed laserShutDown_tiles_compressed[0]
#define laserShutDown_tile_1_compressed laserShutDown_tiles_compressed[1]
#define laserShutDown_tile_2_compressed laserShutDown_tiles_compressed[2]
#define JTPKGFX3_entries_num 11
extern unsigned char *JTPKGFX3_appvar[11];
unsigned char JTPKGFX3_init(void);

#ifdef __cplusplus
}
#endif

#endif

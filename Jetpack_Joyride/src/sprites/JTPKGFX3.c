#include "JTPKGFX3.h"
#include <fileioc.h>

#define JTPKGFX3_HEADER_SIZE 0

unsigned char *JTPKGFX3_appvar[11] =
{
    (unsigned char*)0,
    (unsigned char*)1461,
    (unsigned char*)1867,
    (unsigned char*)2261,
    (unsigned char*)2515,
    (unsigned char*)2893,
    (unsigned char*)3124,
    (unsigned char*)3430,
    (unsigned char*)3781,
    (unsigned char*)4609,
    (unsigned char*)5251,
};

unsigned char *avatarSheet_tiles_compressed[4] =
{
    (unsigned char*)0,
    (unsigned char*)381,
    (unsigned char*)755,
    (unsigned char*)1135,
};

unsigned char *exhaustSheet_tiles_compressed[6] =
{
    (unsigned char*)0,
    (unsigned char*)65,
    (unsigned char*)140,
    (unsigned char*)223,
    (unsigned char*)297,
    (unsigned char*)359,
};

unsigned char *coinSheet_tiles_compressed[4] =
{
    (unsigned char*)0,
    (unsigned char*)126,
    (unsigned char*)240,
    (unsigned char*)288,
};

unsigned char *zapperSheet_tiles_compressed[3] =
{
    (unsigned char*)0,
    (unsigned char*)84,
    (unsigned char*)171,
};

unsigned char *electricSheet_tiles_compressed[2] =
{
    (unsigned char*)0,
    (unsigned char*)198,
};

unsigned char *missileWarning_tiles_compressed[3] =
{
    (unsigned char*)0,
    (unsigned char*)84,
    (unsigned char*)147,
};

unsigned char *missileIncoming_tiles_compressed[2] =
{
    (unsigned char*)0,
    (unsigned char*)151,
};

unsigned char *powering_tiles_compressed[4] =
{
    (unsigned char*)0,
    (unsigned char*)209,
    (unsigned char*)440,
    (unsigned char*)647,
};

unsigned char *firing_tiles_compressed[3] =
{
    (unsigned char*)0,
    (unsigned char*)211,
    (unsigned char*)412,
};

unsigned char *laserShutDown_tiles_compressed[3] =
{
    (unsigned char*)0,
    (unsigned char*)225,
    (unsigned char*)451,
};

unsigned char JTPKGFX3_init(void)
{
    unsigned int data, i;
    ti_var_t appvar;

    ti_CloseAll();

    appvar = ti_Open("JTPKGFX3", "r");
    if (appvar == 0)
    {
        return 0;
    }

    data = (unsigned int)ti_GetDataPtr(appvar) - (unsigned int)JTPKGFX3_appvar[0] + JTPKGFX3_HEADER_SIZE;
    for (i = 0; i < 11; i++)
    {
        JTPKGFX3_appvar[i] += data;
    }

    ti_CloseAll();

    data = (unsigned int)JTPKGFX3_appvar[0] - (unsigned int)avatarSheet_tiles_compressed[0];
    for (i = 0; i < avatarSheet_tiles_num; i++)
    {
        avatarSheet_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[1] - (unsigned int)exhaustSheet_tiles_compressed[0];
    for (i = 0; i < exhaustSheet_tiles_num; i++)
    {
        exhaustSheet_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[2] - (unsigned int)coinSheet_tiles_compressed[0];
    for (i = 0; i < coinSheet_tiles_num; i++)
    {
        coinSheet_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[3] - (unsigned int)zapperSheet_tiles_compressed[0];
    for (i = 0; i < zapperSheet_tiles_num; i++)
    {
        zapperSheet_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[4] - (unsigned int)electricSheet_tiles_compressed[0];
    for (i = 0; i < electricSheet_tiles_num; i++)
    {
        electricSheet_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[5] - (unsigned int)missileWarning_tiles_compressed[0];
    for (i = 0; i < missileWarning_tiles_num; i++)
    {
        missileWarning_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[6] - (unsigned int)missileIncoming_tiles_compressed[0];
    for (i = 0; i < missileIncoming_tiles_num; i++)
    {
        missileIncoming_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[8] - (unsigned int)powering_tiles_compressed[0];
    for (i = 0; i < powering_tiles_num; i++)
    {
        powering_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[9] - (unsigned int)firing_tiles_compressed[0];
    for (i = 0; i < firing_tiles_num; i++)
    {
        firing_tiles_compressed[i] += data;
    }

    data = (unsigned int)JTPKGFX3_appvar[10] - (unsigned int)laserShutDown_tiles_compressed[0];
    for (i = 0; i < laserShutDown_tiles_num; i++)
    {
        laserShutDown_tiles_compressed[i] += data;
    }

    return 1;
}


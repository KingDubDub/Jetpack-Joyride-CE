#include "JTPKGFX2.h"
#include <fileioc.h>

#define JTPKGFX2_HEADER_SIZE 0

unsigned char *JTPKGFX2_appvar[5] =
{
    (unsigned char*)0,
    (unsigned char*)138,
    (unsigned char*)153,
    (unsigned char*)220,
    (unsigned char*)234,
};

unsigned char JTPKGFX2_init(void)
{
    unsigned int data, i;
    ti_var_t appvar;

    ti_CloseAll();

    appvar = ti_Open("JTPKGFX2", "r");
    if (appvar == 0)
    {
        return 0;
    }

    data = (unsigned int)ti_GetDataPtr(appvar) - (unsigned int)JTPKGFX2_appvar[0] + JTPKGFX2_HEADER_SIZE;
    for (i = 0; i < 5; i++)
    {
        JTPKGFX2_appvar[i] += data;
    }

    ti_CloseAll();

    return 1;
}


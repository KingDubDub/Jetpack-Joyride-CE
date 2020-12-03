#include "JTPKGFX1.h"
#include <fileioc.h>

#define JTPKGFX1_HEADER_SIZE 0

unsigned char *JTPKGFX1_appvar[1] =
{
    (unsigned char*)0,
};

unsigned char JTPKGFX1_init(void)
{
    unsigned int data, i;
    ti_var_t appvar;

    ti_CloseAll();

    appvar = ti_Open("JTPKGFX1", "r");
    if (appvar == 0)
    {
        return 0;
    }

    data = (unsigned int)ti_GetDataPtr(appvar) - (unsigned int)JTPKGFX1_appvar[0] + JTPKGFX1_HEADER_SIZE;
    for (i = 0; i < 1; i++)
    {
        JTPKGFX1_appvar[i] += data;
    }

    ti_CloseAll();

    return 1;
}


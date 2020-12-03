#include "JTPKPAL.h"
#include <fileioc.h>

#define JTPKPAL_HEADER_SIZE 0

unsigned char *JTPKPAL_appvar[1] =
{
    (unsigned char*)0,
};

unsigned char JTPKPAL_init(void)
{
    unsigned int data, i;
    ti_var_t appvar;

    ti_CloseAll();

    appvar = ti_Open("JTPKPAL", "r");
    if (appvar == 0)
    {
        return 0;
    }

    data = (unsigned int)ti_GetDataPtr(appvar) - (unsigned int)JTPKPAL_appvar[0] + JTPKPAL_HEADER_SIZE;
    for (i = 0; i < 1; i++)
    {
        JTPKPAL_appvar[i] += data;
    }

    ti_CloseAll();

    return 1;
}


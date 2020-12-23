#include <stdint.h>

//max number of lasers, used for initializing:
#define MaxLasers 7

//max number of lasers per formations, so only the ones used are updated:
const uint8_t laserMax[3] =
{
    2,
    4,
    7
};

//Y-coords, no X's needed:
const uint8_t lsrY[3][MaxLasers] =
{{
    24,
    204
},{
    24,
    54,
    84,
    204
},{
    24,
    54,
    84,
    114,
    144,
    174,
    204
}};

//TTL for lasers, it's the last 108 cycles that counts:
const uint8_t halfLife[3][MaxLasers] =
{{
    108,
    108
},{
    108,
    108,
    108,
    108
},{
    108,
    108,
    216,
    216,
    216,
    108,
    108
}};

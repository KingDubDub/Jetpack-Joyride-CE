#include <stdint.h>

//max number of lasers, used for initializing:
#define MaxLasers 7

//max number of... do I really need to explain this one?
#define LASER_FORMATIONS 5

//max number of lasers per formations, so only the ones used are updated:
const uint8_t laserMax[LASER_FORMATIONS] =
{
    1,
    2,
    4,
    4,
    7
};

//Y-coords, no X's needed:
const uint8_t lsrY[LASER_FORMATIONS][MaxLasers] =
{{
    114
},{
    24,
    204
},{
    24,
    144,
    174,
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
const uint8_t halfLife[LASER_FORMATIONS][MaxLasers] =
{{
    108
},{
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

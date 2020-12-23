#include <stdint.h>

#include "coin_formations.c"

//max coins per coin formation, so the coin loop can finish sooner:
extern const uint8_t coin_max[6];

//coin formation type arrays:
extern const uint24_t ctx[6][MaxCoins];
extern const uint8_t cty[6][MaxCoins];

#include "laser_formations.c"

extern const uint8_t laserMax[3];

extern const uint8_t lsrY[3][MaxLasers];

extern const uint8_t halfLife[3][MaxLasers];

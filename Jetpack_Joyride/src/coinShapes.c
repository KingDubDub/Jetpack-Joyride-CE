#include <stdint.h>
#include "coinShapes.h"

//max coins per coin formation, so the coin loop can finish sooner:
extern const uint8_t abbreviatedMax[6];

//coin formation type arrays:
extern const uint24_t ctx[6][MaxCoins];
extern const uint8_t cty[6][MaxCoins];

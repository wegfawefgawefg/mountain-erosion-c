#include "util.h"
#include <stdlib.h>

float rand_range(float min, float max)
{
    float scale = rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

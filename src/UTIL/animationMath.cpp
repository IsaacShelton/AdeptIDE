
#include "UTIL/animationMath.h"

double clampedHalfDelta(double delta){
    return delta < 2 ? delta / 2 : 1;
}

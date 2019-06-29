
#include "UTIL/animationMath.h"

double clampedHalfDelta(double delta){
    return delta < 2.5 ? delta / 2.5 : 1;
}


#include <math.h>
#include "UTIL/animationMath.h"

double clampedHalfDelta(double delta){
    return delta < 2.5 ? delta / 2.5 : 1;
}

double easeOutElastic(double t){
    // Only do easing function before t=1
    if(t >= 1.0) return 1.0;

    float p = 0.3;
    return powf(6.0f, -10.0f * t) * sinf((t - p / 4.0f) * (2.0f * M_PI) / p) + 1.0f;
}

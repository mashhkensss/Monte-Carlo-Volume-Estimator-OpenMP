#include "hit.h"
#include <cmath>
#include <algorithm>

bool hit_test(float x, float y, float z) {
    float a = 2.0f;

    // piriform formula: a^2(y^2 + z^2) - a(x^3) + (x^4) = 0
    return a * a * (y * y + z * z) - a * (x * x * x) + (x * x * x * x) <= 0.0f;
}

const float* get_axis_range() {
    static const float range[6] = {0.0f, 2.0f, -0.65f, 0.65f, -0.65f, 0.65f};

    return range;
}

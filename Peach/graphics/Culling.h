#ifndef PEACH_CULLING_H
#define PEACH_CULLING_H

#include "Peach/core/Structs.h"
#include <math.h>

// test some stuff ig
static inline int mBoundsInFrustum(vec3 minimum, vec3 maximum, mat4 clip) {
    for (int axis = 0; axis < 3; ++axis) {
        for (int sign = -1; sign <= 1; sign += 2) {
            float distance = clip[3][3] + sign * clip[3][axis];
            float magnitude = fabsf(distance);
            for (int component = 0; component < 3; ++component) {
                float plane = clip[component][3] + sign * clip[component][axis];
                float support = plane >= 0 ? maximum[component] : minimum[component];
                distance += plane * support;
                magnitude += fabsf(plane * support);
            }
            if (isfinite(distance) && distance < -1e-5f * fmaxf(1.0f, magnitude)) return 0;
        }
    }
    return 1;
}

#endif

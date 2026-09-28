#ifndef PEACH_TRANSFORM_H
#define PEACH_TRANSFORM_H

#include <Peach/Common.h>

// Identity: position/rotation = 0, scale = 1. A zero-initialized scale collapses geometry.
// Rotation uses radians. Local vertices are scaled, rotated X then Y then Z,
// and translated. For 2D rotation, set rotation.z.
mTransform mTransformCreate(void);

#endif //PEACH_TRANSFORM_H

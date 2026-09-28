#ifndef PEACH_TRANSFORM_INTERNAL_H
#define PEACH_TRANSFORM_INTERNAL_H

#include <Peach/Transform.h>

// Renderer-only conversion. NULL transform produces identity.
int mTransformMatrix(const mTransform* transform, float* matrix);

#endif //PEACH_TRANSFORM_INTERNAL_H

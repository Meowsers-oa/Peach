#include <Peach/Common.h>
#include <Peach/Transform.h>
#include "TransformInternal.h"
#include <cglm/struct.h>
#include <math.h>
#include <string.h>

mTransform mTransformCreate(void) {
    return (mTransform){.scale = {.x = 1.0f, .y = 1.0f, .z = 1.0f}};
}

int mTransformMatrix(const mTransform* transform, float* matrix) {
    mat4s result = glms_mat4_identity();
    if (transform != NULL) {
        for (int i = 0; i < 3; i++) {
            if (!isfinite(transform->position.raw[i]) || !isfinite(transform->rotation.raw[i]) ||
                !isfinite(transform->scale.raw[i])) return M_FAILURE;
        }
        result = glms_translate(result, transform->position);
        result = glms_rotate_z(result, transform->rotation.z);
        result = glms_rotate_y(result, transform->rotation.y);
        result = glms_rotate_x(result, transform->rotation.x);
        result = glms_scale(result, transform->scale);
    }
    for (int column = 0; column < 4; column++) {
        for (int row = 0; row < 4; row++) {
            if (!isfinite(result.raw[column][row])) return M_FAILURE;
        }
    }
    memcpy(matrix, result.raw, sizeof(result.raw));
    return M_SUCCESS;
}

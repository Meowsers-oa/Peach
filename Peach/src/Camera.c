#include <Peach/Camera.h>
#include <math.h>
#include <string.h>

int mCameraCreate(mCamera* camera, float width, float height) {
    if (camera == NULL) return M_FAILURE;
    mCamera result = {.zoom = 1.0f,
                      .width = width, .height = height};
    if (mCameraUpdate(&result) == M_FAILURE) return M_FAILURE;
    *camera = result;
    return M_SUCCESS;
}

int mCameraUpdate(mCamera* camera) {
    if (camera == NULL || !isfinite(camera->x) || !isfinite(camera->y) ||
        !isfinite(camera->rotation) || !isfinite(camera->zoom) || camera->zoom <= 0.0f ||
        !isfinite(camera->width) || !isfinite(camera->height) ||
        camera->width <= 0.0f || camera->height <= 0.0f) return M_FAILURE;

    float c = cosf(camera->rotation);
    float s = sinf(camera->rotation);
    float view[16] = {
        c, -s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        -c * camera->x - s * camera->y, s * camera->x - c * camera->y, 0, 1
    };
    float projection[16] = {
        2.0f * camera->zoom / camera->width, 0, 0, 0,
        0, -2.0f * camera->zoom / camera->height, 0, 0,
        0, 0, -1, 0,
        -1, 1, 0, 1
    };
    float viewProjection[16] = {0};
    for (int column = 0; column < 4; column++) {
        for (int row = 0; row < 4; row++) {
            for (int k = 0; k < 4; k++) {
                viewProjection[column * 4 + row] += projection[k * 4 + row] * view[column * 4 + k];
            }
            if (!isfinite(viewProjection[column * 4 + row])) return M_FAILURE;
        }
    }
    memcpy(camera->view, view, sizeof(view));
    memcpy(camera->projection, projection, sizeof(projection));
    memcpy(camera->viewProjection, viewProjection, sizeof(viewProjection));
    return M_SUCCESS;
}

int mCameraResize(mCamera* camera, float width, float height) {
    if (camera == NULL) return M_FAILURE;
    mCamera result = *camera;
    result.width = width;
    result.height = height;
    if (mCameraUpdate(&result) == M_FAILURE) return M_FAILURE;
    *camera = result;
    return M_SUCCESS;
}

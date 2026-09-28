#ifndef PEACH_CAMERA_H
#define PEACH_CAMERA_H

#include <Peach/Common.h>

typedef struct {
    float x;
    float y;
    float rotation;
    float zoom;
    float width;
    float height;
    float view[16];
    float projection[16];
    float viewProjection[16];
}mCamera;

// Orthographic camera. Position is the world point at the viewport center.
// Rotation is in radians; zoom > 1 magnifies. Positive world Y points down.
// Creation centers the camera at (width / 2, height / 2), with zoom = 1.
int mCameraCreate(mCamera* camera, float width, float height);
// Call after editing position, rotation or zoom. Matrices are column-major.
// Invalid parameters return M_FAILURE and leave the previous matrices intact.
int mCameraUpdate(mCamera* camera);
// Updates viewport dimensions without changing the camera position.
int mCameraResize(mCamera* camera, float width, float height);

#endif //PEACH_CAMERA_H

#ifndef PEACH_CAMERA_H
#define PEACH_CAMERA_H

#include <Peach/Common.h>

// Pixel coordinates, top-left origin. Rotation is in radians.
int mCameraCreate(mCamera* camera, float width, float height);
// Recalculate after changing camera fields.
int mCameraUpdate(mCamera* camera);
int mCameraResize(mCamera* camera, float width, float height);

#endif //PEACH_CAMERA_H

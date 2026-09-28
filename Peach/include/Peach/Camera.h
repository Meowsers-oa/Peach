#ifndef PEACH_CAMERA_H
#define PEACH_CAMERA_H

#include <Peach/Common.h>

// Orthographic camera in window pixels. Position is the world point at the
// top-left of the viewport. Creation starts at (0, 0), with zoom = 1.
// Positive X points right and Y points down, matching mouse coordinates.
// Rotation is in radians; zoom > 1 magnifies around the top-left corner.
int mCameraCreate(mCamera* camera, float width, float height);
// Call after editing position, rotation or zoom. Matrices are column-major.
// Invalid parameters return M_FAILURE and leave the previous matrices intact.
int mCameraUpdate(mCamera* camera);
// Updates viewport dimensions without changing the camera position.
int mCameraResize(mCamera* camera, float width, float height);

#endif //PEACH_CAMERA_H

#ifndef PEACH_RENDERER_H
#define PEACH_RENDERER_H

#include <Peach/Common.h>
#include <Peach/Camera.h>
#include <Peach/Texture.h>
#include <Peach/Transform.h>

int mRendererCreate(mContext* ctx);
void mRendererDestroy(mContext* ctx);
// Clears the next frame. mUpdate flushes, presents, then calls this.
void mRendererBegin(mContext* ctx);
void mRendererFlush(mContext* ctx);
void mRendererPresent(mContext* ctx);
// Call between frames. Fixed pixels upscale by integers with letterboxing; 0,0 restores window size.
int mRendererSetResolution(mContext* ctx, int width, int height);
// Converts GLFW window coordinates to render pixels. Returns false in the letterbox bars.
M_BOOL mRendererWindowToScreen(mContext* ctx, double x, double y, double* screenX, double* screenY);

// Column-major camera matrix; NULL restores pixel coordinates.
void mRendererSetViewProjection(mContext* ctx, const float* viewProjection);
// Set before drawing the frame. Lights and geometry share this camera.
void mRendererSetCamera(mContext* ctx, const mCamera* camera);

// Triangle lists in pixels. NULL transform means identity; NULL texture means white.
// Geometry is copied; textures must survive until flush. Flush before external GL changes.
int mAddVertices(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                 const mTransform* transform, const mTexture* texture);
int mAddVerticesIndexed(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                        const unsigned int* indices, unsigned int indexCount,
                        const mTransform* transform, const mTexture* texture);

#endif //PEACH_RENDERER_H

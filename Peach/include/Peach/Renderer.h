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

// Column-major camera matrix; NULL restores pixel coordinates.
void mRendererSetViewProjection(mContext* ctx, const float* viewProjection);
// Copies the camera matrix; reapply after camera changes.
void mRendererSetCamera(mContext* ctx, const mCamera* camera);

// Triangle lists in pixels. NULL transform means identity; NULL texture means white.
// Geometry is copied; textures must survive until flush. Flush before external GL changes.
int mAddVertices(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                 const mTransform* transform, const mTexture* texture);
int mAddVerticesIndexed(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                        const unsigned int* indices, unsigned int indexCount,
                        const mTransform* transform, const mTexture* texture);

#endif //PEACH_RENDERER_H

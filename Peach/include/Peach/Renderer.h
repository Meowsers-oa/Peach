#ifndef PEACH_RENDERER_H
#define PEACH_RENDERER_H

#include <Peach/Common.h>
#include <Peach/Camera.h>
#include <Peach/Texture.h>

#define M_RENDERER_MAX_VERTICES 16384
#define M_RENDERER_MAX_INDICES 24576
#define M_RENDERER_MAX_TEXTURES 16

typedef struct {
    float x;
    float y;
    float z;
    float u;
    float v;
    mColor color;
}mVertex;

// Managed by the window lifecycle. The owning OpenGL context must be current.
int mRendererCreate(mContext* ctx);
void mRendererDestroy(mContext* ctx);
// Begin clears the back buffer; mUpdate flushes, presents, then begins the next frame.
void mRendererBegin(mContext* ctx);
void mRendererFlush(mContext* ctx);

// Column-major matrices, compatible with cglm. NULL restores the default camera:
// window coordinates, origin at the top left, positive Y down, Z from -1 to 1.
// Camera changes flush pending geometry. Custom cameras persist across frames.
void mRendererSetViewProjection(mContext* ctx, const float* viewProjection);
// Copies the camera's current matrix. Reapply after camera updates or resize.
// NULL restores the automatic window-sized camera.
void mRendererSetCamera(mContext* ctx, const mCamera* camera);

// Triangle lists. Counts must be multiples of three (indexCount for indexed lists).
// Indices are local to this submission. Oversized submissions are split automatically.
// transform is a column-major model matrix; NULL means identity. Vertices and
// matrices are copied during submission. NULL texture uses an internal white texture.
// Color multiplies the sampled texture. Draw order is preserved, with straight-alpha
// blending and no depth test. Textures must remain valid until the batch is flushed.
int mAddVertices(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                 const float* transform, const mTexture* texture);
int mAddVerticesIndexed(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                        const unsigned int* indices, unsigned int indexCount,
                        const float* transform, const mTexture* texture);

// Flush before changing external GL state, texture contents, or render targets.
// Flush sets its own shader, VAO, textures, blend/depth/cull/scissor state and does
// not restore them. A context must not be copied or moved after window creation.

#endif //PEACH_RENDERER_H

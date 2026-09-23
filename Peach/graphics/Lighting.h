#ifndef PEACH_LIGHTING_H
#define PEACH_LIGHTING_H

#include "Peach/graphics/Renderer.h"

// Internal scene storage, consumed by the lighting and color passes.
struct mRenderBatch {
    mVertex* vertices;
    unsigned int* indices;
    unsigned int vertexCount, indexCount;
    unsigned int textures[MAX_TEXTURE_SLOTS];
    unsigned int textureCount;
    unsigned int shaderProgram;
    mat4 model, view, projection;
    int viewport[4];
    unsigned int framebuffer;
    int depthTest, cullFace, blend;
    int depthFunc, cullMode, frontFace;
    unsigned char depthWrite;
    struct mRenderBatch* next;
};

void mLightingRenderScene(mContext* ctx);
void mLightingShutdown(mContext* ctx);
void mRendererDiscardBatches(mContext* ctx);

#endif

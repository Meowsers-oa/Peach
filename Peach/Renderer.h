//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_RENDERER_H
#define PEACH_RENDERER_H

#include "Structs.h"
#include "Texture.h"
#include "Camera.h"

// Renderer lifecycle
void mRendererInit(mContext* ctx);
void mRendererShutdown(mContext* ctx);

// Batch controls
void mRendererBegin(mContext* ctx);
void mRendererEnd(mContext* ctx);
void mRendererFlush(mContext* ctx);

// Clear, depth test & viewport
void mRendererClear(mContext* ctx);
void mRendererClearColor(Color color);
void mRendererSetDepthTest(int enable);
void mRendererSetCullFace(int enable);

// Camera and transformation matrices
void mRendererSetProjection(mContext* ctx, mat4 proj);
void mRendererSetView(mContext* ctx, mat4 view);
void mRendererSetModel(mContext* ctx, mat4 model);
void mRendererSetCamera2D(mContext* ctx, float left, float right, float bottom, float top);
void mRendererSetCamera3D(mContext* ctx, const mCamera3D* camera);
void mRendererSetCamera(mContext* ctx, const mCamera3D* camera);
void mRendererSetPerspective(mContext* ctx, float fovDeg, float aspect, float nearVal, float farVal);
void mRendererSetLookAt(mContext* ctx, vec3 eye, vec3 center, vec3 up);

// Texture slot management for batch renderer
int mRendererBindTexture(mContext* ctx, const mTexture* texture);
int mRendererBindTextureId(mContext* ctx, unsigned int textureId);

// Core Vertex Batching:
void mAddVertices(mContext* ctx, const mVertex* vertices, unsigned int count);
void mAddVerticesIndexed(mContext* ctx, const mVertex* vertices, unsigned int vertexCount, const unsigned int* indices, unsigned int indexCount);

// Convenience 2D/3D helpers
void mAddVertex(mContext* ctx, mVertex vertex);
void mAddTriangle(mContext* ctx, mVertex v1, mVertex v2, mVertex v3);
void mAddQuad(mContext* ctx, mVertex v1, mVertex v2, mVertex v3, mVertex v4);
void mAddQuad3D(mContext* ctx, vec3 center, vec2 size, vec3 normal, Color color);
void mAddQuad3DTextured(mContext* ctx, vec3 center, vec2 size, vec3 normal, const mTexture* texture, Color tint);
void mAddCube(mContext* ctx, vec3 position, vec3 size, Color color);
void mAddCubeTextured(mContext* ctx, vec3 position, vec3 size, const mTexture* texture, Color tint);

// Statistics
mRendererStats mRendererGetStats(mContext* ctx);
void mRendererResetStats(mContext* ctx);

// Function aliases
void mInitRenderer(mContext* ctx);
void mBeginBatch(mContext* ctx);
void mEndBatch(mContext* ctx);
void mFlushBatch(mContext* ctx);

#endif //PEACH_RENDERER_H

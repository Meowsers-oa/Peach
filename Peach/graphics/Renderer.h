//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_RENDERER_H
#define PEACH_RENDERER_H

#include "Peach/core/Structs.h"
#include "Peach/graphics/Texture.h"
#include "Peach/graphics/Camera.h"

// Renderer lifecycle
void mRendererInit(mContext* ctx);
void mRendererShutdown(mContext* ctx);

// A Begin/End pair is a scene: End builds shadows then draws all queued batches.
// Flush seals the current batch; inside Begin/End it does not draw immediately.
// Keep submitted textures alive until End.
// Batch controls
void mRendererBegin(mContext* ctx);
void mRendererEnd(mContext* ctx);
void mRendererFlush(mContext* ctx);

// Clear, depth test & viewport
void mRendererClear(mContext* ctx);
void mRendererClearColor(mColor color);
void mRendererSetDepthTest(int enable);
void mRendererSetCullFace(int enable);
// Enabled by default. Culls whole batches independently for the camera and
// each shadow face. Disable for custom shaders that move vertices beyond bounds.
void mRendererSetFrustumCulling(mContext* ctx, int enable);

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
void mAddQuad3D(mContext* ctx, vec3 center, vec2 size, vec3 normal, mColor color);
void mAddQuad3DTextured(mContext* ctx, vec3 center, vec2 size, vec3 normal, const mTexture* texture, mColor tint);
void mAddCube(mContext* ctx, vec3 position, vec3 size, mColor color);
void mAddCubeTextured(mContext* ctx, vec3 position, vec3 size, const mTexture* texture, mColor tint);

// Rotation is in radians about the object's center, applied X, then Y, then Z
// around fixed axes in model space. For quads it follows the normal-based orientation.
void mAddQuad3DRotated(mContext* ctx, vec3 center, vec2 size, vec3 normal, vec3 rotation, mColor color);
void mAddQuad3DTexturedRotated(mContext* ctx, vec3 center, vec2 size, vec3 normal, vec3 rotation, const mTexture* texture, mColor tint);
void mAddCubeRotated(mContext* ctx, vec3 position, vec3 size, vec3 rotation, mColor color);
void mAddCubeTexturedRotated(mContext* ctx, vec3 position, vec3 size, vec3 rotation, const mTexture* texture, mColor tint);

void mDraw(mContext *ctx, mObject* obj);

// Draw after scene geometry: a depth-tested, unlit ball that does not cast shadows.
void mDrawLightMarker(mContext* ctx, const mLight* light, float radius);
void mDrawLightMarkers(mContext* ctx, float radius);

// Statistics
mRendererStats mRendererGetStats(mContext* ctx);
void mRendererResetStats(mContext* ctx);

// Lights are borrowed: keep them alive until removed or renderer shutdown.
// Edit their fields before End to update them. Up to MAX_LIGHTS are supported.
// Constructors enable shadows and supply range/cone/bias defaults.
mLight mCreatePointLight(vec3 position, mColor color, float intensity);
mLight mCreateSpotLight(vec3 position, vec3 direction, mColor color, float intensity);
int mRendererAddLight(mContext* ctx, mLight* light); // 1 on success, 0 if invalid/full.
void mRendererRemoveLight(mContext* ctx, mLight* light);
void mRendererSetAmbientLight(mContext* ctx, float intensity);
void addLight(mContext* ctx, mLight* light);
void removeLight(mContext* ctx, mLight* light);


#endif //PEACH_RENDERER_H

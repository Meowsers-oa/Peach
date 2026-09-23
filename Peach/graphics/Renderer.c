//
// Created by Štěpán Toman on 20.09.2026.
//

#include "Peach/graphics/Renderer.h"
#include "Peach/graphics/Shader.h"
#include "Peach/graphics/Lighting.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#define VERTEX_HASH_CAPACITY 65536
#define VERTEX_HASH_MASK (VERTEX_HASH_CAPACITY - 1)

static int s_vertexHashTable[VERTEX_HASH_CAPACITY];
static int s_vertexNext[MAX_BATCH_VERTICES];

static inline uint32_t hashVertex(const mVertex* v) {
    int32_t data[12];
    data[0] = (int32_t)roundf(v->position[0] * 1000.0f);
    data[1] = (int32_t)roundf(v->position[1] * 1000.0f);
    data[2] = (int32_t)roundf(v->position[2] * 1000.0f);
    data[3] = (int32_t)roundf(v->color.r * 1000.0f);
    data[4] = (int32_t)roundf(v->color.g * 1000.0f);
    data[5] = (int32_t)roundf(v->color.b * 1000.0f);
    data[6] = (int32_t)roundf(v->color.a * 1000.0f);
    data[7] = (int32_t)roundf(v->normals[0] * 1000.0f);
    data[8] = (int32_t)roundf(v->normals[1] * 1000.0f);
    data[9] = (int32_t)roundf(v->normals[2] * 1000.0f);
    data[10] = (int32_t)roundf(v->texCoord[0] * 1000.0f);
    data[11] = (int32_t)roundf(v->texCoord[1] * 1000.0f);

    uint32_t hash = 2166136261u;
    for (int i = 0; i < 12; i++) {
        hash ^= (uint32_t)data[i];
        hash *= 16777619u;
    }
    hash ^= (uint32_t)roundf(v->texId);
    hash *= 16777619u;
    return hash;
}
static inline bool vertexEqual(const mVertex* a, const mVertex* b) {
    return fabsf(a->position[0] - b->position[0]) < 1e-4f &&
           fabsf(a->position[1] - b->position[1]) < 1e-4f &&
           fabsf(a->position[2] - b->position[2]) < 1e-4f &&
           fabsf(a->color.r - b->color.r) < 1e-4f &&
           fabsf(a->color.g - b->color.g) < 1e-4f &&
           fabsf(a->color.b - b->color.b) < 1e-4f &&
           fabsf(a->color.a - b->color.a) < 1e-4f &&
           fabsf(a->normals[0] - b->normals[0]) < 1e-4f &&
           fabsf(a->normals[1] - b->normals[1]) < 1e-4f &&
           fabsf(a->normals[2] - b->normals[2]) < 1e-4f &&
           fabsf(a->texCoord[0] - b->texCoord[0]) < 1e-4f &&
           fabsf(a->texCoord[1] - b->texCoord[1]) < 1e-4f &&
           fabsf(a->texId - b->texId) < 1e-4f;
}

static void mRendererStartBatch(mContext* ctx) {
    ctx->renderer.vertexBufferPtr = ctx->renderer.vertexBuffer;
    ctx->renderer.indexBufferPtr = ctx->renderer.indexBuffer;
    ctx->renderer.vertexCount = 0;
    ctx->renderer.indexCount = 0;
    ctx->renderer.textureSlotIndex = 1;
    ctx->renderer.textureSlots[0] = ctx->renderer.whiteTexture;
    for (int i = 1; i < MAX_TEXTURE_SLOTS; i++) {
        ctx->renderer.textureSlots[i] = 0;
    }
    memset(s_vertexHashTable, -1, sizeof(s_vertexHashTable));
}
void mRendererInit(mContext *ctx) {
    if (!ctx) return;

    ctx->renderer.vertexBuffer = (mVertex*)malloc(sizeof(mVertex) * MAX_BATCH_VERTICES);
    ctx->renderer.vertexBufferPtr = ctx->renderer.vertexBuffer;
    ctx->renderer.indexBuffer = (unsigned int*)malloc(sizeof(unsigned int) * MAX_BATCH_INDICES);
    ctx->renderer.indexBufferPtr = ctx->renderer.indexBuffer;

    glGenVertexArrays(1, &ctx->renderer.vao);
    glBindVertexArray(ctx->renderer.vao);

    glGenBuffers(1, &ctx->renderer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mVertex) * MAX_BATCH_VERTICES, NULL, GL_DYNAMIC_DRAW);

    // Attribute 0: position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mVertex), (const void*)offsetof(mVertex, position));

    // Attribute 1: color (Color / 4 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(mVertex), (const void*)offsetof(mVertex, color));

    // Attribute 2: normals (vec3)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(mVertex), (const void*)offsetof(mVertex, normals));

    // Attribute 3: texCoord (vec2)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(mVertex), (const void*)offsetof(mVertex, texCoord));

    // Attribute 4: texId (float)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(mVertex), (const void*)offsetof(mVertex, texId));

    glGenBuffers(1, &ctx->renderer.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->renderer.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * MAX_BATCH_INDICES, NULL, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    // Create 1x1 solid white texture for untextured geometry
    mTexture whiteTex = mCreateColorTexture(WHITE);
    ctx->renderer.whiteTexture = whiteTex.id;

    // A complete fallback sampler is required even when the shader skips shadows.
    float noOccluder = 1.0f;
    glGenTextures(1, &ctx->renderer.shadowTexture);
    glActiveTexture(GL_TEXTURE0 + MAX_TEXTURE_SLOTS);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ctx->renderer.shadowTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, 1, 1, 1, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, &noOccluder);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);

    glm_mat4_identity(ctx->renderer.viewMatrix);
    glm_mat4_identity(ctx->renderer.projectionMatrix);
    glm_mat4_identity(ctx->renderer.modelMatrix);

    memset(&ctx->renderer.stats, 0, sizeof(mRendererStats));
    ctx->renderer.isBatching = 0;
    ctx->renderer.ambientLight = 0.12f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    mRendererStartBatch(ctx);
}
void mRendererShutdown(mContext *ctx) {
    if (!ctx) return;
    mRendererDiscardBatches(ctx);
    mLightingShutdown(ctx);

    if (ctx->renderer.vertexBuffer) {
        free(ctx->renderer.vertexBuffer);
        ctx->renderer.vertexBuffer = NULL;
        ctx->renderer.vertexBufferPtr = NULL;
    }

    if (ctx->renderer.indexBuffer) {
        free(ctx->renderer.indexBuffer);
        ctx->renderer.indexBuffer = NULL;
        ctx->renderer.indexBufferPtr = NULL;
    }

    if (ctx->renderer.vao) { glDeleteVertexArrays(1, &ctx->renderer.vao); ctx->renderer.vao = 0; }
    if (ctx->renderer.vbo) { glDeleteBuffers(1, &ctx->renderer.vbo); ctx->renderer.vbo = 0; }
    if (ctx->renderer.ebo) { glDeleteBuffers(1, &ctx->renderer.ebo); ctx->renderer.ebo = 0; }
    if (ctx->renderer.whiteTexture) { mDeleteTextureId(ctx->renderer.whiteTexture); ctx->renderer.whiteTexture = 0; }
}
void mRendererDiscardBatches(mContext* ctx) {
    mRenderBatch* batch = ctx->renderer.firstBatch;
    while (batch) {
        mRenderBatch* next = batch->next;
        free(batch->vertices);
        free(batch->indices);
        free(batch);
        batch = next;
    }
    ctx->renderer.firstBatch = ctx->renderer.lastBatch = NULL;
}

void mRendererBegin(mContext *ctx) {
    if (!ctx) return;
    if (ctx->renderer.isBatching) mRendererEnd(ctx);
    ctx->renderer.isBatching = 1;
    mRendererStartBatch(ctx);
}

void mRendererFlush(mContext *ctx) {
    if (!ctx || ctx->renderer.indexCount == 0) return;
    mRenderer* r = &ctx->renderer;
    mRenderBatch* batch = calloc(1, sizeof(*batch));
    if (batch) {
        batch->vertices = malloc(r->vertexCount * sizeof(mVertex));
        batch->indices = malloc(r->indexCount * sizeof(unsigned int));
    }
    if (!batch || !batch->vertices || !batch->indices) {
        fprintf(stderr, "Peach: unable to allocate scene batch.\n");
        if (batch) { free(batch->vertices); free(batch->indices); free(batch); }
        mRendererStartBatch(ctx);
        return;
    }
    batch->vertexCount = r->vertexCount;
    batch->indexCount = r->indexCount;
    memcpy(batch->vertices, r->vertexBuffer, r->vertexCount * sizeof(mVertex));
    memcpy(batch->indices, r->indexBuffer, r->indexCount * sizeof(unsigned int));
    memcpy(batch->textures, r->textureSlots, sizeof(batch->textures));
    batch->textureCount = r->textureSlotIndex;
    batch->shaderProgram = r->shaderProgram;
    glm_mat4_copy(r->modelMatrix, batch->model);
    glm_mat4_copy(r->viewMatrix, batch->view);
    glm_mat4_copy(r->projectionMatrix, batch->projection);
    glGetIntegerv(GL_VIEWPORT, batch->viewport);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&batch->framebuffer);
    batch->depthTest = glIsEnabled(GL_DEPTH_TEST);
    batch->cullFace = glIsEnabled(GL_CULL_FACE);
    batch->blend = glIsEnabled(GL_BLEND);
    glGetIntegerv(GL_DEPTH_FUNC, &batch->depthFunc);
    glGetIntegerv(GL_CULL_FACE_MODE, &batch->cullMode);
    glGetIntegerv(GL_FRONT_FACE, &batch->frontFace);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &batch->depthWrite);
    if (r->lastBatch) r->lastBatch->next = batch;
    else r->firstBatch = batch;
    r->lastBatch = batch;
    mRendererStartBatch(ctx);
    if (!r->isBatching) {
        mLightingRenderScene(ctx);
        mRendererDiscardBatches(ctx);
    }
}

void mRendererEnd(mContext *ctx) {
    if (!ctx) return;
    // Keep Flush from drawing before all the shadow casters have been collected.
    ctx->renderer.isBatching = 1;
    mRendererFlush(ctx);
    mLightingRenderScene(ctx);
    mRendererDiscardBatches(ctx);
    ctx->renderer.isBatching = 0;
}
void mRendererClear(mContext *ctx) {
    if (!ctx) return;
    glClearColor(ctx->window.clearColor.r, ctx->window.clearColor.g, ctx->window.clearColor.b, ctx->window.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void mRendererClearColor(mColor color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void mRendererSetDepthTest(int enable) {
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}
void mRendererSetCullFace(int enable) {
    if (enable) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}
void mRendererSetProjection(mContext *ctx, mat4 proj) {
    if (!ctx) return;
    mRendererFlush(ctx);
    glm_mat4_copy(proj, ctx->renderer.projectionMatrix);
}
void mRendererSetView(mContext *ctx, mat4 view) {
    if (!ctx) return;
    mRendererFlush(ctx);
    glm_mat4_copy(view, ctx->renderer.viewMatrix);
}
void mRendererSetModel(mContext *ctx, mat4 model) {
    if (!ctx) return;
    mRendererFlush(ctx);
    glm_mat4_copy(model, ctx->renderer.modelMatrix);
}
void mRendererSetCamera2D(mContext *ctx, float left, float right, float bottom, float top) {
    if (!ctx) return;
    mRendererFlush(ctx);
    glm_ortho(left, right, bottom, top, -1.0f, 1.0f, ctx->renderer.projectionMatrix);
    glm_mat4_identity(ctx->renderer.viewMatrix);
    glm_mat4_identity(ctx->renderer.modelMatrix);
}
void mRendererSetCamera3D(mContext *ctx, const mCamera3D *camera) {
    if (!ctx || !camera) return;
    mRendererFlush(ctx);
    float aspect = 0.0f;
    if (ctx->window.height > 0) {
        aspect = (float)ctx->window.width / (float)ctx->window.height;
    }
    mCameraGetViewMatrix(camera, ctx->renderer.viewMatrix);
    mCameraGetProjectionMatrix(camera, aspect, ctx->renderer.projectionMatrix);
    glm_mat4_identity(ctx->renderer.modelMatrix);
}
void mRendererSetCamera(mContext *ctx, const mCamera3D *camera) {
    mRendererSetCamera3D(ctx, camera);
}
void mRendererSetPerspective(mContext *ctx, float fovDeg, float aspect, float nearVal, float farVal) {
    if (!ctx) return;
    mRendererFlush(ctx);
    float actualAspect = aspect;
    if (actualAspect <= 0.0f && ctx->window.height > 0) {
        actualAspect = (float)ctx->window.width / (float)ctx->window.height;
    }
    if (actualAspect <= 0.0f) actualAspect = 800.0f / 600.0f;
    glm_perspective(glm_rad(fovDeg), actualAspect, nearVal, farVal, ctx->renderer.projectionMatrix);
}
void mRendererSetLookAt(mContext *ctx, vec3 eye, vec3 center, vec3 up) {
    if (!ctx) return;
    mRendererFlush(ctx);
    glm_lookat(eye, center, up, ctx->renderer.viewMatrix);
}
int mRendererBindTexture(mContext* ctx, const mTexture* texture) {
    if (!texture) return 0;
    return mRendererBindTextureId(ctx, texture->id);
}
int mRendererBindTextureId(mContext* ctx, unsigned int textureId) {
    if (!ctx) return 0;
    if (textureId == 0 || textureId == ctx->renderer.whiteTexture) return 0;

    for (unsigned int i = 1; i < ctx->renderer.textureSlotIndex; i++) {
        if (ctx->renderer.textureSlots[i] == textureId) {
            return (int)i;
        }
    }

    if (ctx->renderer.textureSlotIndex >= MAX_TEXTURE_SLOTS) {
        mRendererFlush(ctx);
    }

    int slot = (int)ctx->renderer.textureSlotIndex;
    ctx->renderer.textureSlots[slot] = textureId;
    ctx->renderer.textureSlotIndex++;
    return slot;
}
static inline float resolveVertexTexId(mContext* ctx, float rawTexId) {
    unsigned int texHandle = (unsigned int)roundf(rawTexId);
    if (texHandle == 0 || texHandle == ctx->renderer.whiteTexture) {
        return 0.0f;
    }
    int slot = mRendererBindTextureId(ctx, texHandle);
    return (float)slot;
}

// Reserve textures before storing any indices: binding a new texture may seal a batch.
static void prepareVertices(mContext* ctx, const mVertex* vertices, unsigned int count,
                            unsigned int indexCount) {
    unsigned int textures[MAX_TEXTURE_SLOTS];
    unsigned int textureCount = ctx->renderer.textureSlotIndex;
    memcpy(textures, ctx->renderer.textureSlots, sizeof(textures));
    int flush = ctx->renderer.vertexCount + count > MAX_BATCH_VERTICES ||
                ctx->renderer.indexCount + indexCount > MAX_BATCH_INDICES;
    for (unsigned int i = 0; i < count && !flush; ++i) {
        unsigned int id = (unsigned int)roundf(vertices[i].texId);
        if (!id) continue;
        unsigned int slot = 0;
        while (slot < textureCount && textures[slot] != id) ++slot;
        if (slot == textureCount) {
            if (textureCount == MAX_TEXTURE_SLOTS) flush = 1;
            else textures[textureCount++] = id;
        }
    }
    if (flush) mRendererFlush(ctx);
    for (unsigned int i = 0; i < count; ++i)
        mRendererBindTextureId(ctx, (unsigned int)roundf(vertices[i].texId));
}

void mAddVertices(mContext* ctx, const mVertex* vertices, unsigned int count) {
    if (!ctx || !vertices || !count) return;
    ctx->renderer.isBatching = 1;
    // Reserve a full triangle so a capacity/texture boundary cannot split it.
    for (unsigned int first = 0; first < count; first += 3) {
        unsigned int group = count - first < 3 ? count - first : 3;
        prepareVertices(ctx, vertices + first, group, group);
        for (unsigned int i = first; i < first + group; ++i) {
            mVertex v = vertices[i];
            v.texId = resolveVertexTexId(ctx, v.texId);
            uint32_t bucket = hashVertex(&v) & VERTEX_HASH_MASK;
            int foundIndex = -1;
            for (int curr = s_vertexHashTable[bucket]; curr != -1; curr = s_vertexNext[curr]) {
                if (vertexEqual(&ctx->renderer.vertexBuffer[curr], &v)) {
                    foundIndex = curr;
                    break;
                }
            }
            if (foundIndex == -1) {
                foundIndex = (int)ctx->renderer.vertexCount++;
                *ctx->renderer.vertexBufferPtr++ = v;
                s_vertexNext[foundIndex] = s_vertexHashTable[bucket];
                s_vertexHashTable[bucket] = foundIndex;
                ctx->renderer.stats.vertexCount++;
            }
            *ctx->renderer.indexBufferPtr++ = (unsigned int)foundIndex;
            ctx->renderer.indexCount++;
            ctx->renderer.stats.indexCount++;
        }
    }
}

void mAddVerticesIndexed(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                         const unsigned int* indices, unsigned int indexCount) {
    if (!ctx || !vertices || !vertexCount || !indices || !indexCount) return;
    for (unsigned int i = 0; i < indexCount; ++i) {
        if (indices[i] >= vertexCount) {
            fprintf(stderr, "Peach: vertex index out of bounds.\n");
            return;
        }
    }
    ctx->renderer.isBatching = 1;
    // Large or texture-heavy meshes are split on triangle boundaries.
    unsigned int unique[MAX_TEXTURE_SLOTS], uniqueCount = 0;
    int split = vertexCount > MAX_BATCH_VERTICES || indexCount > MAX_BATCH_INDICES;
    for (unsigned int i = 0; i < vertexCount && !split; ++i) {
        unsigned int id = (unsigned int)roundf(vertices[i].texId);
        if (!id || id == ctx->renderer.whiteTexture) continue;
        unsigned int slot = 0;
        while (slot < uniqueCount && unique[slot] != id) ++slot;
        if (slot == uniqueCount) {
            if (uniqueCount == MAX_TEXTURE_SLOTS - 1) split = 1;
            else unique[uniqueCount++] = id;
        }
    }
    if (split) {
        for (unsigned int i = 0; i + 2 < indexCount; i += 3) {
            mVertex triangle[3] = {vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]]};
            mAddVertices(ctx, triangle, 3);
        }
        return;
    }
    prepareVertices(ctx, vertices, vertexCount, indexCount);
    unsigned int baseVertex = ctx->renderer.vertexCount;
    for (unsigned int i = 0; i < vertexCount; ++i) {
        mVertex v = vertices[i];
        v.texId = resolveVertexTexId(ctx, v.texId);
        unsigned int newIdx = ctx->renderer.vertexCount++;
        *ctx->renderer.vertexBufferPtr++ = v;
        uint32_t bucket = hashVertex(&v) & VERTEX_HASH_MASK;
        s_vertexNext[newIdx] = s_vertexHashTable[bucket];
        s_vertexHashTable[bucket] = (int)newIdx;
    }
    for (unsigned int i = 0; i < indexCount; ++i)
        *ctx->renderer.indexBufferPtr++ = baseVertex + indices[i];
    ctx->renderer.indexCount += indexCount;
    ctx->renderer.stats.vertexCount += vertexCount;
    ctx->renderer.stats.indexCount += indexCount;
}
void mAddVertex(mContext* ctx, mVertex vertex) {
    mAddVertices(ctx, &vertex, 1);
}

void mAddTriangle(mContext* ctx, mVertex v1, mVertex v2, mVertex v3) {
    mVertex triangle[3] = { v1, v2, v3 };
    mAddVertices(ctx, triangle, 3);
}
void mAddQuad(mContext* ctx, mVertex v1, mVertex v2, mVertex v3, mVertex v4) {
    mVertex quadVertices[4] = { v1, v2, v3, v4 };
    unsigned int quadIndices[6] = { 0, 1, 2, 2, 3, 0 };
    mAddVerticesIndexed(ctx, quadVertices, 4, quadIndices, 6);
}
// Rotate before batching so every object can retain its own orientation.
static void rotateVertices(mVertex* vertices, unsigned int count, vec3 center, vec3 rotation) {
    if (rotation[0] == 0.0f && rotation[1] == 0.0f && rotation[2] == 0.0f) return;

    mat4 transform = GLM_MAT4_IDENTITY_INIT;
    // Column vectors: Rz * Ry * Rx applies X, then Y, then Z.
    glm_rotate_z(transform, rotation[2], transform);
    glm_rotate_y(transform, rotation[1], transform);
    glm_rotate_x(transform, rotation[0], transform);

    for (unsigned int i = 0; i < count; i++) {
        vec3 offset;
        glm_vec3_sub(vertices[i].position, center, offset);
        glm_mat4_mulv3(transform, offset, 0.0f, offset);
        glm_vec3_add(center, offset, vertices[i].position);
        glm_mat4_mulv3(transform, vertices[i].normals, 0.0f, vertices[i].normals);
    }
}

void mAddQuad3DTextured(mContext* ctx, vec3 center, vec2 size, vec3 normal, const mTexture* texture, mColor tint) {
    mAddQuad3DTexturedRotated(ctx, center, size, normal, (vec3){0.0f, 0.0f, 0.0f}, texture, tint);
}

void mAddQuad3DRotated(mContext* ctx, vec3 center, vec2 size, vec3 normal, vec3 rotation, mColor color) {
    mAddQuad3DTexturedRotated(ctx, center, size, normal, rotation, NULL, color);
}

void mAddQuad3DTexturedRotated(mContext* ctx, vec3 center, vec2 size, vec3 normal, vec3 rotation, const mTexture* texture, mColor tint) {
    if (!ctx) return;

    vec3 norm;
    glm_vec3_copy(normal, norm);
    if (glm_vec3_norm2(norm) < 1e-6f) {
        norm[0] = 0.0f; norm[1] = 0.0f; norm[2] = 1.0f;
    } else {
        glm_vec3_normalize(norm);
    }

    vec3 right, up;
    vec3 tempUp = { 0.0f, 1.0f, 0.0f };
    if (fabsf(norm[1]) > 0.99f) {
        tempUp[0] = 1.0f; tempUp[1] = 0.0f; tempUp[2] = 0.0f;
    }
    glm_vec3_cross(tempUp, norm, right);
    glm_vec3_normalize(right);
    glm_vec3_cross(norm, right, up);
    glm_vec3_normalize(up);

    float halfW = size[0] * 0.5f;
    float halfH = size[1] * 0.5f;

    vec3 wVec, hVec;
    glm_vec3_scale(right, halfW, wVec);
    glm_vec3_scale(up, halfH, hVec);

    vec3 p0, p1, p2, p3;
    glm_vec3_sub(center, wVec, p0); glm_vec3_sub(p0, hVec, p0);
    glm_vec3_add(center, wVec, p1); glm_vec3_sub(p1, hVec, p1);
    glm_vec3_add(center, wVec, p2); glm_vec3_add(p2, hVec, p2);
    glm_vec3_sub(center, wVec, p3); glm_vec3_add(p3, hVec, p3);

    float texId = texture ? (float)texture->id : 0.0f;

    mVertex vertices[4] = {
        { .position = { p0[0], p0[1], p0[2] }, .color = tint, .normals = { norm[0], norm[1], norm[2] }, .texCoord = { 0.0f, 0.0f }, .texId = texId },
        { .position = { p1[0], p1[1], p1[2] }, .color = tint, .normals = { norm[0], norm[1], norm[2] }, .texCoord = { 1.0f, 0.0f }, .texId = texId },
        { .position = { p2[0], p2[1], p2[2] }, .color = tint, .normals = { norm[0], norm[1], norm[2] }, .texCoord = { 1.0f, 1.0f }, .texId = texId },
        { .position = { p3[0], p3[1], p3[2] }, .color = tint, .normals = { norm[0], norm[1], norm[2] }, .texCoord = { 0.0f, 1.0f }, .texId = texId }
    };
    unsigned int indices[6] = { 0, 1, 2, 2, 3, 0 };
    rotateVertices(vertices, 4, center, rotation);
    mAddVerticesIndexed(ctx, vertices, 4, indices, 6);
}

void mAddQuad3D(mContext* ctx, vec3 center, vec2 size, vec3 normal, mColor color) {
    mAddQuad3DTextured(ctx, center, size, normal, NULL, color);
}
void mAddCubeTextured(mContext* ctx, vec3 position, vec3 size, const mTexture* texture, mColor tint) {
    mAddCubeTexturedRotated(ctx, position, size, (vec3){0.0f, 0.0f, 0.0f}, texture, tint);
}

void mAddCubeRotated(mContext* ctx, vec3 position, vec3 size, vec3 rotation, mColor color) {
    mAddCubeTexturedRotated(ctx, position, size, rotation, NULL, color);
}

void mAddCubeTexturedRotated(mContext* ctx, vec3 position, vec3 size, vec3 rotation, const mTexture* texture, mColor tint) {
    if (!ctx) return;

    float hx = size[0] * 0.5f;
    float hy = size[1] * 0.5f;
    float hz = size[2] * 0.5f;
    float x = position[0], y = position[1], z = position[2];
    float texId = texture ? (float)texture->id : 0.0f;

    mVertex vertices[24] = {
        // Front (+Z)
        { .position = { x - hx, y - hy, z + hz }, .color = tint, .normals = { 0.0f, 0.0f, 1.0f }, .texCoord = { 0.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y - hy, z + hz }, .color = tint, .normals = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y + hy, z + hz }, .color = tint, .normals = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 1.0f }, .texId = texId },
        { .position = { x - hx, y + hy, z + hz }, .color = tint, .normals = { 0.0f, 0.0f, 1.0f }, .texCoord = { 0.0f, 1.0f }, .texId = texId },

        // Back (-Z)
        { .position = { x + hx, y - hy, z - hz }, .color = tint, .normals = { 0.0f, 0.0f, -1.0f }, .texCoord = { 0.0f, 0.0f }, .texId = texId },
        { .position = { x - hx, y - hy, z - hz }, .color = tint, .normals = { 0.0f, 0.0f, -1.0f }, .texCoord = { 1.0f, 0.0f }, .texId = texId },
        { .position = { x - hx, y + hy, z - hz }, .color = tint, .normals = { 0.0f, 0.0f, -1.0f }, .texCoord = { 1.0f, 1.0f }, .texId = texId },
        { .position = { x + hx, y + hy, z - hz }, .color = tint, .normals = { 0.0f, 0.0f, -1.0f }, .texCoord = { 0.0f, 1.0f }, .texId = texId },

        // Top (+Y)
        { .position = { x - hx, y + hy, z + hz }, .color = tint, .normals = { 0.0f, 1.0f, 0.0f }, .texCoord = { 0.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y + hy, z + hz }, .color = tint, .normals = { 0.0f, 1.0f, 0.0f }, .texCoord = { 1.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y + hy, z - hz }, .color = tint, .normals = { 0.0f, 1.0f, 0.0f }, .texCoord = { 1.0f, 1.0f }, .texId = texId },
        { .position = { x - hx, y + hy, z - hz }, .color = tint, .normals = { 0.0f, 1.0f, 0.0f }, .texCoord = { 0.0f, 1.0f }, .texId = texId },

        // Bottom (-Y)
        { .position = { x - hx, y - hy, z - hz }, .color = tint, .normals = { 0.0f, -1.0f, 0.0f }, .texCoord = { 0.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y - hy, z - hz }, .color = tint, .normals = { 0.0f, -1.0f, 0.0f }, .texCoord = { 1.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y - hy, z + hz }, .color = tint, .normals = { 0.0f, -1.0f, 0.0f }, .texCoord = { 1.0f, 1.0f }, .texId = texId },
        { .position = { x - hx, y - hy, z + hz }, .color = tint, .normals = { 0.0f, -1.0f, 0.0f }, .texCoord = { 0.0f, 1.0f }, .texId = texId },

        // Right (+X)
        { .position = { x + hx, y - hy, z + hz }, .color = tint, .normals = { 1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y - hy, z - hz }, .color = tint, .normals = { 1.0f, 0.0f, 0.0f }, .texCoord = { 1.0f, 0.0f }, .texId = texId },
        { .position = { x + hx, y + hy, z - hz }, .color = tint, .normals = { 1.0f, 0.0f, 0.0f }, .texCoord = { 1.0f, 1.0f }, .texId = texId },
        { .position = { x + hx, y + hy, z + hz }, .color = tint, .normals = { 1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 1.0f }, .texId = texId },

        // Left (-X)
        { .position = { x - hx, y - hy, z - hz }, .color = tint, .normals = { -1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 0.0f }, .texId = texId },
        { .position = { x - hx, y - hy, z + hz }, .color = tint, .normals = { -1.0f, 0.0f, 0.0f }, .texCoord = { 1.0f, 0.0f }, .texId = texId },
        { .position = { x - hx, y + hy, z + hz }, .color = tint, .normals = { -1.0f, 0.0f, 0.0f }, .texCoord = { 1.0f, 1.0f }, .texId = texId },
        { .position = { x - hx, y + hy, z - hz }, .color = tint, .normals = { -1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 1.0f }, .texId = texId }
    };

    unsigned int indices[36];
    for (unsigned int face = 0; face < 6; face++) {
        unsigned int vOffset = face * 4;
        unsigned int iOffset = face * 6;
        indices[iOffset + 0] = vOffset + 0;
        indices[iOffset + 1] = vOffset + 1;
        indices[iOffset + 2] = vOffset + 2;
        indices[iOffset + 3] = vOffset + 2;
        indices[iOffset + 4] = vOffset + 3;
        indices[iOffset + 5] = vOffset + 0;
    }

    rotateVertices(vertices, 24, position, rotation);
    mAddVerticesIndexed(ctx, vertices, 24, indices, 36);
}
void mAddCube(mContext* ctx, vec3 position, vec3 size, mColor color) {
    mAddCubeTextured(ctx, position, size, NULL, color);
}

mRendererStats mRendererGetStats(mContext* ctx) {
    if (!ctx) {
        mRendererStats empty = {0};
        return empty;
    }
    return ctx->renderer.stats;
}
void mRendererResetStats(mContext* ctx) {
    if (ctx) {
        memset(&ctx->renderer.stats, 0, sizeof(mRendererStats));
    }
}

void mInitRenderer(mContext* ctx) { mRendererInit(ctx); }
void mBeginBatch(mContext* ctx) { mRendererBegin(ctx); }
void mEndBatch(mContext* ctx) { mRendererEnd(ctx); }
void mFlushBatch(mContext* ctx) { mRendererFlush(ctx); }

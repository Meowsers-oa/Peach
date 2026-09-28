#include <Peach/Renderer.h>
#include <Peach/Shader.h>
#include "TransformInternal.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mRendererCreate(mContext* ctx) {
    if (ctx == NULL || ctx->window.handle == NULL || ctx->renderer != NULL) return M_FAILURE;
    mRenderer* renderer = calloc(1, sizeof(mRenderer));
    if (renderer == NULL) return M_FAILURE;
    ctx->renderer = renderer;

    int textureLimit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureLimit);
    if (mShaderLoad(&renderer->shader, PEACH_SHADER_DIR "/Batch2D.vert",
                    PEACH_SHADER_DIR "/Batch2D.frag") == M_FAILURE) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    // Match batching to the sampler array actually declared in the shader file.
    const char* samplerName = "uTextures[0]";
    unsigned int samplerIndex;
    glGetUniformIndices(renderer->shader.handle, 1, &samplerName, &samplerIndex);
    int samplerCount = 0;
    int samplerType = 0;
    if (samplerIndex != GL_INVALID_INDEX) {
        glGetActiveUniformsiv(renderer->shader.handle, 1, &samplerIndex, GL_UNIFORM_SIZE, &samplerCount);
        glGetActiveUniformsiv(renderer->shader.handle, 1, &samplerIndex, GL_UNIFORM_TYPE, &samplerType);
    }
    renderer->viewProjectionLocation = glGetUniformLocation(renderer->shader.handle, "uViewProjection");
    if (samplerCount < 2 || samplerCount > M_RENDERER_MAX_TEXTURES || samplerCount > textureLimit ||
        samplerType != GL_SAMPLER_2D || renderer->viewProjectionLocation < 0) {
        fprintf(stderr, "Invalid batch shader uniforms or texture capacity.\n");
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    renderer->textureLimit = (unsigned int)samplerCount;
    int samplers[M_RENDERER_MAX_TEXTURES];
    for (unsigned int i = 0; i < renderer->textureLimit; i++) samplers[i] = (int)i;
    mShaderUse(&renderer->shader);
    glUniform1iv(glGetUniformLocation(renderer->shader.handle, "uTextures[0]"), (int)renderer->textureLimit, samplers);

    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    glGenBuffers(1, &renderer->ebo);
    if (renderer->vao == 0 || renderer->vbo == 0 || renderer->ebo == 0) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->vertices), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(renderer->indices), NULL, GL_STREAM_DRAW);
    int vertexBytes, indexBytes;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vertexBytes);
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexBytes);
    if ((size_t)vertexBytes != sizeof(renderer->vertices) || (size_t)indexBytes != sizeof(renderer->indices)) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(mBatchVertex), (void*)offsetof(mBatchVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(mBatchVertex), (void*)offsetof(mBatchVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(mBatchVertex), (void*)offsetof(mBatchVertex, color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(mBatchVertex), (void*)offsetof(mBatchVertex, textureSlot));

    unsigned char white[] = {255, 255, 255, 255};
    if (mTextureCreate(&renderer->whiteTexture, 1, 1, white) == M_FAILURE) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    renderer->textures[0] = renderer->whiteTexture.handle;
    renderer->textureCount = 1;
    mRendererBegin(ctx);
    return M_SUCCESS;
}

void mRendererDestroy(mContext* ctx) {
    if (ctx == NULL || ctx->renderer == NULL) return;
    mRenderer* renderer = ctx->renderer;
    // Pending geometry is discarded during destruction.
    glDeleteTextures(1, &renderer->whiteTexture.handle);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->ebo);
    glDeleteVertexArrays(1, &renderer->vao);
    mShaderDestroy(&renderer->shader);
    free(renderer);
    ctx->renderer = NULL;
}

static void updateProjection(mContext* ctx) {
    mRenderer* renderer = ctx->renderer;
    int width, height;
    glfwGetWindowSize(ctx->window.handle, &width, &height);
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    mCamera camera;
    if (mCameraCreate(&camera, (float)width, (float)height) == M_SUCCESS) {
        memcpy(renderer->viewProjection, camera.viewProjection, sizeof(renderer->viewProjection));
    }
}

void mRendererBegin(mContext* ctx) {
    if (ctx == NULL || ctx->renderer == NULL) return;
    mRendererFlush(ctx);
    int width, height;
    glfwGetFramebufferSize(ctx->window.handle, &width, &height);
    glViewport(0, 0, width, height);
    if (!ctx->renderer->customCamera) updateProjection(ctx);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(ctx->window.bgColor.r, ctx->window.bgColor.g, ctx->window.bgColor.b, ctx->window.bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void mRendererSetViewProjection(mContext* ctx, const float* viewProjection) {
    if (ctx == NULL || ctx->renderer == NULL) return;
    mRendererFlush(ctx);
    ctx->renderer->customCamera = viewProjection != NULL;
    if (viewProjection != NULL) {
        memcpy(ctx->renderer->viewProjection, viewProjection, sizeof(ctx->renderer->viewProjection));
    } else {
        updateProjection(ctx);
    }
}

void mRendererSetCamera(mContext* ctx, const mCamera* camera) {
    mRendererSetViewProjection(ctx, camera != NULL ? camera->viewProjection : NULL);
}

void mRendererFlush(mContext* ctx) {
    if (ctx == NULL || ctx->renderer == NULL || ctx->renderer->indexCount == 0) return;
    mRenderer* renderer = ctx->renderer;
    mShaderUse(&renderer->shader);
    glUniformMatrix4fv(renderer->viewProjectionLocation, 1, GL_FALSE, renderer->viewProjection);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->vertices), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->vertexCount * sizeof(mBatchVertex), renderer->vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(renderer->indices), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, renderer->indexCount * sizeof(unsigned int), renderer->indices);
    for (unsigned int i = 0; i < renderer->textureLimit; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        unsigned int texture = i < renderer->textureCount ? renderer->textures[i] : renderer->whiteTexture.handle;
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindSampler(i, 0);
    }

    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_PRIMITIVE_RESTART);
    glDisable(GL_RASTERIZER_DISCARD);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, (int)renderer->indexCount, GL_UNSIGNED_INT, NULL);

    renderer->vertexCount = 0;
    renderer->indexCount = 0;
    renderer->textureCount = 1;
}

static unsigned int prepareBatch(mContext* ctx, unsigned int vertexCount, unsigned int indexCount, unsigned int texture) {
    mRenderer* renderer = ctx->renderer;
    if (vertexCount > M_RENDERER_MAX_VERTICES - renderer->vertexCount ||
        indexCount > M_RENDERER_MAX_INDICES - renderer->indexCount) {
        mRendererFlush(ctx);
    }
    for (unsigned int i = 0; i < renderer->textureCount; i++) {
        if (renderer->textures[i] == texture) return i;
    }
    if (renderer->textureCount == renderer->textureLimit) mRendererFlush(ctx);
    unsigned int slot = renderer->textureCount++;
    renderer->textures[slot] = texture;
    return slot;
}

static void appendVertex(mRenderer* renderer, const mVertex* vertex, const float* transform, unsigned int textureSlot) {
    mBatchVertex* output = &renderer->vertices[renderer->vertexCount++];
    for (int row = 0; row < 4; row++) {
        output->position[row] = transform[row] * vertex->x + transform[4 + row] * vertex->y +
                                transform[8 + row] * vertex->z + transform[12 + row];
    }
    output->uv[0] = vertex->u;
    output->uv[1] = vertex->v;
    output->color = vertex->color;
    output->textureSlot = (float)textureSlot;
}

static int addVertices(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                       const unsigned int* indices, unsigned int indexCount,
                       const mTransform* transform, const mTexture* texture) {
    if (ctx == NULL || ctx->renderer == NULL) return M_FAILURE;
    if (indexCount == 0) return M_SUCCESS;
    if (vertices == NULL || vertexCount == 0 || indexCount % 3 != 0) return M_FAILURE;
    if (texture != NULL && texture->handle == 0) return M_FAILURE;
    // Validate the entire submission before any flush or partial draw.
    if (indices != NULL) {
        for (unsigned int i = 0; i < indexCount; i++) {
            if (indices[i] >= vertexCount) return M_FAILURE;
        }
    }

    float matrix[16];
    if (mTransformMatrix(transform, matrix) == M_FAILURE) return M_FAILURE;

    mRenderer* renderer = ctx->renderer;
    unsigned int handle = texture != NULL ? texture->handle : renderer->whiteTexture.handle;
    if (vertexCount <= M_RENDERER_MAX_VERTICES && indexCount <= M_RENDERER_MAX_INDICES) {
        unsigned int slot = prepareBatch(ctx, vertexCount, indexCount, handle);
        unsigned int base = renderer->vertexCount;
        for (unsigned int i = 0; i < vertexCount; i++) appendVertex(renderer, &vertices[i], matrix, slot);
        for (unsigned int i = 0; i < indexCount; i++) {
            renderer->indices[renderer->indexCount++] = base + (indices != NULL ? indices[i] : i);
        }
    } else {
        // Expand oversized lists one triangle at a time, preserving submission order.
        for (unsigned int i = 0; i < indexCount; i += 3) {
            unsigned int slot = prepareBatch(ctx, 3, 3, handle);
            for (unsigned int j = 0; j < 3; j++) {
                renderer->indices[renderer->indexCount++] = renderer->vertexCount;
                appendVertex(renderer, &vertices[indices != NULL ? indices[i + j] : i + j], matrix, slot);
            }
        }
    }
    return M_SUCCESS;
}

int mAddVertices(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                 const mTransform* transform, const mTexture* texture) {
    return addVertices(ctx, vertices, vertexCount, NULL, vertexCount, transform, texture);
}

int mAddVerticesIndexed(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                        const unsigned int* indices, unsigned int indexCount,
                        const mTransform* transform, const mTexture* texture) {
    if (indices == NULL && indexCount != 0) return M_FAILURE;
    return addVertices(ctx, vertices, vertexCount, indices, indexCount, transform, texture);
}

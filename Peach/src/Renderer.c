#include <Peach/Renderer.h>
#include <Peach/Shader.h>
#include <Peach/Map.h>
#include <math.h>
#include "../include/Peach/TransformInternal.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void destroyTarget(mRenderTarget* target) {
    glDeleteFramebuffers(1, &target->framebuffer);
    glDeleteTextures(1, &target->texture);
    *target = (mRenderTarget){0};
}

static int createTarget(mRenderTarget* target, int width, int height, int format) {
    *target = (mRenderTarget){.width = width, .height = height};
    glGenTextures(1, &target->texture);
    glGenFramebuffers(1, &target->framebuffer);
    if (target->texture == 0 || target->framebuffer == 0) return M_FAILURE;
    glBindTexture(GL_TEXTURE_2D, target->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->texture, 0);
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ? M_SUCCESS : M_FAILURE;
}

static int resizeTargets(mRenderer* renderer, int width, int height) {
    if (renderer->sceneTarget.width == width && renderer->sceneTarget.height == height) return M_SUCCESS;
    int maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if (width <= 0 || height <= 0 || width > maxSize || height > maxSize) return M_FAILURE;

    int unpackBuffer, texture, drawFramebuffer, readFramebuffer;
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    mRenderTarget scene = {0}, lighting = {0};
    int result = createTarget(&scene, width, height, GL_RGBA8);
    if (result == M_SUCCESS) result = createTarget(&lighting, width, height, GL_RGBA16F);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (unsigned int)unpackBuffer);
    glBindTexture(GL_TEXTURE_2D, (unsigned int)texture);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (unsigned int)drawFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, (unsigned int)readFramebuffer);
    if (result == M_FAILURE) {
        destroyTarget(&scene);
        destroyTarget(&lighting);
        return M_FAILURE;
    }
    destroyTarget(&renderer->sceneTarget);
    destroyTarget(&renderer->lightTarget);
    renderer->sceneTarget = scene;
    renderer->lightTarget = lighting;
    return M_SUCCESS;
}

static void setDrawState(void) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_PRIMITIVE_RESTART);
    glDisable(GL_RASTERIZER_DISCARD);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_DITHER);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

static mViewport outputViewport(mContext* ctx) {
    int width, height;
    glfwGetFramebufferSize(ctx->window.handle, &width, &height);
    mViewport viewport = {.width = width, .height = height};
    mRenderer* renderer = ctx->renderer;
    if (width <= 0 || height <= 0 || renderer->resolutionWidth == 0) return viewport;
    int windowWidth, windowHeight;
    glfwGetWindowSize(ctx->window.handle, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return viewport;
    viewport.width = (int)ceil(renderer->sceneTarget.width * renderer->pixelScale * width / windowWidth);
    viewport.height = (int)ceil(renderer->sceneTarget.height * renderer->pixelScale * height / windowHeight);
    viewport.y = height - viewport.height;
    return viewport;
}

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
    if (mShaderLoad(&renderer->lightShader, PEACH_SHADER_DIR "/Light2D.vert",
                    PEACH_SHADER_DIR "/Light2D.frag") == M_FAILURE ||
        mShaderLoad(&renderer->compositeShader, PEACH_SHADER_DIR "/Composite.vert",
                    PEACH_SHADER_DIR "/Composite.frag") == M_FAILURE) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    renderer->lightProjectionLocation = glGetUniformLocation(renderer->lightShader.handle, "uViewProjection");
    renderer->lightPositionLocation = glGetUniformLocation(renderer->lightShader.handle, "uLight");
    renderer->lightColorLocation = glGetUniformLocation(renderer->lightShader.handle, "uColor");
    int sceneLocation = glGetUniformLocation(renderer->compositeShader.handle, "uScene");
    int lightingLocation = glGetUniformLocation(renderer->compositeShader.handle, "uLighting");
    if (renderer->lightProjectionLocation < 0 || renderer->lightPositionLocation < 0 ||
        renderer->lightColorLocation < 0 || sceneLocation < 0 || lightingLocation < 0) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    mShaderUse(&renderer->compositeShader);
    glUniform1i(sceneLocation, 0);
    glUniform1i(lightingLocation, 1);
    renderer->ambient = M_COLOR_WHITE;
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
    if (mRendererSetResolution(ctx, 0, 0) == M_FAILURE) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
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
    mShaderDestroy(&renderer->lightShader);
    mShaderDestroy(&renderer->compositeShader);
    destroyTarget(&renderer->sceneTarget);
    destroyTarget(&renderer->lightTarget);
    free(renderer);
    ctx->renderer = NULL;
}

static void updateProjection(mContext* ctx) {
    mRenderer* renderer = ctx->renderer;
    int width, height;
    glfwGetWindowSize(ctx->window.handle, &width, &height);
    if (renderer->resolutionWidth != 0) {
        width = renderer->sceneTarget.width;
        height = renderer->sceneTarget.height;
    }
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
    mRenderer* renderer = ctx->renderer;
    int width = renderer->resolutionWidth;
    int height = renderer->resolutionHeight;
    if (width == 0) {
        glfwGetFramebufferSize(ctx->window.handle, &width, &height);
    } else {
        glfwGetWindowSize(ctx->window.handle, &width, &height);
        width = (int)ceil(width / renderer->pixelScale);
        height = (int)ceil(height / renderer->pixelScale);
    }
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    if (resizeTargets(renderer, width, height) == M_FAILURE) {
        fprintf(stderr, "Failed to resize render targets.\n");
        ctx->window.running = M_FALSE;
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->sceneTarget.framebuffer);
    glViewport(0, 0, width, height);
    if (!renderer->customCamera) updateProjection(ctx);
    setDrawState();
    glClearColor(ctx->window.bgColor.r, ctx->window.bgColor.g, ctx->window.bgColor.b, ctx->window.bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

int mRendererSetResolution(mContext* ctx, int width, int height) {
    if (ctx == NULL || ctx->renderer == NULL || width < 0 || height < 0 || (width == 0) != (height == 0)) return M_FAILURE;
    int targetWidth = width, targetHeight = height;
    double pixelScale = 1.0;
    if (width == 0) {
        glfwGetFramebufferSize(ctx->window.handle, &targetWidth, &targetHeight);
        if (targetWidth < 1) targetWidth = 1;
        if (targetHeight < 1) targetHeight = 1;
    } else {
        int windowWidth, windowHeight;
        glfwGetWindowSize(ctx->window.handle, &windowWidth, &windowHeight);
        if (windowWidth <= 0 || windowHeight <= 0) return M_FAILURE;
        pixelScale = fmin((double)windowWidth / width, (double)windowHeight / height);
        if (pixelScale >= 1.0) pixelScale = floor(pixelScale);
        targetWidth = (int)ceil(windowWidth / pixelScale);
        targetHeight = (int)ceil(windowHeight / pixelScale);
    }
    mRendererFlush(ctx);
    if (resizeTargets(ctx->renderer, targetWidth, targetHeight) == M_FAILURE) return M_FAILURE;
    ctx->renderer->pixelScale = pixelScale;
    ctx->renderer->resolutionWidth = width;
    ctx->renderer->resolutionHeight = height;
    mRendererBegin(ctx);
    return M_SUCCESS;
}

M_BOOL mRendererWindowToScreen(mContext* ctx, double x, double y, double* screenX, double* screenY) {
    if (ctx == NULL || ctx->renderer == NULL || !isfinite(x) || !isfinite(y)) return M_FALSE;
    int width, height;
    glfwGetWindowSize(ctx->window.handle, &width, &height);
    if (width <= 0 || height <= 0) return M_FALSE;
    double scale = ctx->renderer->resolutionWidth != 0 ? ctx->renderer->pixelScale : 1.0;
    if (screenX != NULL) *screenX = x / scale;
    if (screenY != NULL) *screenY = y / scale;
    return x >= 0 && y >= 0 && x < width && y < height;
}

void mRendererPresent(mContext* ctx) {
    if (ctx == NULL || ctx->renderer == NULL) return;
    mRendererFlush(ctx);
    mRenderer* renderer = ctx->renderer;
    mViewport viewport = outputViewport(ctx);
    if (viewport.width <= 0 || viewport.height <= 0) return;

    setDrawState();
    glBindVertexArray(renderer->vao);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->lightTarget.framebuffer);
    glViewport(0, 0, renderer->lightTarget.width, renderer->lightTarget.height);
    glClearColor(renderer->ambient.r, renderer->ambient.g, renderer->ambient.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    mShaderUse(&renderer->lightShader);
    glUniformMatrix4fv(renderer->lightProjectionLocation, 1, GL_FALSE, renderer->viewProjection);
    mMapIter iter = mMap_iter(ctx->lights);
    mMapEntry entry;
    while (mMap_next(&iter, &entry)) {
        if (entry.valueSize != sizeof(mLight)) continue;
        const mLight* light = entry.value;
        if (!light->enabled || !isfinite(light->x) || !isfinite(light->y) || !isfinite(light->radius) ||
            !isfinite(light->intensity) || light->radius <= 0 || light->intensity <= 0) continue;
        float r = light->color.r * light->intensity;
        float g = light->color.g * light->intensity;
        float b = light->color.b * light->intensity;
        if (!isfinite(r) || !isfinite(g) || !isfinite(b) || r < 0 || g < 0 || b < 0) continue;
        glUniform3f(renderer->lightPositionLocation, light->x, light->y, light->radius);
        glUniform3f(renderer->lightColorLocation, r, g, b);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
    mShaderUse(&renderer->compositeShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->sceneTarget.texture);
    glBindSampler(0, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderer->lightTarget.texture);
    glBindSampler(1, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
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
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->sceneTarget.framebuffer);
    glViewport(0, 0, renderer->sceneTarget.width, renderer->sceneTarget.height);
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
    setDrawState();
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

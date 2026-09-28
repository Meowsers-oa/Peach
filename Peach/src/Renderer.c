#include <Peach/Renderer.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float position[4];
    float uv[2];
    mColor color;
    float textureSlot;
}mBatchVertex;

struct mRenderer {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int program;
    int viewProjectionLocation;
    float viewProjection[16];
    int customCamera;
    mTexture whiteTexture;
    unsigned int textures[M_RENDERER_MAX_TEXTURES];
    unsigned int textureCount;
    unsigned int textureLimit;
    unsigned int vertexCount;
    unsigned int indexCount;
    mBatchVertex vertices[M_RENDERER_MAX_VERTICES];
    unsigned int indices[M_RENDERER_MAX_INDICES];
};

static unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    if (shader == 0) return 0;
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        printf("Failed to compile renderer shader: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static unsigned int createProgram(unsigned int textureLimit) {
    const char* vertexSource =
        "#version 410 core\n"
        "layout(location = 0) in vec4 aPosition;\n"
        "layout(location = 1) in vec2 aUV;\n"
        "layout(location = 2) in vec4 aColor;\n"
        "layout(location = 3) in float aTextureSlot;\n"
        "uniform mat4 uViewProjection;\n"
        "out vec2 vUV;\n"
        "out vec4 vColor;\n"
        "flat out int vTextureSlot;\n"
        "void main() {\n"
        "    gl_Position = uViewProjection * aPosition;\n"
        "    vUV = aUV;\n"
        "    vColor = aColor;\n"
        "    vTextureSlot = int(aTextureSlot);\n"
        "}\n";

    // Constant sampler indices work on OpenGL 4.1, including macOS drivers.
    char fragmentSource[8192];
    int length = snprintf(fragmentSource, sizeof(fragmentSource),
        "#version 410 core\n"
        "in vec2 vUV;\n"
        "in vec4 vColor;\n"
        "flat in int vTextureSlot;\n"
        "uniform sampler2D uTextures[%u];\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    vec2 dx = dFdx(vUV);\n"
        "    vec2 dy = dFdy(vUV);\n"
        "    vec4 sampled = vec4(1.0);\n"
        "    switch (vTextureSlot) {\n", textureLimit);
    for (unsigned int i = 0; i < textureLimit; i++) {
        length += snprintf(fragmentSource + length, sizeof(fragmentSource) - (size_t)length,
            "    case %u: sampled = textureGrad(uTextures[%u], vUV, dx, dy); break;\n", i, i);
    }
    snprintf(fragmentSource + length, sizeof(fragmentSource) - (size_t)length,
        "    }\n    fragColor = sampled * vColor;\n}\n");

    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertex == 0 || fragment == 0) {
        if (vertex != 0) glDeleteShader(vertex);
        if (fragment != 0) glDeleteShader(fragment);
        return 0;
    }

    unsigned int program = glCreateProgram();
    if (program != 0) {
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (program == 0) return 0;

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[2048];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        printf("Failed to link renderer shader: %s\n", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

int mRendererCreate(mContext* ctx) {
    if (ctx == NULL || ctx->window.handle == NULL || ctx->renderer != NULL) return M_FAILURE;
    mRenderer* renderer = calloc(1, sizeof(mRenderer));
    if (renderer == NULL) return M_FAILURE;
    ctx->renderer = renderer;

    int textureLimit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureLimit);
    if (textureLimit < 2) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    renderer->textureLimit = textureLimit < M_RENDERER_MAX_TEXTURES ? (unsigned int)textureLimit : M_RENDERER_MAX_TEXTURES;
    renderer->program = createProgram(renderer->textureLimit);
    if (renderer->program == 0) {
        mRendererDestroy(ctx);
        return M_FAILURE;
    }
    renderer->viewProjectionLocation = glGetUniformLocation(renderer->program, "uViewProjection");
    int samplers[M_RENDERER_MAX_TEXTURES];
    for (unsigned int i = 0; i < renderer->textureLimit; i++) samplers[i] = (int)i;
    glUseProgram(renderer->program);
    glUniform1iv(glGetUniformLocation(renderer->program, "uTextures[0]"), (int)renderer->textureLimit, samplers);

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
    glDeleteProgram(renderer->program);
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
    glUseProgram(renderer->program);
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
    if (transform != NULL) {
        for (int row = 0; row < 4; row++) {
            output->position[row] = transform[row] * vertex->x + transform[4 + row] * vertex->y +
                                    transform[8 + row] * vertex->z + transform[12 + row];
        }
    } else {
        output->position[0] = vertex->x;
        output->position[1] = vertex->y;
        output->position[2] = vertex->z;
        output->position[3] = 1.0f;
    }
    output->uv[0] = vertex->u;
    output->uv[1] = vertex->v;
    output->color = vertex->color;
    output->textureSlot = (float)textureSlot;
}

static int addVertices(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                       const unsigned int* indices, unsigned int indexCount,
                       const float* transform, const mTexture* texture) {
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

    mRenderer* renderer = ctx->renderer;
    unsigned int handle = texture != NULL ? texture->handle : renderer->whiteTexture.handle;
    if (vertexCount <= M_RENDERER_MAX_VERTICES && indexCount <= M_RENDERER_MAX_INDICES) {
        unsigned int slot = prepareBatch(ctx, vertexCount, indexCount, handle);
        unsigned int base = renderer->vertexCount;
        for (unsigned int i = 0; i < vertexCount; i++) appendVertex(renderer, &vertices[i], transform, slot);
        for (unsigned int i = 0; i < indexCount; i++) {
            renderer->indices[renderer->indexCount++] = base + (indices != NULL ? indices[i] : i);
        }
    } else {
        // Expand oversized lists one triangle at a time, preserving submission order.
        for (unsigned int i = 0; i < indexCount; i += 3) {
            unsigned int slot = prepareBatch(ctx, 3, 3, handle);
            for (unsigned int j = 0; j < 3; j++) {
                renderer->indices[renderer->indexCount++] = renderer->vertexCount;
                appendVertex(renderer, &vertices[indices != NULL ? indices[i + j] : i + j], transform, slot);
            }
        }
    }
    return M_SUCCESS;
}

int mAddVertices(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                 const float* transform, const mTexture* texture) {
    return addVertices(ctx, vertices, vertexCount, NULL, vertexCount, transform, texture);
}

int mAddVerticesIndexed(mContext* ctx, const mVertex* vertices, unsigned int vertexCount,
                        const unsigned int* indices, unsigned int indexCount,
                        const float* transform, const mTexture* texture) {
    if (indices == NULL && indexCount != 0) return M_FAILURE;
    return addVertices(ctx, vertices, vertexCount, indices, indexCount, transform, texture);
}

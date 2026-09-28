#include <Peach/Peach.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); exit(1); \
} } while (0)

static const unsigned int quadIndices[] = {0, 1, 2, 2, 3, 0};

static void makeQuad(mVertex* quad, float x, float y, float size, mColor color) {
    quad[0] = (mVertex){.x = x, .y = y, .u = 0, .v = 0, .color = color};
    quad[1] = (mVertex){.x = x + size, .y = y, .u = 1, .v = 0, .color = color};
    quad[2] = (mVertex){.x = x + size, .y = y + size, .u = 1, .v = 1, .color = color};
    quad[3] = (mVertex){.x = x, .y = y + size, .u = 0, .v = 1, .color = color};
}

static void pixel(mContext* ctx, int x, int y, int r, int g, int b) {
    int width, height, windowWidth, windowHeight;
    glfwGetFramebufferSize(ctx->window.handle, &width, &height);
    glfwGetWindowSize(ctx->window.handle, &windowWidth, &windowHeight);
    unsigned char result[4];
    glReadPixels(x * width / windowWidth, height - 1 - y * height / windowHeight,
                 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, result);
    CHECK(abs((int)result[0] - r) <= 3);
    CHECK(abs((int)result[1] - g) <= 3);
    CHECK(abs((int)result[2] - b) <= 3);
    CHECK(glGetError() == GL_NO_ERROR);
}

int main(void) {
    mContext ctx = mContextCreate();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    mWindowInfo info = {.width = 64, .height = 64, .title = "Renderer tests"};
    CHECK(mWindowCreate(&ctx, &info) == M_SUCCESS);
    // File loading, compilation/link failures, and program ownership.
    mShader shader = {0};
    CHECK(mShaderLoad(&shader, PEACH_TEST_SHADER_DIR "/missing.vert",
                      PEACH_TEST_SHADER_DIR "/Batch2D.frag") == M_FAILURE);
    CHECK(shader.handle == 0);
    CHECK(mShaderLoad(&shader, PEACH_TEST_SHADER_DIR "/Batch2D.vert",
                      PEACH_TEST_SHADER_FIXTURE_DIR "/Invalid.frag") == M_FAILURE);
    CHECK(shader.handle == 0);
    CHECK(mShaderLoad(&shader, PEACH_TEST_SHADER_DIR "/Batch2D.vert",
                      PEACH_TEST_SHADER_FIXTURE_DIR "/Mismatch.frag") == M_FAILURE);
    CHECK(shader.handle == 0);
    CHECK(mShaderLoad(&shader, PEACH_TEST_SHADER_DIR "/Batch2D.vert",
                      PEACH_TEST_SHADER_DIR "/Batch2D.frag") == M_SUCCESS);
    unsigned int program = shader.handle;
    CHECK(glIsProgram(program));
    CHECK(mShaderLoad(&shader, PEACH_TEST_SHADER_DIR "/Batch2D.vert",
                      PEACH_TEST_SHADER_DIR "/Batch2D.frag") == M_FAILURE);
    CHECK(shader.handle == program);
    mShaderUse(&shader);
    int activeProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
    CHECK((unsigned int)activeProgram == program);
    mShaderUse(NULL);
    glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
    CHECK(activeProgram == 0);
    mShaderUse(&shader);
    mShaderDestroy(&shader);
    CHECK(shader.handle == 0 && !glIsProgram(program));
    glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
    CHECK(activeProgram == 0);
    mShaderDestroy(&shader);
    CHECK(glGetError() == GL_NO_ERROR);

    ctx.window.bgColor = M_COLOR_BLACK;
    mRendererBegin(&ctx);

    // Non-indexed geometry, rotation, scale and translation in column-major order.
    mVertex triangle[] = {
        {.x = 0, .y = 0, .color = M_COLOR_RED},
        {.x = 8, .y = 0, .color = M_COLOR_RED},
        {.x = 0, .y = 8, .color = M_COLOR_RED}
    };
    mTransform transform = mTransformCreate();
    transform.position = (vec3s){.x = 32, .y = 10};
    transform.rotation.z = 1.57079632679f;
    transform.scale = (vec3s){.x = 2, .y = 2, .z = 1};
    CHECK(mAddVertices(&ctx, triangle, 3, &transform, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 28, 14, 255, 0, 0);
    pixel(&ctx, 4, 4, 0, 0, 0);

    // Multiple indexed submissions must rebase indices; texture tint and alpha order.
    mRendererBegin(&ctx);
    unsigned char green[] = {0, 255, 0, 255};
    mTexture texture = {0};
    CHECK(mTextureCreate(&texture, 1, 1, green) == M_SUCCESS);
    mVertex quad[4];
    makeQuad(quad, 0, 0, 16, M_COLOR_RED);
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, NULL) == M_SUCCESS);
    makeQuad(quad, 20, 0, 16, M_COLOR(1, .5f, 1, 1));
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, &texture) == M_SUCCESS);
    makeQuad(quad, 0, 0, 16, M_COLOR(0, 0, 1, .5f));
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 8, 8, 128, 0, 128);
    pixel(&ctx, 28, 8, 0, 128, 0);

    // UV orientation and RGBA upload must be independent of caller unpack state.
    unsigned char image[] = {255, 0, 0, 255, 0, 255, 0, 255,
                             0, 0, 255, 255, 255, 255, 255, 255};
    mTexture imageTexture = {0};
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 5);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 1);
    CHECK(mTextureCreate(&imageTexture, 2, 2, image) == M_SUCCESS);
    int textureBinding, minFilter, magFilter;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
    glBindTexture(GL_TEXTURE_2D, imageTexture.handle);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &magFilter);
    CHECK(minFilter == GL_NEAREST && magFilter == GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, (unsigned int)textureBinding);
    int rowLength, skipRows;
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
    CHECK(rowLength == 5 && skipRows == 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    mRendererBegin(&ctx);
    makeQuad(quad, 0, 0, 32, M_COLOR_WHITE);
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, &imageTexture) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 4, 4, 255, 0, 0);
    pixel(&ctx, 28, 4, 0, 255, 0);
    pixel(&ctx, 4, 28, 0, 0, 255);
    pixel(&ctx, 28, 28, 255, 255, 255);
    // Sampling near a texel boundary remains sharp instead of blending colors.
    pixel(&ctx, 15, 4, 255, 0, 0);
    pixel(&ctx, 16, 4, 0, 255, 0);
    mTextureDestroy(&ctx, &imageTexture);

    // stb_image converts encoded RGB data to RGBA, retaining top-down rows.
    unsigned char tga[] = {
        0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 24, 32,
        0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255
    };
    CHECK(mTextureLoadMemory(&imageTexture, tga, sizeof(tga)) == M_SUCCESS);
    CHECK(imageTexture.width == 2 && imageTexture.height == 2);
    CHECK(mTextureLoadMemory(&imageTexture, tga, sizeof(tga)) == M_FAILURE);
    mRendererBegin(&ctx);
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, &imageTexture) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 4, 4, 255, 0, 0);
    pixel(&ctx, 28, 4, 0, 255, 0);
    pixel(&ctx, 4, 28, 0, 0, 255);
    pixel(&ctx, 28, 28, 255, 255, 255);
    // Sampling near a texel boundary remains sharp instead of blending colors.
    pixel(&ctx, 15, 4, 255, 0, 0);
    pixel(&ctx, 16, 4, 0, 255, 0);
    mTextureDestroy(&ctx, &imageTexture);
    CHECK(mTextureLoadMemory(&imageTexture, tga, 4) == M_FAILURE);
    CHECK(imageTexture.handle == 0);
    CHECK(mTextureLoad(&imageTexture, PEACH_TEST_ASSET_DIR "/missing.png") == M_FAILURE);
    CHECK(mTextureLoad(&imageTexture, PEACH_TEST_ASSET_DIR "/Checker.png") == M_SUCCESS);
    CHECK(imageTexture.width == 32 && imageTexture.height == 32);
    mTextureDestroy(&ctx, &imageTexture);

    // Camera position, rotation and zoom compose as the inverse camera transform.
    mCamera camera2d;
    CHECK(mCameraCreate(&camera2d, 64, 64) == M_SUCCESS);
    CHECK(camera2d.x == 32 && camera2d.y == 32 && camera2d.zoom == 1);
    camera2d.x = 0;
    camera2d.y = 0;
    camera2d.rotation = 1.57079632679f;
    camera2d.zoom = 2;
    CHECK(mCameraUpdate(&camera2d) == M_SUCCESS);
    mRendererBegin(&ctx);
    mRendererSetCamera(&ctx, &camera2d);
    CHECK(mAddVertices(&ctx, triangle, 3, NULL, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 36, 28, 255, 0, 0);
    pixel(&ctx, 4, 4, 0, 0, 0);
    CHECK(mCameraResize(&camera2d, 128, 128) == M_SUCCESS);
    CHECK(camera2d.x == 0 && camera2d.y == 0);
    CHECK(fabsf(camera2d.projection[0] - .03125f) < .00001f);
    mCamera saved = camera2d;
    CHECK(mCameraResize(&camera2d, 0, 128) == M_FAILURE);
    CHECK(memcmp(&camera2d, &saved, sizeof(saved)) == 0);
    camera2d.zoom = 0;
    CHECK(mCameraUpdate(&camera2d) == M_FAILURE);
    CHECK(memcmp(camera2d.viewProjection, saved.viewProjection, sizeof(saved.viewProjection)) == 0);
    mRendererSetCamera(&ctx, NULL);

    // A camera change must flush existing geometry and apply only to later draws.
    mRendererBegin(&ctx);
    makeQuad(quad, 0, 0, 8, M_COLOR_RED);
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, NULL) == M_SUCCESS);
    float camera[] = {2.0f / 64, 0, 0, 0, 0, -2.0f / 64, 0, 0,
                      0, 0, -1, 0, -0.5f, 1, 0, 1};
    mRendererSetViewProjection(&ctx, camera);
    makeQuad(quad, 0, 0, 8, M_COLOR_BLUE);
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 4, 4, 255, 0, 0);
    pixel(&ctx, 20, 4, 0, 0, 255);
    mRendererSetViewProjection(&ctx, NULL);

    // Indexed submissions use the same transform component, including non-unit scale.
    mRendererBegin(&ctx);
    mTransform scaled = mTransformCreate();
    scaled.scale = (vec3s){.x = .5f, .y = .5f, .z = 1};
    makeQuad(quad, 32, 32, 16, M_COLOR_RED);
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, &scaled, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 20, 20, 255, 0, 0);
    pixel(&ctx, 36, 36, 0, 0, 0);

    // Exhaust texture slots and verify both sides of the automatic flush.
    mRendererBegin(&ctx);
    mTexture textures[M_RENDERER_MAX_TEXTURES + 2] = {0};
    for (unsigned int i = 0; i < M_RENDERER_MAX_TEXTURES + 2; i++) {
        unsigned char rgba[] = {(unsigned char)(i * 10), 64, 128, 255};
        CHECK(mTextureCreate(&textures[i], 1, 1, rgba) == M_SUCCESS);
        makeQuad(quad, (float)(i % 8) * 8, (float)(i / 8) * 8, 8, M_COLOR_WHITE);
        CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, &textures[i]) == M_SUCCESS);
    }
    mRendererFlush(&ctx);
    for (unsigned int i = 0; i < M_RENDERER_MAX_TEXTURES + 2; i++) {
        pixel(&ctx, (int)(i % 8) * 8 + 4, (int)(i / 8) * 8 + 4, (int)i * 10, 64, 128);
        mTextureDestroy(&ctx, &textures[i]);
    }

    // Oversized non-indexed and indexed submissions cross vertex/index limits.
    unsigned int count = ((M_RENDERER_MAX_VERTICES / 3) + 2) * 3;
    mVertex* large = calloc(count, sizeof(mVertex));
    CHECK(large != NULL);
    for (unsigned int i = 0; i < count; i++) large[i] = triangle[i % 3];
    mRendererBegin(&ctx);
    CHECK(mAddVertices(&ctx, large, count, NULL, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 2, 2, 255, 0, 0);
    free(large);

    // Small submissions also roll over capacity without changing alpha order.
    mRendererBegin(&ctx);
    makeQuad(quad, 0, 0, 8, M_COLOR_RED);
    for (unsigned int i = 0; i <= M_RENDERER_MAX_VERTICES / 4; i++) {
        CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, NULL) == M_SUCCESS);
    }
    makeQuad(quad, 0, 0, 8, M_COLOR(0, 0, 1, .5f));
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 4, 4, 128, 0, 128);

    count = M_RENDERER_MAX_INDICES + 3;
    unsigned int* largeIndices = calloc(count, sizeof(unsigned int));
    CHECK(largeIndices != NULL);
    for (unsigned int i = 0; i < count; i++) largeIndices[i] = i % 3;
    mRendererBegin(&ctx);
    CHECK(mAddVerticesIndexed(&ctx, triangle, 3, largeIndices, count, NULL, NULL) == M_SUCCESS);
    mRendererFlush(&ctx);
    pixel(&ctx, 2, 2, 255, 0, 0);

    // Invalid indices late in a large submission must not draw a partial batch.
    mRendererBegin(&ctx);
    largeIndices[count - 1] = 3;
    CHECK(mAddVerticesIndexed(&ctx, triangle, 3, largeIndices, count, NULL, NULL) == M_FAILURE);
    mRendererFlush(&ctx);
    pixel(&ctx, 2, 2, 0, 0, 0);
    free(largeIndices);
    CHECK(mAddVertices(&ctx, triangle, 2, NULL, NULL) == M_FAILURE);
    CHECK(mAddVerticesIndexed(&ctx, triangle, 3, NULL, 3, NULL, NULL) == M_FAILURE);
    CHECK(mAddVertices(&ctx, NULL, 0, NULL, NULL) == M_SUCCESS);

    // Destruction flushes users of the texture before deleting its GPU storage.
    makeQuad(quad, 0, 0, 8, M_COLOR_WHITE);
    CHECK(mAddVerticesIndexed(&ctx, quad, 4, quadIndices, 6, NULL, &texture) == M_SUCCESS);
    mTextureDestroy(&ctx, &texture);
    CHECK(texture.handle == 0);
    pixel(&ctx, 4, 4, 0, 255, 0);
    mTimeReset(&ctx.time);
    mUpdate(&ctx);
    CHECK(ctx.time.frameCount == 1 && ctx.time.deltaTime >= 0.0);
    CHECK(isfinite(ctx.time.fps) && ctx.time.frameTime == ctx.time.deltaTime * 1000.0);
    CHECK(glGetError() == GL_NO_ERROR);
    pixel(&ctx, 4, 4, 0, 0, 0);
    mDestroy(&ctx);
    CHECK(ctx.renderer == NULL && ctx.window.handle == NULL);
    puts("Renderer checks passed.");
    return 0;
}

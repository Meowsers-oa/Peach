//
// Created by Štěpán Toman on 27.09.2026.
//

#include <Peach/Peach.h>
#include <cglm/cglm.h>

int main() {
    mContext ctx = mContextCreate();
    mWindowInfo info = {.width = 1200, .height = 800, .title = "Peach App"};
    if (mWindowCreate(&ctx, &info) == M_FAILURE) {
        mDestroy(&ctx);
        return -1;
    }

    mTexture texture = {0};
    if (mTextureLoad(&texture, PEACH_SANDBOX_ASSET_DIR "/Checker.png") == M_FAILURE) {
        mDestroy(&ctx);
        return -1;
    }

    mVertex quad[] = {
        {.x = -100, .y = -100, .u = 0, .v = 0, .color = M_COLOR_WHITE},
        {.x =  100, .y = -100, .u = 1, .v = 0, .color = M_COLOR_WHITE},
        {.x =  100, .y =  100, .u = 1, .v = 1, .color = M_COLOR_WHITE},
        {.x = -100, .y =  100, .u = 0, .v = 1, .color = M_COLOR_WHITE}
    };
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    mVertex triangle[] = {
        {.x = 100, .y = 100, .color = M_COLOR_RED},
        {.x = 300, .y = 100, .color = M_COLOR_GREEN},
        {.x = 200, .y = 300, .color = M_COLOR_BLUE}
    };

    mCamera camera;
    if (mCameraCreate(&camera, (float)info.width, (float)info.height) == M_FAILURE) {
        mTextureDestroy(&ctx, &texture);
        mDestroy(&ctx);
        return -1;
    }

    while (ctx.window.running) {
        int width, height;
        glfwGetWindowSize(ctx.window.handle, &width, &height);
        if (width > 0 && height > 0) mCameraResize(&camera, (float)width, (float)height);
        mRendererSetCamera(&ctx, &camera);
        mat4 transform = GLM_MAT4_IDENTITY_INIT;
        glm_translate(transform, (vec3){600.0f, 400.0f, 0.0f});
        glm_rotate_z(transform, (float)glfwGetTime(), transform);
        mAddVertices(&ctx, triangle, 3, NULL, NULL);
        mAddVerticesIndexed(&ctx, quad, 4, indices, 6, &transform[0][0], &texture);
        mUpdate(&ctx);
        if (mKeyPressed(&ctx, M_KEY_ESCAPE)) ctx.window.running = M_FALSE;
    }

    mTextureDestroy(&ctx, &texture);
    mDestroy(&ctx);
    return 0;
}

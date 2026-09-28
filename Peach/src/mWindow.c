//
// Created by Štěpán Toman on 27.09.2026.
//

#include <Peach/mWindow.h>
#include <Peach/Input.h>
#include <Peach/Renderer.h>

int mWindowCreate(mContext *ctx, mWindowInfo* info) {
    GLFWwindow* window = glfwCreateWindow(info->width, info->height, info->title, NULL, NULL);

    if (window == NULL) {
        printf("Failed to create window!");
        return M_FAILURE;
    }

    ctx->window.running = M_TRUE;
    ctx->window.handle = window;
    glfwMakeContextCurrent(ctx->window.handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize rendering backend!");
        glfwDestroyWindow(window);
        ctx->window = (mWindow){0};
        return M_FAILURE;
    }

    int fWidth, fHeight;
    glfwGetFramebufferSize(ctx->window.handle, &fWidth, &fHeight);
    glViewport(0, 0, fWidth, fHeight);
    glfwSwapInterval(0);

    mColor color = (mColor){.r = .2f, .g = .2f, .b = .2f, .a = .2f};
    ctx->window.bgColor = color;

    if (mRendererCreate(ctx) == M_FAILURE) {
        printf("Failed to initialize renderer!");
        glfwDestroyWindow(window);
        ctx->window = (mWindow){0};
        return M_FAILURE;
    }

    mInputInit(ctx);

    return M_SUCCESS;
}

void frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mWindowUpdate(mContext *ctx) {
    mRendererFlush(ctx);
    glfwSwapBuffers(ctx->window.handle);

    mInputUpdate(ctx);
    ctx->window.running = !glfwWindowShouldClose(ctx->window.handle);
    mRendererBegin(ctx);
}

void mWindowDestroy(mContext *ctx) {
    if (ctx->window.handle == NULL) return;
    glfwMakeContextCurrent(ctx->window.handle);
    mRendererDestroy(ctx);
    glfwDestroyWindow(ctx->window.handle);
    ctx->window = (mWindow){0};
}

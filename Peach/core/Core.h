//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_CORE_H
#define PEACH_CORE_H

#include <stdio.h>

#include "Peach/platform/Input.h"
#include "Peach/core/Structs.h"
#include "Peach/platform/Window.h"
#include "Peach/graphics/Renderer.h"
#include "Peach/core/Utils.h"
#include "Peach/graphics/Ui.h"
#include "Peach/core/Frame.h"

static inline mContext mCreateContext() {
    mContext context = {0};

    if (!glfwInit()) {
        const char* desc = NULL;
        int error = glfwGetError(&desc);
        printf("ERROR: Failed to initialize windowing system!: %d: %s", error, desc ? desc : "Unknown error");
        return context;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    context.window.clearColor.r = 1.f;
    context.window.clearColor.g = 1.f;
    context.window.clearColor.b = 1.f;
    context.window.clearColor.a = 1.f;

    mTimeStart(&context.time);

    return context;
}

// Initialize an empty context and its window in one call. Returns 1 on success.
static inline int mInit(mContext* ctx, int width, int height, const char* title) {
    if (!ctx) return 0;
    *ctx = mCreateContext();
    if (mWindowCreate(ctx, width, height, title)) return 1;
    glfwTerminate();
    return 0;
}

// Legacy manual loop support; new code uses mBeginFrame/mEndFrame.
static inline void mUpdate(mContext* ctx) {
    if (ctx->frame.active) {
        mEndFrame(ctx);
        return;
    }
    mUiRender(ctx);
    mTimeUpdate(&ctx->time);
    mWindowUpdate(ctx);
    mInputUpdate();
    mUiUpdate(ctx);
}

static inline void mEnd(mContext* ctx) {
    if (!ctx) return;
    if (ctx->window.handle) glfwMakeContextCurrent(ctx->window.handle);
    mFrameShutdown(ctx);
    mUiEnd(ctx);
    mRendererShutdown(ctx);
    if (ctx->window.handle != NULL) mWindowEnd(ctx);
    glfwTerminate();
}



#endif //PEACH_CORE_H

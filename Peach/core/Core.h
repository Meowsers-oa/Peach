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

static inline mContext mCreateContext() {
    mContext context = {0};

    if (!glfwInit()) {
        const char* desc;
        printf("ERROR: Failed to initialize windowing system!: %d: %s", glfwGetError(&desc), desc);
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

static inline void mUpdate(mContext* ctx) {
    mTimeUpdate(&ctx->time);
    mWindowUpdate(ctx);
    mUpdateInput();
}

static inline void mEnd(mContext* ctx) {
    mRendererShutdown(ctx);
    if (ctx->window.handle != NULL) mWindowEnd(ctx);
    glfwTerminate();
}



#endif //PEACH_CORE_H

//
// Created by Štěpán Toman on 20.09.2026.
//

#include <stdio.h>
#include "Peach/platform/Window.h"
#include "Peach/platform/Input.h"
#include "Peach/graphics/Renderer.h"
#include "Peach/graphics/Ui.h"

static mContext* ctx;
static const char* lastTitle;

int mWindowCreate(mContext *context, int width, int height, const char *title) {
    ctx = context;
    context->window.handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (context->window.handle == NULL) {
        const char* desc;
        printf("ERROR: Failed to create window!: %d: %s", glfwGetError(&desc), desc);
        return -1;
    }

    context->window.running = 1;

    glfwMakeContextCurrent(context->window.handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to load rendering backend!");
        return -1;
    }

    context->window.title = title;
    glfwGetFramebufferSize(context->window.handle, &context->window.width, &context->window.height);
    glViewport(0, 0, context->window.width, context->window.height);
    glfwSetFramebufferSizeCallback(context->window.handle, mOnWindowResize);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glfwSwapInterval(0);

    lastTitle = context->window.title;

    mInputStart(context->window.handle);
    mRendererInit(context);
    if (!mUiStart(context)) {
        mRendererShutdown(context);
        mWindowEnd(context);
        context->window.running = 0;
        return -1;
    }

    glClearColor(context->window.clearColor.r, context->window.clearColor.g, context->window.clearColor.b, context->window.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return 0;
}

void mOnWindowResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    ctx->window.width = width;
    ctx->window.height = height;
}

void mWindowUpdate(mContext *context) {
    context = ctx;
    if (context->window.title != lastTitle) {
        lastTitle = context->window.title;
        glfwSetWindowTitle(context->window.handle, context->window.title);
    }

    context->window.running = !glfwWindowShouldClose(context->window.handle);

    glfwSwapBuffers(context->window.handle);
    glfwPollEvents();

    glClearColor(context->window.clearColor.r, context->window.clearColor.g, context->window.clearColor.b, context->window.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void mWindowStop(mContext *context) {
    context->window.running = 0;
    glfwSetWindowShouldClose(context->window.handle, 1);

}

void mWindowEnd(mContext *context) {
    if (context->window.handle) glfwDestroyWindow(context->window.handle);
    context->window.handle = NULL;
}


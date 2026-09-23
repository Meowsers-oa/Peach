//
// Created by Štěpán Toman on 20.09.2026.
//

#include <stdio.h>
#include "Peach/platform/Window.h"
#include "Peach/platform/Input.h"
#include "Peach/graphics/Renderer.h"
#include "Peach/graphics/Ui.h"

int mWindowCreate(mContext *context, int width, int height, const char *title) {
    if (!context || context->window.handle || width <= 0 || height <= 0 || !title) return 0;
    context->window.handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (context->window.handle == NULL) {
        const char* desc = NULL;
        int error = glfwGetError(&desc);
        printf("ERROR: Failed to create window!: %d: %s", error, desc ? desc : "Unknown error");
        return 0;
    }

    context->window.running = 1;

    glfwMakeContextCurrent(context->window.handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to load rendering backend!");
        mWindowEnd(context);
        return 0;
    }

    context->window.title = title;
    glfwGetFramebufferSize(context->window.handle, &context->window.width, &context->window.height);
    glViewport(0, 0, context->window.width, context->window.height);
    glfwSetWindowUserPointer(context->window.handle, context);
    glfwSetFramebufferSizeCallback(context->window.handle, mOnWindowResize);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glfwSwapInterval(0);

    mInputStart(context->window.handle);
    mRendererInit(context);
    if (!mUiStart(context)) {
        mRendererShutdown(context);
        mWindowEnd(context);
        context->window.running = 0;
        return 0;
    }

    glClearColor(context->window.clearColor.r, context->window.clearColor.g, context->window.clearColor.b, context->window.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return 1;
}

void mOnWindowResize(GLFWwindow *window, int width, int height) {
    mContext* context = glfwGetWindowUserPointer(window);
    if (!context) return;
    context->window.width = width;
    context->window.height = height;
}

void mWindowUpdate(mContext *context) {
    if (!context || !context->window.handle) return;
    glfwMakeContextCurrent(context->window.handle);
    glViewport(0, 0, context->window.width, context->window.height);
    glfwSetWindowTitle(context->window.handle, context->window.title);

    context->window.running = !glfwWindowShouldClose(context->window.handle);

    glfwSwapBuffers(context->window.handle);
    glfwPollEvents();

    glClearColor(context->window.clearColor.r, context->window.clearColor.g, context->window.clearColor.b, context->window.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void mWindowStop(mContext *context) {
    if (!context || !context->window.handle) return;
    context->window.running = 0;
    glfwSetWindowShouldClose(context->window.handle, 1);

}

void mWindowEnd(mContext *context) {
    if (!context) return;
    if (context->window.handle) glfwDestroyWindow(context->window.handle);
    context->window.handle = NULL;
    context->window.running = 0;
}


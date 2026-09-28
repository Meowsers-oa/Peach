//
// Created by Štěpán Toman on 27.09.2026.
//

#include <Peach/Peach.h>


mContext mContextCreate() {
    mContext ctx = (mContext){0};
    ctx.window = (mWindow){0};

    if (!glfwInit()) {
        printf("Failed to initialize windowing system!");
        return ctx;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    return ctx;
}

void mUpdate(mContext *ctx) {
    mWindowUpdate(ctx);
}

void mDestroy(mContext *ctx) {
    mWindowDestroy(ctx);
    glfwTerminate();
}

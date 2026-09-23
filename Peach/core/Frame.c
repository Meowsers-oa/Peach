#include "Peach/core/Frame.h"
#include "Peach/core/Utils.h"
#include "Peach/platform/Input.h"
#include "Peach/graphics/Shader.h"
#include "Peach/graphics/Renderer.h"
#include "Peach/graphics/PostProcess.h"
#include "Peach/graphics/Ui.h"
#include <string.h>

int mSetPostProcess(mContext* ctx, const char* fragment) {
    if (!ctx || !ctx->window.handle || ctx->frame.active) return 0;
    unsigned int program = 0;
    if (fragment) {
        program = (strchr(fragment, '/') || strchr(fragment, '\\'))
            ? mCreatePostProcessShader(fragment)
            : mCreateResourceShaderProgram("fullscreen.vert", fragment);
        if (!program) return 0;
    }
    if (ctx->frame.postProcess) glDeleteProgram(ctx->frame.postProcess);
    ctx->frame.postProcess = program;
    if (!program) mRenderTargetFree(&ctx->frame.scene);
    return 1;
}

int mBeginFrame(mContext* ctx) {
    if (!ctx || !ctx->window.handle || !ctx->window.running || ctx->frame.active) return 0;
    glfwMakeContextCurrent(ctx->window.handle);
    glfwPollEvents();
    glfwGetFramebufferSize(ctx->window.handle, &ctx->window.width, &ctx->window.height);
    while (!glfwWindowShouldClose(ctx->window.handle) &&
           (ctx->window.width <= 0 || ctx->window.height <= 0)) {
        glfwWaitEventsTimeout(0.05);
        glfwGetFramebufferSize(ctx->window.handle, &ctx->window.width, &ctx->window.height);
    }
    if (glfwWindowShouldClose(ctx->window.handle)) {
        ctx->window.running = 0;
        return 0;
    }
    mTimeUpdate(&ctx->time);
    mInputUpdate();
    if (!ctx->renderer.shaderProgram) mLoadDefaultShaders(ctx);
    if (!ctx->renderer.shaderProgram) return 0;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, ctx->window.width, ctx->window.height);
    if (ctx->frame.postProcess) {
        if (!mRenderTargetResize(&ctx->frame.scene, ctx->window.width, ctx->window.height) ||
            !mRenderTargetBegin(ctx, &ctx->frame.scene)) return 0;
    }
    mRendererClear(ctx);
    mRendererBegin(ctx);
    mUiUpdate(ctx);
    ctx->frame.active = 1;
    return 1;
}

void mEndFrame(mContext* ctx) {
    if (!ctx || !ctx->frame.active) return;
    if (ctx->frame.scene.active) {
        mRenderTargetEnd(ctx, &ctx->frame.scene);
        mDrawFullscreenQuad(ctx, ctx->frame.postProcess, &ctx->frame.scene.color);
    } else {
        mRendererEnd(ctx);
    }
    mUiRender(ctx);
    glfwSetWindowTitle(ctx->window.handle, ctx->window.title);
    glfwSwapBuffers(ctx->window.handle);
    ctx->frame.active = 0;
}

void mFrameShutdown(mContext* ctx) {
    if (!ctx) return;
    if (ctx->frame.scene.active) mRenderTargetEnd(ctx, &ctx->frame.scene);
    mRenderTargetFree(&ctx->frame.scene);
    if (ctx->frame.postProcess) glDeleteProgram(ctx->frame.postProcess);
    ctx->frame.postProcess = 0;
    ctx->frame.active = 0;
}

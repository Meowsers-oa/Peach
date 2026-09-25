#include "Peach/graphics/PostProcess.h"
#include "Peach/graphics/Renderer.h"
#include <stdio.h>

int mRenderTargetResize(mRenderTarget* target, int width, int height) {
    if (!target || target->active || width <= 0 || height <= 0) return 0;
    if (target->framebuffer && target->color.width == width && target->color.height == height) return 1;
    GLint draw, read, texture, renderbuffer, maxTexture, maxRenderbuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
    if (target->framebuffer && (draw == (GLint)target->framebuffer || read == (GLint)target->framebuffer)) return 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexture);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbuffer);
    if (width > maxTexture || height > maxTexture || width > maxRenderbuffer || height > maxRenderbuffer) return 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
    mRenderTarget next = {0};
    glGenTextures(1, &next.color.id);
    glBindTexture(GL_TEXTURE_2D, next.color.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenRenderbuffers(1, &next.depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, next.depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glGenFramebuffers(1, &next.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, next.framebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, next.color.id, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, next.depthBuffer);
    int complete = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)draw);
    glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
    glBindRenderbuffer(GL_RENDERBUFFER, (GLuint)renderbuffer);
    if (!complete) {
        mRenderTargetFree(&next);
        fprintf(stderr, "Peach: post-process framebuffer allocation failed.\n");
        return 0;
    }
    next.color.width = width;
    next.color.height = height;
    next.color.channels = 4;
    mRenderTargetFree(target);
    *target = next;
    return 1;
}

void mRenderTargetFree(mRenderTarget* target) {
    if (!target || target->active) return;
    if (target->framebuffer) glDeleteFramebuffers(1, &target->framebuffer);
    if (target->depthBuffer) glDeleteRenderbuffers(1, &target->depthBuffer);
    if (target->color.id) glDeleteTextures(1, &target->color.id);
    *target = (mRenderTarget){0};
}

int mRenderTargetBegin(mContext* ctx, mRenderTarget* target) {
    if (!ctx || !target || !target->framebuffer || target->active) return 0;
    mRendererEnd(ctx);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &target->previousFramebuffer);
    glGetIntegerv(GL_VIEWPORT, target->previousViewport);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->framebuffer);
    glViewport(0, 0, target->color.width, target->color.height);
    target->active = 1;
    return 1;
}

void mRenderTargetEnd(mContext* ctx, mRenderTarget* target) {
    if (!ctx || !target || !target->active) return;
    GLint current;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current);
    if (current != (GLint)target->framebuffer) return;
    mRendererEnd(ctx);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)target->previousFramebuffer);
    glViewport(target->previousViewport[0], target->previousViewport[1],
               target->previousViewport[2], target->previousViewport[3]);
    target->active = 0;
}

int mDrawFullscreenQuad(mContext* ctx, unsigned int program, const mTexture* source) {
    if (!ctx || !program) return 0;
    GLint linked;
    if (!glIsProgram(program)) return 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) return 0;
    GLuint input = source ? source->id : ctx->renderer.whiteTexture;
    if (!input || !glIsTexture(input)) return 0;
    GLint framebuffer, viewport[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return 0;
    // Reject undefined texture feedback, including attachments other than color 0.
    if (framebuffer) {
        GLint maxAttachments;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
        for (int i = 0; i < maxAttachments; ++i) {
            GLint type, name;
            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                                   GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
            if (type != GL_TEXTURE) continue;
            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                                   GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
            if ((GLuint)name == input) return 0;
        }
    }
    mRendererEnd(ctx);
    GLint oldProgram, vao, activeTexture, texture, sampler, polygon[2];
    GLboolean depthWrite, colorWrite[4];
    GLenum caps[] = {GL_DEPTH_TEST, GL_CULL_FACE, GL_BLEND, GL_SCISSOR_TEST, GL_STENCIL_TEST, GL_RASTERIZER_DISCARD};
    GLboolean enabled[6];
    for (int i = 0; i < 6; ++i) { enabled[i] = glIsEnabled(caps[i]); glDisable(caps[i]); }
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glGetIntegerv(GL_POLYGON_MODE, polygon);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glGetBooleanv(GL_COLOR_WRITEMASK, colorWrite);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
    glGetIntegerv(GL_SAMPLER_BINDING, &sampler);
    glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (!ctx->renderer.fullscreenVao) glGenVertexArrays(1, &ctx->renderer.fullscreenVao);
    glBindVertexArray(ctx->renderer.fullscreenVao);
    glBindTexture(GL_TEXTURE_2D, input);
    glBindSampler(0, 0);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
    glUniform2f(glGetUniformLocation(program, "uResolution"), (float)viewport[2], (float)viewport[3]);
    glUniform1f(glGetUniformLocation(program, "uTime"), ctx->time.timeSinceStart);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ctx->renderer.pendingStats.drawCalls++;
    if (!ctx->frame.active) ctx->renderer.stats = ctx->renderer.pendingStats;
    glUseProgram((GLuint)oldProgram);
    glBindVertexArray((GLuint)vao);
    glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
    glBindSampler(0, (GLuint)sampler);
    glActiveTexture((GLenum)activeTexture);
    glDepthMask(depthWrite);
    glColorMask(colorWrite[0], colorWrite[1], colorWrite[2], colorWrite[3]);
    glPolygonMode(GL_FRONT_AND_BACK, (GLenum)polygon[0]);
    for (int i = 0; i < 6; ++i) if (enabled[i]) glEnable(caps[i]);
    return 1;
}

#ifndef PEACH_POST_PROCESS_H
#define PEACH_POST_PROCESS_H

#include "Peach/core/Structs.h"

// Initialize targets to {0}. Resize preserves the old target on failure.
// Color is RGBA16F, with a depth buffer. Resizing changes the color texture ID.
int mRenderTargetResize(mRenderTarget* target, int width, int height);
void mRenderTargetFree(mRenderTarget* target);
// Finish pending geometry before switching targets. Begin/End pairs may nest,
// but must end in reverse order. Do not resize/free an active target.
int mRenderTargetBegin(mContext* ctx, mRenderTarget* target);
void mRenderTargetEnd(mContext* ctx, mRenderTarget* target);

// Caller owns the returned shader program; delete it before mEnd.
// The vertex shader is supplied by Peach. fragmentPath is a filesystem path.
unsigned int mCreatePostProcessShader(const char* fragmentPath);

// Immediate pass covering the current viewport. NULL source uses white,
// allowing procedural shaders. Never sample the target currently being drawn.
// Provides uTexture (unit 0), uResolution (output pixels), uTime (seconds).
// Custom uniforms can be set with glUseProgram/glUniform before this call.
int mDrawFullscreenQuad(mContext* ctx, unsigned int program, const mTexture* source);

#endif

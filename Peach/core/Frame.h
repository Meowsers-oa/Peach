#ifndef PEACH_FRAME_H
#define PEACH_FRAME_H

#include "Peach/core/Structs.h"

// Normal application loop: while (mBeginFrame(&ctx)) { draw; mEndFrame(&ctx); }
// Begin handles events, time, input, resizing, clear, UI and scene batching.
// Returns 0 on close/error. A minimized window waits for events until restored.
int mBeginFrame(mContext* ctx);
// Finishes geometry, applies post-processing, draws UI, and swaps buffers.
void mEndFrame(mContext* ctx);

// Set once outside a frame. Peach owns the shader and the intermediate target.
// A bare name loads from the configured shader resources; a path loads that file.
// NULL disables post-processing. A failed load keeps the previous effect.
int mSetPostProcess(mContext* ctx, const char* fragment);
// Called by mEnd; application code does not need to call this.
void mFrameShutdown(mContext* ctx);

#endif

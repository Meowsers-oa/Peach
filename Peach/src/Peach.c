//
// Created by Štěpán Toman on 27.09.2026.
//

#include <Peach/Peach.h>

mContext mContextCreate() {
    mContext ctx = (mContext){0};

    if (!glfwInit()) {
        printf("Failed to initialize windowing system!");
        return ctx;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    ctx.resourcePool = mMapCreate();
    ctx.sprites = mMapCreate();
    ctx.lights = mMapCreate();

    return ctx;
}

void mUpdate(mContext* ctx) {
    mWindowUpdate(ctx);
    mTimeUpdate(&ctx->time);
}

void mEnd(mContext* ctx) {
    if (ctx == NULL) return;
    if (ctx->window.handle != NULL) glfwMakeContextCurrent(ctx->window.handle);
    mRendererFlush(ctx);

    if (ctx->sprites != NULL) {
        for (size_t i = 0; i < ctx->sprites->capacity; i++) {
            mMapSlot* slot = &ctx->sprites->entries[i];
            if (slot->is_occupied && slot->valueSize == sizeof(mSprite)) {
                mSpriteDestroy(ctx, slot->value);
            }
        }
        mMapDestroy(ctx->sprites);
        ctx->sprites = NULL;
    }

    mMapDestroy(ctx->lights);
    ctx->lights = NULL;

    // Shapes can reference texture records, so release their buffers first.
    if (ctx->resourcePool != NULL) {
        for (int pass = 0; pass < 2; pass++) {
            for (size_t i = 0; i < ctx->resourcePool->capacity; i++) {
                mMapSlot* slot = &ctx->resourcePool->entries[i];
                if (slot->is_occupied && ((slot->resourceType == M_RESOURCE_SHAPE) == (pass == 0))) {
                    mRemoveResource(ctx, slot->key);
                }
            }
        }
        mMapDestroy(ctx->resourcePool);
        ctx->resourcePool = NULL;
    }
    mWindowDestroy(ctx);
    glfwTerminate();
    *ctx = (mContext){0};
}

void mDestroy(mContext* ctx) {
    mEnd(ctx);
}

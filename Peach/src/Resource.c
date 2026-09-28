#include <Peach/Resource.h>
#include <Peach/Renderer.h>
#include <Peach/Texture.h>
#include <Peach/Shader.h>
#include <Peach/Shape.h>
#include <string.h>

static mMapSlot* resourceSlot(mContext* ctx, const char* key) {
    if (ctx == NULL || ctx->resourcePool == NULL) return NULL;
    void* value = mMapGet(ctx->resourcePool, key);
    if (value == NULL) return NULL;
    for (size_t i = 0; i < ctx->resourcePool->capacity; i++) {
        mMapSlot* slot = &ctx->resourcePool->entries[i];
        if (slot->is_occupied && slot->value == value) return slot;
    }
    return NULL;
}

static void updateSprites(mContext* ctx, unsigned int handle, const mTexture* replacement) {
    mMapIter iter = mMap_iter(ctx->sprites);
    mMapEntry entry;
    while (mMap_next(&iter, &entry)) {
        if (entry.valueSize != sizeof(mSprite)) continue;
        mSprite* sprite = entry.value;
        if (sprite->texture.handle == handle) {
            sprite->texture = replacement != NULL ? *replacement : (mTexture){0};
        }
    }
}

static void prepareCleanup(mContext* ctx) {
    if (ctx->window.handle != NULL) glfwMakeContextCurrent(ctx->window.handle);
    mRendererFlush(ctx);
}

static void releaseResource(mContext* ctx, mResourceType type, void* value) {
    if (type == M_RESOURCE_TEXTURE) {
        mTexture* texture = value;
        updateSprites(ctx, texture->handle, NULL);
        mTextureDestroy(ctx, texture);
    } else if (type == M_RESOURCE_SHADER) {
        mShaderDestroy(value);
    } else if (type == M_RESOURCE_SHAPE) {
        mShapeDestroy(value);
    }
}

M_BOOL mResourceStore(mContext* ctx, const char* key, const void* value, size_t size, mResourceType type) {
    if (ctx == NULL || ctx->resourcePool == NULL || key == NULL || value == NULL || size == 0) return M_FALSE;
    if (type < M_RESOURCE_VALUE || type > M_RESOURCE_SHAPE ||
        (type == M_RESOURCE_TEXTURE && size != sizeof(mTexture)) ||
        (type == M_RESOURCE_SHADER && size != sizeof(mShader)) ||
        (type == M_RESOURCE_SHAPE && size != sizeof(mShape))) return M_FALSE;

    mMapSlot* old = resourceSlot(ctx, key);
    // Each GPU handle or heap buffer has one owning resource entry.
    for (size_t i = 0; i < ctx->resourcePool->capacity; i++) {
        mMapSlot* slot = &ctx->resourcePool->entries[i];
        if (!slot->is_occupied || slot->resourceType != type || type == M_RESOURCE_VALUE) continue;
        M_BOOL shared = M_FALSE;
        if (type == M_RESOURCE_TEXTURE) {
            unsigned int handle = ((const mTexture*)value)->handle;
            shared = handle != 0 && handle == ((mTexture*)slot->value)->handle;
        } else if (type == M_RESOURCE_SHADER) {
            unsigned int handle = ((const mShader*)value)->handle;
            shared = handle != 0 && handle == ((mShader*)slot->value)->handle;
        } else if (type == M_RESOURCE_SHAPE) {
            const mShape* shape = value;
            mShape* stored = slot->value;
            shared = (shape->vertices != NULL && shape->vertices == stored->vertices) ||
                     (shape->indices != NULL && shape->indices == stored->indices);
        }
        if (shared) {
            // Re-registering the identical stored value under its own key is a no-op.
            return slot == old && slot->valueSize == size && memcmp(slot->value, value, size) == 0;
        }
    }

    mResourceType oldType = old != NULL ? old->resourceType : M_RESOURCE_VALUE;
    mTexture oldTexture = {0};
    mShader oldShader = {0};
    mShape oldShape = {0};
    if (oldType == M_RESOURCE_TEXTURE) oldTexture = *(mTexture*)old->value;
    if (oldType == M_RESOURCE_SHADER) oldShader = *(mShader*)old->value;
    if (oldType == M_RESOURCE_SHAPE) oldShape = *(mShape*)old->value;

    if (!mMapSetBytes(ctx->resourcePool, key, value, size)) return M_FALSE;
    mMapSlot* stored = resourceSlot(ctx, key);
    stored->resourceType = type;
    if (oldType != M_RESOURCE_VALUE) {
        prepareCleanup(ctx);
        if (oldType == M_RESOURCE_TEXTURE) {
            updateSprites(ctx, oldTexture.handle, type == M_RESOURCE_TEXTURE ? stored->value : NULL);
            mTextureDestroy(ctx, &oldTexture);
        } else if (oldType == M_RESOURCE_SHADER) {
            mShaderDestroy(&oldShader);
        } else if (oldType == M_RESOURCE_SHAPE) {
            mShapeDestroy(&oldShape);
        }
    }
    return M_TRUE;
}

void* mGetResource(mContext* ctx, const char* key) {
    return ctx != NULL ? mMapGet(ctx->resourcePool, key) : NULL;
}

mResourceType mResourceTypeOf(mContext* ctx, const char* key) {
    mMapSlot* slot = resourceSlot(ctx, key);
    return slot != NULL ? slot->resourceType : M_RESOURCE_VALUE;
}

M_BOOL mContainsResource(mContext* ctx, const char* key) {
    return ctx != NULL ? mMapContains(ctx->resourcePool, key) : M_FALSE;
}

void mRemoveResource(mContext* ctx, const char* key) {
    mMapSlot* slot = resourceSlot(ctx, key);
    if (slot == NULL) return;
    prepareCleanup(ctx);
    releaseResource(ctx, slot->resourceType, slot->value);
    mMapRemove(ctx->resourcePool, key);
}

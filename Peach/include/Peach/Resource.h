#ifndef PEACH_RESOURCE_H
#define PEACH_RESOURCE_H

#include <Peach/Map.h>

M_BOOL mResourceStore(mContext* ctx, const char* key, const void* value, size_t size, mResourceType type);

// Success transfers ownership to the context; failure does not.
static inline M_BOOL mAddTexture(mContext* ctx, const char* key, mTexture texture) {
    return mResourceStore(ctx, key, &texture, sizeof(texture), M_RESOURCE_TEXTURE);
}

static inline M_BOOL mAddShader(mContext* ctx, const char* key, mShader shader) {
    return mResourceStore(ctx, key, &shader, sizeof(shader), M_RESOURCE_SHADER);
}

static inline M_BOOL mAddShape(mContext* ctx, const char* key, mShape shape) {
    return mResourceStore(ctx, key, &shape, sizeof(shape), M_RESOURCE_SHAPE);
}

static inline M_BOOL mAddSpriteSheet(mContext* ctx, const char* key, mSpriteSheet sheet) {
    return mResourceStore(ctx, key, &sheet, sizeof(sheet), M_RESOURCE_SPRITE_SHEET);
}

void* mGetResource(mContext* ctx, const char* key);
mResourceType mResourceTypeOf(mContext* ctx, const char* key);
M_BOOL mContainsResource(mContext* ctx, const char* key);
void mRemoveResource(mContext* ctx, const char* key);

#endif //PEACH_RESOURCE_H

#ifndef PEACH_RESOURCE_H
#define PEACH_RESOURCE_H

#include <Peach/Map.h>

// Success transfers texture/shader/shape ownership to the context; failure does not.
#define mAddResource(ctx, key, type, ...) \
    mResourceStore((ctx), (key), (type[]){__VA_ARGS__}, sizeof(type), \
        _Generic((type*)0, mTexture*: M_RESOURCE_TEXTURE, mShader*: M_RESOURCE_SHADER, \
                 mShape*: M_RESOURCE_SHAPE, default: M_RESOURCE_VALUE))

M_BOOL mResourceStore(mContext* ctx, const char* key, const void* value, size_t size, mResourceType type);
void* mGetResource(mContext* ctx, const char* key);
mResourceType mResourceTypeOf(mContext* ctx, const char* key);
M_BOOL mContainsResource(mContext* ctx, const char* key);
void mRemoveResource(mContext* ctx, const char* key);

#endif //PEACH_RESOURCE_H

#ifndef PEACH_TEXTURE_H
#define PEACH_TEXTURE_H

#include <Peach/Common.h>

// Initialize to {0}. RGBA8, top-down rows, nearest filtering.
int mTextureCreate(mTexture* texture, int width, int height, const unsigned char* pixels);
int mTextureLoad(mTexture* texture, const char* path);
int mTextureLoadMemory(mTexture* texture, const unsigned char* data, int size);
// Registered textures are owned by mEnd/mRemoveResource.
void mTextureDestroy(mContext* ctx, mTexture* texture);

#endif //PEACH_TEXTURE_H

#ifndef PEACH_TEXTURE_H
#define PEACH_TEXTURE_H

#include <Peach/Common.h>

typedef struct {
    unsigned int handle;
    int width;
    int height;
}mTexture;

// Initialize texture to {0}. All operations require the owning OpenGL context.
// Uploads packed RGBA8 pixels, with nearest-neighbor filtering and clamp-to-edge wrapping.
// The first pixel row is sampled at v = 0. Source pixels are copied immediately.
int mTextureCreate(mTexture* texture, int width, int height, const unsigned char* pixels);
// Decode an image file (PNG, JPEG, BMP, TGA and other stb_image formats) to RGBA8.
// Images are kept top-down, matching the renderer's top-left UV convention.
int mTextureLoad(mTexture* texture, const char* path);
// Decode encoded image bytes, not raw pixels. size is the byte count.
int mTextureLoadMemory(mTexture* texture, const unsigned char* data, int size);
// Flush pending draws before deletion. Destroy user textures before mDestroy.
void mTextureDestroy(mContext* ctx, mTexture* texture);

#endif //PEACH_TEXTURE_H

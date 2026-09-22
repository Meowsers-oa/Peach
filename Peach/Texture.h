//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_TEXTURE_H
#define PEACH_TEXTURE_H

#include "Structs.h"

// Load texture from file using stb_image
mTexture mLoadTexture(const char* filePath);

// Create texture from raw pixel data
mTexture mCreateTexture(int width, int height, int channels, const unsigned char* data);

// Create a 1x1 solid color texture
mTexture mCreateColorTexture(Color color);

// Unload / Delete texture from GPU memory
void mUnloadTexture(mTexture* texture);
void mDeleteTexture(mTexture* texture);
void mDeleteTextureId(unsigned int textureId);

// Bind / Unbind texture to/from a texture unit slot
void mBindTexture(const mTexture* texture, unsigned int slot);
void mBindTextureId(unsigned int textureId, unsigned int slot);
void mUnbindTexture(unsigned int slot);

// Set texture filtering and wrapping modes
void mSetTextureFilter(const mTexture* texture, GLint minFilter, GLint magFilter);
void mSetTextureWrap(const mTexture* texture, GLint wrapS, GLint wrapT);

#endif //PEACH_TEXTURE_H

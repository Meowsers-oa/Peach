//
// Created by Štěpán Toman on 20.09.2026.
//

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Texture.h"
#include <stdio.h>
#include <stdlib.h>

mTexture mLoadTexture(const char* filePath) {
    mTexture texture = {0};
    if (!filePath) {
        fprintf(stderr, "[Peach Texture] Error: filePath is NULL\n");
        return texture;
    }

    stbi_set_flip_vertically_on_load(1);

    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);
    if (!data) {
        fprintf(stderr, "[Peach Texture] Failed to load image at '%s': %s\n", filePath, stbi_failure_reason());
        return texture;
    }

    texture = mCreateTexture(width, height, channels, data);
    stbi_image_free(data);

    return texture;
}

mTexture mCreateTexture(int width, int height, int channels, const unsigned char* data) {
    mTexture texture = {0};
    texture.width = width;
    texture.height = height;
    texture.channels = channels;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum internalFormat = GL_RGBA8;
    GLenum dataFormat = GL_RGBA;

    if (channels == 1) {
        internalFormat = GL_R8;
        dataFormat = GL_RED;
    } else if (channels == 2) {
        internalFormat = GL_RG8;
        dataFormat = GL_RG;
    } else if (channels == 3) {
        internalFormat = GL_RGB8;
        dataFormat = GL_RGB;
    } else if (channels == 4) {
        internalFormat = GL_RGBA8;
        dataFormat = GL_RGBA;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

mTexture mCreateColorTexture(Color color) {
    unsigned char pixel[4] = {
        (unsigned char)(color.r * 255.0f),
        (unsigned char)(color.g * 255.0f),
        (unsigned char)(color.b * 255.0f),
        (unsigned char)(color.a * 255.0f)
    };
    return mCreateTexture(1, 1, 4, pixel);
}

void mUnloadTexture(mTexture* texture) {
    if (texture && texture->id != 0) {
        glDeleteTextures(1, &texture->id);
        texture->id = 0;
        texture->width = 0;
        texture->height = 0;
        texture->channels = 0;
    }
}

void mDeleteTexture(mTexture* texture) {
    mUnloadTexture(texture);
}

void mDeleteTextureId(unsigned int textureId) {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
    }
}

void mBindTexture(const mTexture* texture, unsigned int slot) {
    if (texture && texture->id != 0) {
        mBindTextureId(texture->id, slot);
    }
}

void mBindTextureId(unsigned int textureId, unsigned int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void mUnbindTexture(unsigned int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void mSetTextureFilter(const mTexture* texture, GLint minFilter, GLint magFilter) {
    if (texture && texture->id != 0) {
        glBindTexture(GL_TEXTURE_2D, texture->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void mSetTextureWrap(const mTexture* texture, GLint wrapS, GLint wrapT) {
    if (texture && texture->id != 0) {
        glBindTexture(GL_TEXTURE_2D, texture->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

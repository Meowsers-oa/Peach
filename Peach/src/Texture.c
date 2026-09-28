#include <Peach/Texture.h>
#include <Peach/Renderer.h>
#include <stb_image.h>
#include <stdio.h>

int mTextureCreate(mTexture* texture, int width, int height, const unsigned char* pixels) {
    if (texture == NULL || texture->handle != 0 || pixels == NULL || width <= 0 || height <= 0) return M_FAILURE;
    int maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if (width > maxSize || height > maxSize) return M_FAILURE;

    int binding, alignment, rowLength, skipRows, skipPixels, unpackBuffer;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &binding);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);

    unsigned int handle = 0;
    glGenTextures(1, &handle);
    if (handle == 0) return M_FAILURE;
    glBindTexture(GL_TEXTURE_2D, handle);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int uploadedWidth = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &uploadedWidth);
    glBindTexture(GL_TEXTURE_2D, (unsigned int)binding);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (unsigned int)unpackBuffer);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
    if (uploadedWidth != width) {
        glDeleteTextures(1, &handle);
        return M_FAILURE;
    }

    *texture = (mTexture){.handle = handle, .width = width, .height = height};
    return M_SUCCESS;
}

void mTextureDestroy(mContext* ctx, mTexture* texture) {
    if (texture == NULL || texture->handle == 0) return;
    mRendererFlush(ctx);
    glDeleteTextures(1, &texture->handle);
    *texture = (mTexture){0};
}

int mTextureLoad(mTexture* texture, const char* path) {
    if (texture == NULL || texture->handle != 0 || path == NULL) return M_FAILURE;
    int width, height;
    stbi_set_flip_vertically_on_load_thread(M_FALSE);
    unsigned char* pixels = stbi_load(path, &width, &height, NULL, STBI_rgb_alpha);
    if (pixels == NULL) {
        printf("Failed to load texture %s: %s\n", path, stbi_failure_reason());
        return M_FAILURE;
    }
    int result = mTextureCreate(texture, width, height, pixels);
    stbi_image_free(pixels);
    return result;
}

int mTextureLoadMemory(mTexture* texture, const unsigned char* data, int size) {
    if (texture == NULL || texture->handle != 0 || data == NULL || size <= 0) return M_FAILURE;
    int width, height;
    stbi_set_flip_vertically_on_load_thread(M_FALSE);
    unsigned char* pixels = stbi_load_from_memory(data, size, &width, &height, NULL, STBI_rgb_alpha);
    if (pixels == NULL) {
        printf("Failed to decode texture: %s\n", stbi_failure_reason());
        return M_FAILURE;
    }
    int result = mTextureCreate(texture, width, height, pixels);
    stbi_image_free(pixels);
    return result;
}

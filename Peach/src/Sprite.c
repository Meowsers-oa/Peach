//
// Created by Štěpán Toman on 28.09.2026.
//

#include <Peach/Sprite.h>
#include <Peach/Map.h>
#include <Peach/Resource.h>
#include <Peach/Renderer.h>
#include <math.h>
#include <inttypes.h>
#include <stdio.h>

static mSprite* storeSprite(mContext* ctx, mSprite sprite) {
    char key[21];
    do {
        sprite.id = rand_ui64();
        snprintf(key, sizeof(key), "%" PRIu64, sprite.id);
    } while (mMapContains(ctx->sprites, key));

    if (!mMapSet(ctx->sprites, key, mSprite, sprite)) return NULL;
    return mMapGet(ctx->sprites, key);
}

mSprite* mSpriteCreate(mContext* ctx, const char* textureResourceLocation) {
    if (ctx == NULL || ctx->sprites == NULL || textureResourceLocation == NULL) return NULL;
    if (mResourceTypeOf(ctx, textureResourceLocation) != M_RESOURCE_TEXTURE) return NULL;
    mTexture* texPtr = mGetResource(ctx, textureResourceLocation);
    if (texPtr == NULL || texPtr->handle == 0 || texPtr->width <= 0 || texPtr->height <= 0) {
        printf("Invalid resource location: %s", textureResourceLocation);
        return NULL;
    }

    mSprite sprite = {.texture = *texPtr, .width = texPtr->width,
                      .height = texPtr->height, .scale = 1.0f};
    return storeSprite(ctx, sprite);
}

mSprite* mSpriteCreateFromSheet(mContext* ctx, const char* resourceLocation, int frame) {
    if (ctx == NULL || ctx->sprites == NULL ||
        mResourceTypeOf(ctx, resourceLocation) != M_RESOURCE_SPRITE_SHEET) return NULL;
    mSpriteSheet* sheet = mGetResource(ctx, resourceLocation);
    mSprite sprite = {.scale = 1.0f};
    if (mSpriteSetFrame(&sprite, sheet, frame) == M_FAILURE) return NULL;
    sprite.width = sprite.sourceWidth;
    sprite.height = sprite.sourceHeight;
    return storeSprite(ctx, sprite);
}

int mSpriteSetFrame(mSprite* sprite, const mSpriteSheet* sheet, int frame) {
    if (sprite == NULL || sheet == NULL || sheet->sprites == NULL || sheet->info.texture.handle == 0 ||
        frame < 0 || frame >= sheet->spritesAmount) return M_FAILURE;
    const mSprite* source = &sheet->sprites[frame];
    sprite->texture = sheet->info.texture;
    sprite->sourceX = source->sourceX;
    sprite->sourceY = source->sourceY;
    sprite->sourceWidth = source->sourceWidth;
    sprite->sourceHeight = source->sourceHeight;
    return M_SUCCESS;
}

void mSpriteSetSize(mContext* ctx, mSprite* sprite, int width, int height) {
    if (ctx == NULL || sprite == NULL || width <= 0 || height <= 0) return;
    sprite->width = width;
    sprite->height = height;
}

void mSpriteSetPos(mContext* ctx, mSprite* sprite, int x, int y) {
    if (ctx == NULL || sprite == NULL) return;
    sprite->x = x;
    sprite->y = y;
}

void mSpriteSetScale(mContext* ctx, mSprite* sprite, float scale) {
    if (ctx == NULL || sprite == NULL || !isfinite(scale) || scale < 0.0f) return;
    sprite->scale = scale;
}

int mDrawSprite(mContext* ctx, const mSprite* sprite) {
    if (ctx == NULL || ctx->renderer == NULL || sprite == NULL || sprite->texture.handle == 0 ||
        sprite->width <= 0 || sprite->height <= 0 || !isfinite(sprite->scale) || sprite->scale < 0.0f) return M_FAILURE;
    if (sprite->scale == 0.0f) return M_SUCCESS;

    float x = (float)sprite->x;
    float y = (float)sprite->y;
    float right = x + (float)sprite->width * sprite->scale;
    float bottom = y + (float)sprite->height * sprite->scale;
    if (!isfinite(right) || !isfinite(bottom)) return M_FAILURE;
    int sourceWidth = sprite->sourceWidth != 0 ? sprite->sourceWidth : sprite->texture.width;
    int sourceHeight = sprite->sourceHeight != 0 ? sprite->sourceHeight : sprite->texture.height;
    if (sourceWidth <= 0 || sourceHeight <= 0 || sprite->sourceX < 0 || sprite->sourceY < 0 ||
        sourceWidth > sprite->texture.width || sourceHeight > sprite->texture.height ||
        sprite->sourceX > sprite->texture.width - sourceWidth ||
        sprite->sourceY > sprite->texture.height - sourceHeight) return M_FAILURE;
    float u = (float)sprite->sourceX / sprite->texture.width;
    float v = (float)sprite->sourceY / sprite->texture.height;
    float uEnd = (float)(sprite->sourceX + sourceWidth) / sprite->texture.width;
    float vEnd = (float)(sprite->sourceY + sourceHeight) / sprite->texture.height;
    mVertex vertices[] = {
        {.x = x, .y = y, .u = u, .v = v, .color = M_COLOR_WHITE},
        {.x = right, .y = y, .u = uEnd, .v = v, .color = M_COLOR_WHITE},
        {.x = right, .y = bottom, .u = uEnd, .v = vEnd, .color = M_COLOR_WHITE},
        {.x = x, .y = bottom, .u = u, .v = vEnd, .color = M_COLOR_WHITE}
    };
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    return mAddVerticesIndexed(ctx, vertices, 4, indices, 6, NULL, &sprite->texture);
}

void mSpriteDestroy(mContext* ctx, mSprite* sprite) {
    if (ctx == NULL || sprite == NULL) return;
    char key[21];
    snprintf(key, sizeof(key), "%" PRIu64, sprite->id);
    if (mMapGet(ctx->sprites, key) == sprite) mMapRemove(ctx->sprites, key);
}

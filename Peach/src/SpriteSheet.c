//
// Created by Štěpán Toman on 29.09.2026.
//

#include <Peach/SpriteSheet.h>

#include <Peach/Texture.h>

#include <stdlib.h>
#include <limits.h>

mSpriteSheet mCreateSpriteSheet(mSpriteSheetInfo info, const char* filePath) {
    mSpriteSheet sheet = {0};
    if (info.spriteWidth <= 0 || info.spriteHeight <= 0 || info.spritesAmount < 0) return sheet;
    if (mTextureLoad(&sheet.info.texture, filePath) == M_FAILURE) return sheet;

    mTexture* texture = &sheet.info.texture;
    int columns = texture->width / info.spriteWidth;
    int rows = texture->height / info.spriteHeight;
    size_t count = (size_t)columns * rows;
    if (texture->width % info.spriteWidth != 0 || texture->height % info.spriteHeight != 0 ||
        count == 0 || count > INT_MAX || (size_t)info.spritesAmount > count) {
        mSpriteSheetDestroy(NULL, &sheet);
        return sheet;
    }
    if (info.spritesAmount != 0) count = info.spritesAmount;
    if (count > SIZE_MAX / sizeof(mSprite)) {
        mSpriteSheetDestroy(NULL, &sheet);
        return sheet;
    }
    sheet.sprites = calloc(count, sizeof(mSprite));
    if (sheet.sprites == NULL) {
        mSpriteSheetDestroy(NULL, &sheet);
        return sheet;
    }
    sheet.info.spriteWidth = info.spriteWidth;
    sheet.info.spriteHeight = info.spriteHeight;
    sheet.info.spritesAmount = (int)count;
    sheet.spritesAmount = (int)count;
    for (int i = 0; i < sheet.spritesAmount; i++) {
        sheet.sprites[i] = (mSprite){
            .texture = *texture, .width = info.spriteWidth, .height = info.spriteHeight, .scale = 1.0f,
            .sourceX = (i % columns) * info.spriteWidth, .sourceY = (i / columns) * info.spriteHeight,
            .sourceWidth = info.spriteWidth, .sourceHeight = info.spriteHeight
        };
    }
    return sheet;
}

void mSpriteSheetDestroy(mContext* ctx, mSpriteSheet* sheet) {
    if (sheet == NULL) return;
    mTextureDestroy(ctx, &sheet->info.texture);
    free(sheet->sprites);
    *sheet = (mSpriteSheet){0};
}

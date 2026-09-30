//
// Created by Štěpán Toman on 28.09.2026.
//

#ifndef PEACH_SPRITE_H
#define PEACH_SPRITE_H

#include <Peach/Common.h>

// Returns a map-owned sprite; the texture is borrowed from the resource pool.
mSprite* mSpriteCreate(mContext* ctx, const char* resourceLocation);
mSprite* mSpriteCreateFromSheet(mContext* ctx, const char* resourceLocation, int frame);
int mSpriteSetFrame(mSprite* sprite, const mSpriteSheet* sheet, int frame);
// Pixel size and top-left position. Scale 0 hides the sprite.
void mSpriteSetSize(mContext* ctx, mSprite* sprite, int width, int height);
void mSpriteSetPos(mContext* ctx, mSprite* sprite, int x, int y);
void mSpriteSetScale(mContext* ctx, mSprite* sprite, float scale);
int mDrawSprite(mContext* ctx, const mSprite* sprite);
void mSpriteDestroy(mContext* ctx, mSprite* sprite);

#endif //PEACH_SPRITE_H

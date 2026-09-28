//
// Created by Štěpán Toman on 28.09.2026.
//

#ifndef PEACH_SPRITE_H
#define PEACH_SPRITE_H

#include <Peach/Common.h>

// Creates a sprite stored in ctx->sprites, using a texture from the resource pool.
// The returned pointer remains valid until sprite removal or context destruction.
// Sprites borrow the GPU texture; keep it alive until all queued draws finish.
mSprite* mSpriteCreate(mContext* ctx, const char* resourceLocation);
// Size and position use window pixels; (x, y) is the top-left corner.
// Final size is width/height multiplied by scale. Invalid values are ignored.
void mSpriteSetSize(mContext* ctx, mSprite* sprite, int width, int height);
void mSpriteSetPos(mContext* ctx, mSprite* sprite, int x, int y);
// Nonnegative uniform scale. Zero hides the sprite without removing it.
void mSpriteSetScale(mContext* ctx, mSprite* sprite, float scale);
int mDrawSprite(mContext* ctx, const mSprite* sprite);
// Removes the stored sprite and invalidates its pointer; does not delete its texture.
void mSpriteDestroy(mContext* ctx, mSprite* sprite);

#endif //PEACH_SPRITE_H

#include <Peach/Animation.h>
#include <Peach/Sprite.h>
#include <Peach/Resource.h>
#include <math.h>

static M_BOOL validAnimation(const mAnimation* animation) {
    return animation != NULL && animation->sheet != NULL && animation->sheet->sprites != NULL &&
        animation->textureHandle != 0 && animation->sheet->info.texture.handle == animation->textureHandle &&
        animation->firstFrame >= 0 && animation->frameCount > 0 &&
        animation->frameCount <= animation->sheet->spritesAmount &&
        animation->firstFrame <= animation->sheet->spritesAmount - animation->frameCount &&
        isfinite(animation->fps) && animation->fps > 0;
}

mAnimation* mAnimationCreate(mContext* ctx, const char* id, const char* sheet,
                            int firstFrame, int frameCount, double fps, M_BOOL loop) {
    if (ctx == NULL || ctx->animations == NULL || id == NULL || mMapContains(ctx->animations, id) ||
        mResourceTypeOf(ctx, sheet) != M_RESOURCE_SPRITE_SHEET) return NULL;
    const mSpriteSheet* source = mGetResource(ctx, sheet);
    mAnimation animation = {.sheet = source, .textureHandle = source->info.texture.handle,
        .firstFrame = firstFrame, .frameCount = frameCount, .fps = fps, .loop = loop};
    if (!validAnimation(&animation) || !mMapSet(ctx->animations, id, mAnimation, animation)) return NULL;
    return mMapGet(ctx->animations, id);
}

mAnimation* mAnimationGet(mContext* ctx, const char* id) {
    return ctx != NULL ? mMapGet(ctx->animations, id) : NULL;
}

void mAnimationDestroy(mContext* ctx, const char* id) {
    mAnimation* animation = mAnimationGet(ctx, id);
    if (animation == NULL) return;
    mMapIter iter = mMap_iter(ctx->sprites);
    mMapEntry entry;
    while (mMap_next(&iter, &entry)) {
        if (entry.valueSize != sizeof(mSprite)) continue;
        mSprite* sprite = entry.value;
        if (sprite->animation == animation) {
            sprite->animation = NULL;
            sprite->playing = M_FALSE;
        }
    }
    mMapRemove(ctx->animations, id);
}

int mSpriteSetAnimation(mSprite* sprite, const mAnimation* animation) {
    if (sprite == NULL || !validAnimation(animation)) return M_FAILURE;
    if (mSpriteSetFrame(sprite, animation->sheet, animation->firstFrame) == M_FAILURE) return M_FAILURE;
    sprite->animation = animation;
    sprite->frame = 0;
    sprite->frameTime = 0;
    sprite->playing = M_FALSE;
    return M_SUCCESS;
}

void mSpritePlay(mSprite* sprite) {
    if (sprite == NULL || !validAnimation(sprite->animation)) return;
    if (!sprite->animation->loop && sprite->frameTime >= 1.0 / sprite->animation->fps) mSpriteStop(sprite);
    sprite->playing = M_TRUE;
}

void mSpritePause(mSprite* sprite) {
    if (sprite != NULL) sprite->playing = M_FALSE;
}

void mSpriteStop(mSprite* sprite) {
    if (sprite == NULL) return;
    sprite->playing = M_FALSE;
    sprite->frame = 0;
    sprite->frameTime = 0;
    if (validAnimation(sprite->animation)) {
        mSpriteSetFrame(sprite, sprite->animation->sheet, sprite->animation->firstFrame);
    }
}

void mAnimationUpdate(mContext* ctx, double deltaTime) {
    if (ctx == NULL || !isfinite(deltaTime) || deltaTime <= 0) return;
    mMapIter iter = mMap_iter(ctx->sprites);
    mMapEntry entry;
    while (mMap_next(&iter, &entry)) {
        if (entry.valueSize != sizeof(mSprite)) continue;
        mSprite* sprite = entry.value;
        if (!sprite->playing) continue;
        const mAnimation* animation = sprite->animation;
        if (!validAnimation(animation)) {
            sprite->playing = M_FALSE;
            continue;
        }
        double duration = animation->frameCount / animation->fps;
        if (!isfinite(duration) || duration <= 0) continue;
        double elapsed = sprite->frame / animation->fps + sprite->frameTime;
        if (animation->loop) {
            elapsed = fmod(elapsed + fmod(deltaTime, duration), duration);
        } else if (deltaTime >= duration - elapsed) {
            sprite->frame = animation->frameCount - 1;
            sprite->frameTime = 1.0 / animation->fps;
            sprite->playing = M_FALSE;
            mSpriteSetFrame(sprite, animation->sheet, animation->firstFrame + sprite->frame);
            continue;
        } else {
            elapsed += deltaTime;
        }
        sprite->frame = (int)fmin(floor(elapsed * animation->fps), animation->frameCount - 1);
        sprite->frameTime = elapsed - sprite->frame / animation->fps;
        mSpriteSetFrame(sprite, animation->sheet, animation->firstFrame + sprite->frame);
    }
}

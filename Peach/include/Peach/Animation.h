#ifndef PEACH_ANIMATION_H
#define PEACH_ANIMATION_H

#include <Peach/Common.h>

// Uses consecutive frames from a registered sheet. The context owns the animation.
mAnimation* mAnimationCreate(mContext* ctx, const char* id, const char* sheet,
                            int firstFrame, int frameCount, double fps, M_BOOL loop);
mAnimation* mAnimationGet(mContext* ctx, const char* id);
void mAnimationDestroy(mContext* ctx, const char* id);
int mSpriteSetAnimation(mSprite* sprite, const mAnimation* animation);
void mSpritePlay(mSprite* sprite);
void mSpritePause(mSprite* sprite);
void mSpriteStop(mSprite* sprite);
// Called by mUpdate with delta time in seconds.
void mAnimationUpdate(mContext* ctx, double deltaTime);

#endif //PEACH_ANIMATION_H

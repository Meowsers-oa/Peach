#include <Peach/Peach.h>

int main() {
    mContext ctx = mContextCreate();
    mWindowInfo info = {.width = 1280, .height = 720, .title = "Peach App"};
    mWindowCreate(&ctx, &info);
    mRendererSetResolution(&ctx, 320, 180);

    mSpriteSheet sheet = mCreateSpriteSheet(
        (mSpriteSheetInfo){.spriteWidth = 16, .spriteHeight = 16},
        PEACH_SANDBOX_ASSET_DIR "/TinyDungeon.png"
    );
    if (sheet.spritesAmount != 132 || !mAddSpriteSheet(&ctx, "dungeon", sheet)) {
        mSpriteSheetDestroy(&ctx, &sheet);
        mEnd(&ctx);
        return 1;
    }

    mSprite* sprites[132];
    for (int i = 0; i < 132; i++) {
        sprites[i] = mSpriteCreateFromSheet(&ctx, "dungeon", i);
        if (sprites[i] == NULL) {
            mEnd(&ctx);
            return 1;
        }
        mSpriteSetPos(&ctx, sprites[i], 64 + (i % 12) * 16, 2 + (i / 12) * 16);
    }

    mAnimation* preview = mAnimationCreate(&ctx, "preview", "dungeon", 0, 132, 8, M_TRUE);
    mSprite* animated = mSpriteCreateFromSheet(&ctx, "dungeon", 0);
    if (animated == NULL || mSpriteSetAnimation(animated, preview) == M_FAILURE) {
        mEnd(&ctx);
        return 1;
    }
    mSpriteSetPos(&ctx, animated, 16, 74);
    mSpriteSetScale(&ctx, animated, 2);
    mSpritePlay(animated);

    mLightSetAmbient(&ctx, M_COLOR(.5f, .5f, .55f, 1));
    mLight* warm = mLightCreate(&ctx, "warm");
    mLightMake(warm, 130, 80, 100, M_COLOR(1, .7, .4, 1), 1.5f);

    mLight* blue = mLightCreate(&ctx, "blue");
    mLightMake(blue, 210, 110, 70, M_COLOR(.3f, .5f, 1, 1), 1.f);

    while (ctx.window.running) {
        double x, y;
        mMousePosition(&ctx, &x, &y);
        warm->x = (float)x;
        warm->y = (float)y;

        for (int i = 0; i < 132; i++) mDrawSprite(&ctx, sprites[i]);
        mDrawSprite(&ctx, animated);
        mUpdate(&ctx);
    }

    mEnd(&ctx);
    return 0;
}

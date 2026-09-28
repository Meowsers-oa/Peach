#include <Peach/Peach.h>
#include <stdio.h>

int main() {
    mContext ctx = mContextCreate();
    mWindowInfo info = {.width = 1200, .height = 800, .title = "Peach App"};
    mWindowCreate(&ctx, &info);

    mCamera camera;
    mCameraCreate(&camera, (float)info.width, (float)info.height);
    mRendererSetCamera(&ctx, &camera);

    mSprite* checkerSprite = NULL;
    mTexture checker = {0};

    mTextureLoad(&checker, PEACH_SANDBOX_ASSET_DIR "/Checker.png");
    mAddResource(&ctx, "checkerTex", mTexture, checker);

    checker = (mTexture){0};
    checkerSprite = mSpriteCreate(&ctx, "checkerTex");

    mSpriteSetPos(&ctx, checkerSprite, 350, 100);
    mSpriteSetSize(&ctx, checkerSprite, 128, 128);
    mSpriteSetScale(&ctx, checkerSprite, 2.0f);

    while (ctx.window.running) {
        mDrawSprite(&ctx, checkerSprite);

        mUpdate(&ctx);
    }

    mTextureDestroy(&ctx, &checker);
    mEnd(&ctx);
    return 0;
}

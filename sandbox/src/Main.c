#include <Peach/Peach.h>

int main() {
    mContext ctx = mContextCreate();
    mWindowInfo info = {.width = 1200, .height = 800, .title = "Peach App"};
    mWindowCreate(&ctx, &info);

    mCamera camera;
    mCameraCreate(&camera, (float)info.width, (float)info.height);
    mRendererSetCamera(&ctx, &camera);

    mTexture checker = {0};
    mTextureLoad(&checker, PEACH_SANDBOX_ASSET_DIR "/Checker.png");
    mAddResource(&ctx, "checkerTex", mTexture, checker);

    mSprite* checkerSprite = mSpriteCreate(&ctx, "checkerTex");

    mSpriteSetPos(&ctx, checkerSprite, 350, 100);
    mSpriteSetSize(&ctx, checkerSprite, 128, 128);
    mSpriteSetScale(&ctx, checkerSprite, 2.0f);

    while (ctx.window.running) {
        mDrawSprite(&ctx, checkerSprite);
        mUpdate(&ctx);
    }

    mEnd(&ctx);
    return 0;
}

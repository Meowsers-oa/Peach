#include <Peach/Peach.h>

int main() {
    mContext ctx = mContextCreate();
    mWindowInfo info = {.width = 1280, .height = 720, .title = "Peach App"};
    mWindowCreate(&ctx, &info);
    mRendererSetResolution(&ctx, 320, 180);

    mTexture checker = {0};
    mTextureLoad(&checker, PEACH_SANDBOX_ASSET_DIR "/Checker.png");
    mAddResource(&ctx, "checkerTex", mTexture, checker);
    mSprite* sprite = mSpriteCreate(&ctx, "checkerTex");
    mSpriteSetPos(&ctx, sprite, 96, 26);
    mSpriteSetSize(&ctx, sprite, 128, 128);

    mLightSetAmbient(&ctx, M_COLOR(.08f, .08f, .12f, 1));
    mLight* warm = mLightCreate(&ctx, "warm");
    mLightMake(warm, 130, 80, 100, M_COLOR(1, .7, .4, 1), 1.5f);

    mLight* blue = mLightCreate(&ctx, "blue");
    mLightMake(blue, 210, 110, 70, M_COLOR(.3f, .5f, 1, 1), 1.f);

    while (ctx.window.running) {
        double x, y;
        mMousePosition(&ctx, &x, &y);
        mRendererWindowToScreen(&ctx, x, y, &x, &y);
        warm->x = (float)x;
        warm->y = (float)y;

        mDrawSprite(&ctx, sprite);
        mUpdate(&ctx);
    }

    mEnd(&ctx);
    return 0;
}

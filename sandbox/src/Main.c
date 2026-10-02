#include <Peach/Peach.h>
#include <string.h>

int main(int argc, char** argv) {
    mContext ctx = mContextCreate();
    mWindowInfo info = {.width = 1280, .height = 720, .title = "Peach App"};
    mWindowCreate(&ctx, &info);
    mRendererSetResolution(&ctx, 320, 180);

    mLightSetAmbient(&ctx, M_COLOR(.5f, .5f, .55f, 1));
    mLight* warm = mLightCreate(&ctx, "warm");
    mLightMake(warm, 130, 80, 100, M_COLOR(1, .7, .4, 1), 1.5f);


    mParticleEmitter sparks = mParticleEmitterCreate(160, 90, 256, 32, 3, 0,
        M_COLOR_GOLD, M_COLOR(1, .2f, 0, 0));
    if (sparks.particles == NULL || !mAddParticleEmitter(&ctx, "sparks", sparks)) {
        mParticleEmitterDestroy(&sparks);
        mEnd(&ctx);
        return 1;
    }
    mParticleEmitter* emitter = mGetResource(&ctx, "sparks");
    mParticlesStartSpawning(emitter);

    while (ctx.window.running) {
        double x, y;
        mMousePosition(&ctx, &x, &y);
        warm->x = (float)x;
        warm->y = (float)y;
        emitter->x = (float)x;
        emitter->y = (float)y;
        mParticleEmitterUpdate(&ctx, emitter);
        mDrawParticles(&ctx, emitter);
        mUpdate(&ctx);

        char title[64];
        glfwSetWindowTitle(
            ctx.window.handle,
            mFormatString(title, sizeof(title), "%s | %.0f FPS", info.title, ctx.time.averageFPS)
        );
    }
    mEnd(&ctx);
    return 0;
}
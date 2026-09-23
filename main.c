#include "Peach/Peach.h"
#include <math.h>
static void drawShowcase(mContext* ctx, float angle) {
    const mColor plaster = {0.76f, 0.78f, 0.82f, 1.0f};
    const mColor floor = {0.58f, 0.61f, 0.66f, 1.0f};
    const mColor objects = {0.88f, 0.88f, 0.88f, 1.0f};

    mAddQuad3D(ctx, (vec3){0, 0, 0}, (vec2){16, 12}, (vec3){0, 1, 0}, floor);
    mAddQuad3D(ctx, (vec3){0, 3, -6}, (vec2){16, 6}, (vec3){0, 0, 1}, plaster);
    mAddCube(ctx, (vec3){0, 0.12f, -5.85f}, (vec3){16, 0.24f, 0.3f}, plaster);

    mAddCubeRotated(ctx, (vec3){-3.2f, 1.1f, -0.8f}, (vec3){1.5f, 2.2f, 1.5f},
                    (vec3){0, 0.25f, 0}, objects);
    mAddCubeRotated(ctx, (vec3){-1.4f, 0.45f, 1.1f}, (vec3){1.1f, 0.9f, 1.1f},
                    (vec3){0, -0.25f, 0}, objects);

    mAddCubeRotated(ctx, (vec3){3.0f, 1.7f, 0.0f}, (vec3){1.45f, 1.45f, 1.45f},
                    (vec3){0.2f, angle, 0.12f}, objects);
    mAddCube(ctx, (vec3){4.6f, 0.5f, -2.0f}, (vec3){0.65f, 1.0f, 0.65f}, objects);
}


int main(void) {
    mContext ctx = mCreateContext();
    ctx.window.clearColor = (mColor){0.018f, 0.023f, 0.035f, 1.0f};
    if (mWindowCreate(&ctx, 1200, 800, "Peach Title") != 0) return 1;
    mLoadDefaultShaders(&ctx);

    vec3 cameraPos = {9.5f, 8.5f, 15.5f};
    vec3 cameraTarget = {0.0f, 1.0f, -0.8f};
    mCamera3D camera = mCreateCameraPerspective(cameraPos, cameraTarget, 48.0f);

    mLight point = mCreatePointLight((vec3){-4.8f, 3.6f, 2.0f},
                                     (mColor){1.0f, 0.62f, 0.30f, 1.0f}, 24.0f);
    point.range = 13.0f;
    mLight spot = mCreateSpotLight((vec3){4.0f, 6.5f, 2.5f},
                                   (vec3){-0.6f, -6.5f, -3.0f},
                                   (mColor){0.34f, 0.66f, 1.0f, 1.0f}, 80.0f);
    spot.range = 16.0f;
    spot.innerCone = 18.0f;
    spot.outerCone = 27.0f;
    addLight(&ctx, &point);
    addLight(&ctx, &spot);
    mRendererSetAmbientLight(&ctx, 0.065f);

    float angle = 0.35f;

    while (ctx.window.running) {
        angle = fmodf(angle + mGetDeltaTime(&ctx) * 0.35f, 2.0f * GLM_PIf);
        mCameraUpdateFree(&camera, mGetDeltaTime(&ctx), 6.0f, 0.15f);
        mRendererSetCamera3D(&ctx, &camera);

        mRendererBegin(&ctx);

        drawShowcase(&ctx, angle);

        mRendererEnd(&ctx);
        mUpdate(&ctx);
    }

    mEnd(&ctx);
    return 0;
}

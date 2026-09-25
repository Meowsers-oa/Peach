#include "Peach/Peach.h"

int main() {
    mContext ctx;
    if (!mInit(&ctx, 1200, 800, "Peach Title")) return 1;

    ctx.window.clearColor = (mColor){.045f, .055f, .075f, 1};
    mRendererSetAmbientLight(&ctx, .12f);
    mCamera3D camera = mCreateCameraPerspective((vec3){7, 5, 8}, (vec3){0, 1, 0}, 55);

    mLight light = mCreatePointLight((vec3){-2.5f, 4, 2},
                                     (mColor){.r = 1, .g = .85f, .b = .65f, .a = 1}, 35);
    light.range = 18;
    mRendererAddLight(&ctx, &light);

    mMesh yoshiMesh = {0};

    if (!mMeshLoadEx(PEACH_ASSET_DIR "/models/Yoshi/Yoshi.obj", &yoshiMesh, mMeshImportFlipFacing)) {
        mEnd(&ctx);
        return 1;
    }

    mTransform yoshiTransform = {.position = {0, 2.73f, 0}, .scale = {.015f, .015f, .015f}};

    mObject yoshi = mMakeObject(&yoshiMesh, &yoshiTransform);

    while (mBeginFrame(&ctx)) {
        if (!mUiWantsMouse(&ctx) && !mUiWantsKeyboard(&ctx))
            mCameraUpdateFree(&camera, mGetDeltaTime(&ctx), 6.f, .15f);
        mRendererSetCamera3D(&ctx, &camera);

        mUiSetNextWindowPosition(760, 20, 1);
        mUiSetNextWindowSize(420, 230, 1);
        mUiBeginWindow("Properties");
        mUiDragFloat3("Light position", light.position, .05f, -10, 10);
        mUiSliderFloat("Intensity", &light.intensity, 0, 80);
        mUiColorEdit("Light color", &light.color);
        mUiCheckbox("Shadows", &light.castsShadows);
        mUiEndWindow();

        mUiStatsPanel(&ctx);
        mUiGizmoPosition(&ctx, &camera, 0, light.position);

        mAddQuad3D(&ctx, (vec3){0, 0, 0}, (vec2){14, 14}, (vec3){0, 1, 0}, (mColor){.r = .55f, .g = .59f, .b = .65f, .a = 1});
        mDraw(&ctx, &yoshi);
        mDrawLightMarkers(&ctx, .15f);

        mEndFrame(&ctx);
    }

    mMeshFree(&yoshiMesh);
    mEnd(&ctx);
    return 0;
}

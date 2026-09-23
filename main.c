#include "Peach/Peach.h"

int main() {
    mContext ctx;
    if (!mInit(&ctx, 1200, 800, "Peach Title")) return 1;
    if (!mSetPostProcess(&ctx, "postprocess.frag")) {
        mEnd(&ctx);
        return 1;
    }

    ctx.window.clearColor = (mColor){0.045f, 0.055f, 0.075f, 1};
    mRendererSetAmbientLight(&ctx, 0.12f);
    mCamera3D camera = mCreateCameraPerspective((vec3){7, 5, 8}, (vec3){0, 1, 0}, 55);

    mLight light = mCreatePointLight((vec3){-2.5f, 4, 2},
                                     (mColor){1, 0.85f, 0.65f, 1}, 35);
    light.range = 18;
    mRendererAddLight(&ctx, &light);

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
        mUiText("Drag values, or click to type.");
        mUiText("RMB: look. WASD / Q / E: move.");
        mUiEndWindow();
        mUiGizmoPosition(&ctx, &camera, 0, light.position);

        mAddQuad3D(&ctx, (vec3){0, 0, 0}, (vec2){14, 14}, (vec3){0, 1, 0},
                   (mColor){0.55f, 0.59f, 0.65f, 1});
        mAddCubeRotated(&ctx, (vec3){0, 1, 0}, (vec3){2, 2, 2}, (vec3){0, .3f, 0},
                        (mColor){0.35f, 0.6f, 0.85f, 1});
        mDrawLightMarker(&ctx, &light, .15f);

        mEndFrame(&ctx);
    }

    mEnd(&ctx);
    return 0;
}

#include "Peach/Peach.h"

int main(void) {
    mContext ctx = mCreateContext();
    mWindowCreate(&ctx, 800, 600, "Peach 3D");

    mLoadShaders(&ctx, "../Peach/resources/shaders/vertex.glsl", "../Peach/resources/shaders/fragment.glsl");

    unsigned char checkerPixels[] = {
        255, 255, 255, 255,   200, 50,  50,  255,
        50,  200, 50,  255,   50,  50,  200, 255
    };
    mTexture testTexture = mCreateTexture(2, 2, 4, checkerPixels);

    vec3 cameraPos = { 0.0f, 3.0f, 7.0f };
    vec3 cameraTarget = { 0.0f, 0.0f, 0.0f };
    mCamera3D camera = mCreateCameraPerspective(cameraPos, cameraTarget, 45.0f);

    while (ctx.window.running) {
        mCameraUpdateFree(&camera, ctx.time.deltaTime, 10.0f, 0.15f);

        mRendererSetCamera3D(&ctx, &camera);

        mRendererBegin(&ctx);

        mAddCubeTextured(&ctx, (vec3){ 0.0f, 0.0f, 0.0f }, (vec3){ 1.5f, 1.5f, 1.5f }, &testTexture, WHITE);

        mAddCube(&ctx, (vec3){ -3.0f, 0.0f, 0.0f }, (vec3){ 1.0f, 1.0f, 1.0f }, RED);
        mAddCube(&ctx, (vec3){  3.0f, 0.0f, 0.0f }, (vec3){ 1.0f, 1.0f, 1.0f }, BLUE);
        mAddCube(&ctx, (vec3){  0.0f, 2.5f, 0.0f }, (vec3){ 0.8f, 0.8f, 0.8f }, GOLD);

        mAddQuad3D(&ctx, (vec3){ 0.0f, -1.0f, 0.0f }, (vec2){ 12.0f, 12.0f }, (vec3){ 0.0f, 1.0f, 0.0f }, DARK_GRAY);

        mRendererEnd(&ctx);

        mUpdate(&ctx);

        if (mIsKeyDown(KEY_ESCAPE)) {
            mWindowStop(&ctx);
        }
    }

    mDeleteTexture(&testTexture);
    mEnd(&ctx);
    return 0;
}

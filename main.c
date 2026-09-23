#include "Peach/Peach.h"

int main(void) {
    mContext ctx = mCreateContext();
    if (!mWindowCreate(&ctx, 1200, 800, "Peach Title")) return 1;
    if (!mSetPostProcess(&ctx, "postprocess.frag")) {
        mEnd(&ctx);
        return 1;
    }

    mCamera3D camera = mCreateCameraPerspective((vec3){0, 0, 0}, (vec3){0, 0, -1}, 80);

    while (mBeginFrame(&ctx)) {
        mCameraUpdateFree(&camera, mGetDeltaTime(&ctx), 6.f, .15f);
        mRendererSetCamera3D(&ctx, &camera);

        mEndFrame(&ctx);
    }

    mEnd(&ctx);
    return 0;
}

#include <Peach/Light.h>
#include <Peach/Map.h>
#include <math.h>

mLight* mLightCreate(mContext* ctx, const char* id) {
    if (ctx == NULL || ctx->lights == NULL || id == NULL || mMapContains(ctx->lights, id)) return NULL;
    mLight light = {.color = M_COLOR_WHITE, .radius = 64.0f, .intensity = 1.0f, .enabled = M_TRUE};
    if (!mMapSet(ctx->lights, id, mLight, light)) return NULL;
    return mMapGet(ctx->lights, id);
}

mLight* mLightGet(mContext* ctx, const char* id) {
    if (ctx == NULL || mMapValueSize(ctx->lights, id) != sizeof(mLight)) return NULL;
    return mMapGet(ctx->lights, id);
}

void mLightDestroy(mContext* ctx, const char* id) {
    if (ctx != NULL) mMapRemove(ctx->lights, id);
}

void mLightSetAmbient(mContext* ctx, mColor color) {
    if (ctx == NULL || ctx->renderer == NULL || !isfinite(color.r) || !isfinite(color.g) ||
        !isfinite(color.b) || color.r < 0 || color.g < 0 || color.b < 0) return;
    ctx->renderer->ambient = color;
}

void mLightMake(mLight *light, int x, int y, int radius, mColor color, float intensity) {
    light->x = x;
    light->y = y;
    light->radius = radius;
    light->color = color;
    light->intensity = intensity;
}

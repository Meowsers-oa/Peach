#ifndef PEACH_LIGHT_H
#define PEACH_LIGHT_H

#include <Peach/Common.h>

// Map-owned lights use world pixels, like sprites. Set properties before mUpdate.
mLight* mLightCreate(mContext* ctx, const char* id);
mLight* mLightGet(mContext* ctx, const char* id);
void mLightDestroy(mContext* ctx, const char* id);
// White preserves unlit rendering; darker colors make point lights visible.
void mLightSetAmbient(mContext* ctx, mColor color);

void mLightMake(mLight* light, int x, int y, int radius, mColor color, float intensity);

#endif //PEACH_LIGHT_H

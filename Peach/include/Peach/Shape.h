#ifndef PEACH_SHAPE_H
#define PEACH_SHAPE_H

#include <Peach/Common.h>

// Initialize to {0}. Owns copied geometry; borrows its texture. Do not shallow-copy.
int mShapeCreate(mShape* shape, const mVertex* vertices, unsigned int vertexCount,
                 const unsigned int* indices, unsigned int indexCount, const mTexture* texture);
// Pixel size; transform.position and rotation origin are the top-left corner.
int mShapeQuadCreate(mShape* shape, float width, float height, mColor color, const mTexture* texture);
int mShapeTriangleCreate(mShape* shape, vec3s a, vec3s b, vec3s c, mColor color);
void mShapeSetColor(mShape* shape, mColor color);
int mDrawShape(mContext* ctx, const mShape* shape);
void mShapeDestroy(mShape* shape);

#endif //PEACH_SHAPE_H

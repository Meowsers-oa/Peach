#ifndef PEACH_SHAPE_H
#define PEACH_SHAPE_H

#include <Peach/Common.h>

// Initialize shape to {0}. Geometry is copied and owned by the shape; texture is
// borrowed. Do not shallow-copy an initialized shape or replace its buffer pointers.
// Indices are local to vertices. NULL indices with indexCount = 0 selects triangles.
// Creation sets an identity transform; failure leaves the supplied shape unchanged.
int mShapeCreate(mShape* shape, const mVertex* vertices, unsigned int vertexCount,
                 const unsigned int* indices, unsigned int indexCount, const mTexture* texture);
// Quad from (0, 0) to (width, height), in window pixels, with full 0..1 UVs.
// transform.position is its top-left corner. Positive dimensions are required.
// Rotation and scale use that corner as the local origin.
int mShapeQuadCreate(mShape* shape, float width, float height, mColor color, const mTexture* texture);
// A solid triangle; vertices are local pixel offsets and use the supplied color.
int mShapeTriangleCreate(mShape* shape, vec3s a, vec3s b, vec3s c, mColor color);
void mShapeSetColor(mShape* shape, mColor color);
// Submit current geometry, transform and texture to the batch renderer.
int mDrawShape(mContext* ctx, const mShape* shape);
// Frees owned geometry only. Queued draws already have their own vertex copies.
// The borrowed texture must stay alive until those draws are flushed.
void mShapeDestroy(mShape* shape);

#endif //PEACH_SHAPE_H

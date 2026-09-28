#include <Peach/Shape.h>
#include <Peach/Renderer.h>
#include <Peach/Transform.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int mShapeCreate(mShape* shape, const mVertex* vertices, unsigned int vertexCount,
                 const unsigned int* indices, unsigned int indexCount, const mTexture* texture) {
    if (shape == NULL || shape->vertices != NULL || shape->indices != NULL ||
        shape->vertexCount != 0 || shape->indexCount != 0) return M_FAILURE;
    if (vertices == NULL || vertexCount == 0) return M_FAILURE;
    if ((indices == NULL) != (indexCount == 0)) return M_FAILURE;
    if (indexCount != 0) {
        if (indexCount % 3 != 0) return M_FAILURE;
        for (unsigned int i = 0; i < indexCount; i++) {
            if (indices[i] >= vertexCount) return M_FAILURE;
        }
    } else if (vertexCount % 3 != 0) {
        return M_FAILURE;
    }
    size_t vertexBytes = (size_t)vertexCount * sizeof(mVertex);
    size_t indexBytes = (size_t)indexCount * sizeof(unsigned int);
    if (vertexBytes / sizeof(mVertex) != vertexCount ||
        indexBytes / sizeof(unsigned int) != indexCount) return M_FAILURE;

    mShape result = {.transform = mTransformCreate(), .texture = texture,
                     .vertexCount = vertexCount, .indexCount = indexCount};
    result.vertices = malloc(vertexBytes);
    if (result.vertices == NULL) return M_FAILURE;
    memcpy(result.vertices, vertices, vertexBytes);
    if (indexCount != 0) {
        result.indices = malloc(indexBytes);
        if (result.indices == NULL) {
            free(result.vertices);
            return M_FAILURE;
        }
        memcpy(result.indices, indices, indexBytes);
    }
    *shape = result;
    return M_SUCCESS;
}

int mShapeQuadCreate(mShape* shape, float width, float height, mColor color, const mTexture* texture) {
    if (!isfinite(width) || !isfinite(height) || width <= 0.0f || height <= 0.0f) return M_FAILURE;
    mVertex vertices[] = {
        {.x = 0, .y = 0, .u = 0, .v = 0, .color = color},
        {.x = width, .y = 0, .u = 1, .v = 0, .color = color},
        {.x = width, .y = height, .u = 1, .v = 1, .color = color},
        {.x = 0, .y = height, .u = 0, .v = 1, .color = color}
    };
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    return mShapeCreate(shape, vertices, 4, indices, 6, texture);
}

int mShapeTriangleCreate(mShape* shape, vec3s a, vec3s b, vec3s c, mColor color) {
    mVertex vertices[] = {
        {.x = a.x, .y = a.y, .z = a.z, .color = color},
        {.x = b.x, .y = b.y, .z = b.z, .color = color},
        {.x = c.x, .y = c.y, .z = c.z, .color = color}
    };
    return mShapeCreate(shape, vertices, 3, NULL, 0, NULL);
}

void mShapeSetColor(mShape* shape, mColor color) {
    if (shape == NULL || shape->vertices == NULL) return;
    for (unsigned int i = 0; i < shape->vertexCount; i++) shape->vertices[i].color = color;
}

int mDrawShape(mContext* ctx, const mShape* shape) {
    if (shape == NULL) return M_FAILURE;
    if (shape->indices != NULL || shape->indexCount != 0) {
        return mAddVerticesIndexed(ctx, shape->vertices, shape->vertexCount,
                                   shape->indices, shape->indexCount, &shape->transform, shape->texture);
    }
    return mAddVertices(ctx, shape->vertices, shape->vertexCount, &shape->transform, shape->texture);
}

void mShapeDestroy(mShape* shape) {
    if (shape == NULL) return;
    free(shape->vertices);
    free(shape->indices);
    *shape = (mShape){0};
}

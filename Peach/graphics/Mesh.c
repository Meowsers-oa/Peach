#include "Peach/graphics/Mesh.h"
#include "Peach/graphics/Renderer.h"
#include "Peach/graphics/Material.h"

#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mColor multiplyColor(mColor a, mColor b) {
    return (mColor){a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a};
}

int mMeshLoad(const char* filePath, mMesh* outMesh) {
    return mMeshLoadEx(filePath, outMesh, mMeshImportDefault);
}

int mMeshLoadEx(const char* filePath, mMesh* outMesh, unsigned int flags) {
    if (!filePath || !outMesh || (flags & ~mMeshImportFlipFacing)) return 0;

    const struct aiScene* scene = aiImportFile(filePath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices | aiProcess_SortByPType);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        fprintf(stderr, "Peach: failed to load '%s': %s\n", filePath, aiGetErrorString());
        if (scene) aiReleaseImport(scene);
        return 0;
    }

    mMesh result = {.colors = WHITE};
    size_t vertexCount = 0, indexCount = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const struct aiMesh* mesh = scene->mMeshes[i];
        if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)) continue;
        if (mesh->mNumVertices > UINT_MAX - vertexCount) goto fail;
        vertexCount += mesh->mNumVertices;
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            if (mesh->mFaces[f].mNumIndices != 3) continue;
            if (indexCount > UINT_MAX - 3) goto fail;
            indexCount += 3;
        }
    }
    if (!vertexCount || !indexCount ||
        vertexCount > SIZE_MAX / sizeof(mVertex) ||
        indexCount > SIZE_MAX / sizeof(unsigned int)) goto fail;

    if (!mMeshLoadMaterials(scene, filePath, &result)) goto fail;
    result.parts = calloc(scene->mNumMeshes, sizeof(mMeshPart));
    if (!result.parts) goto fail;
    result.vertices = calloc(vertexCount, sizeof(mVertex));
    result.indices = malloc(indexCount * sizeof(unsigned int));
    if (!result.vertices || !result.indices) goto fail;
    result.vertexCount = (unsigned int)vertexCount;
    result.indexCount = (unsigned int)indexCount;

    unsigned int vertexOffset = 0, indexOffset = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const struct aiMesh* mesh = scene->mMeshes[i];
        if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)) continue;
        mMeshPart* part = &result.parts[result.partCount++];
        *part = (mMeshPart){.firstVertex = vertexOffset, .vertexCount = mesh->mNumVertices,
                            .firstIndex = indexOffset, .materialIndex = mesh->mMaterialIndex};
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            mVertex* dest = &result.vertices[vertexOffset + v];
            dest->position[0] = mesh->mVertices[v].x;
            dest->position[1] = mesh->mVertices[v].y;
            dest->position[2] = mesh->mVertices[v].z;
            if (mesh->mNormals) {
                dest->normal[0] = mesh->mNormals[v].x;
                dest->normal[1] = mesh->mNormals[v].y;
                dest->normal[2] = mesh->mNormals[v].z;
                glm_vec3_normalize(dest->normal);
            }
            if (mesh->mTextureCoords[0]) {
                dest->texCoord[0] = mesh->mTextureCoords[0][v].x;
                dest->texCoord[1] = mesh->mTextureCoords[0][v].y;
            }
            dest->color = WHITE;
            if (mesh->mColors[0]) {
                struct aiColor4D color = mesh->mColors[0][v];
                dest->color = (mColor){color.r, color.g, color.b, color.a};
            }
        }
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            const struct aiFace* face = &mesh->mFaces[f];
            if (face->mNumIndices != 3) continue;
            for (unsigned int j = 0; j < 3; ++j) {
                if (face->mIndices[j] >= mesh->mNumVertices) goto fail;
                result.indices[indexOffset++] = vertexOffset + face->mIndices[j];
            }
        }
        part->indexCount = indexOffset - part->firstIndex;
        vertexOffset += mesh->mNumVertices;
    }
    aiReleaseImport(scene);
    if (flags & mMeshImportFlipFacing) {
        for (unsigned int i = 0; i < result.vertexCount; ++i)
            glm_vec3_negate(result.vertices[i].normal);
        for (unsigned int i = 0; i < result.indexCount; i += 3) {
            unsigned int swap = result.indices[i + 1];
            result.indices[i + 1] = result.indices[i + 2];
            result.indices[i + 2] = swap;
        }
    }
    mMeshFree(outMesh);
    *outMesh = result;
    return 1;

fail:
    fprintf(stderr, "Peach: mesh '%s' is empty, too large, invalid, or could not be allocated.\n", filePath);
    aiReleaseImport(scene);
    mMeshFree(&result);
    return 0;
}

void mMeshFree(mMesh* mesh) {
    if (!mesh) return;
    free(mesh->vertices);
    free(mesh->indices);
    for (unsigned int i = 0; i < mesh->textureCount; ++i) mUnloadTexture(&mesh->textures[i]);
    free(mesh->textures);
    free(mesh->materials);
    free(mesh->parts);
    memset(mesh, 0, sizeof(*mesh));
}

void mMeshSubmit(mContext* ctx, const mMesh* mesh, const mMaterial* override,
                 const mTransform* transform) {
    if (!ctx || !mesh || !transform || !mesh->vertices || !mesh->indices ||
        !mesh->vertexCount || !mesh->indexCount || mesh->indexCount % 3 != 0) return;
    for (int axis = 0; axis < 3; ++axis) {
        if (!isfinite(transform->position[axis]) || !isfinite(transform->rotation[axis]) ||
            !isfinite(transform->scale[axis]) || transform->scale[axis] == 0.0f) return;
    }
    if ((size_t)mesh->vertexCount > SIZE_MAX / sizeof(mVertex) ||
        (size_t)mesh->indexCount > SIZE_MAX / sizeof(unsigned int)) return;
    mMeshPart whole = {.vertexCount = mesh->vertexCount, .indexCount = mesh->indexCount};
    unsigned int count = mesh->partCount ? mesh->partCount : 1;
    if (mesh->partCount && !mesh->parts) return;
    // Validate all parts before submitting anything; imported indices are global.
    for (unsigned int p = 0; p < count; ++p) {
        const mMeshPart* part = mesh->partCount ? &mesh->parts[p] : &whole;
        if (part->firstVertex > mesh->vertexCount || part->vertexCount > mesh->vertexCount - part->firstVertex ||
            part->firstIndex > mesh->indexCount || part->indexCount > mesh->indexCount - part->firstIndex ||
            part->indexCount % 3 != 0) return;
        for (unsigned int i = 0; i < part->indexCount; ++i) {
            unsigned int index = mesh->indices[part->firstIndex + i];
            if (index < part->firstVertex || index - part->firstVertex >= part->vertexCount) return;
        }
    }
    mVertex* vertices = malloc((size_t)mesh->vertexCount * sizeof(mVertex));
    unsigned int* indices = malloc((size_t)mesh->indexCount * sizeof(unsigned int));
    if (!vertices || !indices) { free(vertices); free(indices); return; }
    mat4 rotation = GLM_MAT4_IDENTITY_INIT;
    glm_rotate_z(rotation, transform->rotation[2], rotation);
    glm_rotate_y(rotation, transform->rotation[1], rotation);
    glm_rotate_x(rotation, transform->rotation[0], rotation);
    int mirrored = (transform->scale[0] < 0) ^ (transform->scale[1] < 0) ^ (transform->scale[2] < 0);
    float previousSpecular = ctx->renderer.materialSpecular;
    float previousShininess = ctx->renderer.materialShininess;
    for (unsigned int p = 0; p < count; ++p) {
        const mMeshPart* part = mesh->partCount ? &mesh->parts[p] : &whole;
        if (!part->indexCount) continue;
        const mMaterial* material = override;
        if (!material && mesh->materials && part->materialIndex < mesh->materialCount)
            material = &mesh->materials[part->materialIndex];
        mColor tint = material ? multiplyColor(mesh->colors, material->color) : mesh->colors;
        for (unsigned int i = 0; i < part->vertexCount; ++i) {
            mVertex* v = &vertices[i];
            *v = mesh->vertices[part->firstVertex + i];
            for (int axis = 0; axis < 3; ++axis) {
                v->position[axis] *= transform->scale[axis];
                v->normal[axis] /= transform->scale[axis];
            }
            glm_mat4_mulv3(rotation, v->position, 0, v->position);
            glm_vec3_add(v->position, (float*)transform->position, v->position);
            glm_mat4_mulv3(rotation, v->normal, 0, v->normal);
            glm_vec3_normalize(v->normal);
            v->color = multiplyColor(v->color, tint);
            if (material) v->texId = (float)material->texture.id;
        }
        for (unsigned int i = 0; i < part->indexCount; i += 3) {
            indices[i] = mesh->indices[part->firstIndex + i] - part->firstVertex;
            indices[i + 1] = mesh->indices[part->firstIndex + i + (mirrored ? 2 : 1)] - part->firstVertex;
            indices[i + 2] = mesh->indices[part->firstIndex + i + (mirrored ? 1 : 2)] - part->firstVertex;
        }
        float specular = material ? material->specular : previousSpecular;
        float shininess = material ? material->shininess : previousShininess;
        specular = isfinite(specular) ? fmaxf(specular, 0) : 0;
        shininess = isfinite(shininess) ? fmaxf(shininess, 1) : 1;
        if (ctx->renderer.materialSpecular != specular || ctx->renderer.materialShininess != shininess)
            mRendererFlush(ctx);
        ctx->renderer.materialSpecular = specular;
        ctx->renderer.materialShininess = shininess;
        mAddVerticesIndexed(ctx, vertices, part->vertexCount, indices, part->indexCount);
    }
    if (ctx->renderer.materialSpecular != previousSpecular || ctx->renderer.materialShininess != previousShininess)
        mRendererFlush(ctx);
    ctx->renderer.materialSpecular = previousSpecular;
    ctx->renderer.materialShininess = previousShininess;
    free(indices);
    free(vertices);
}

void mDrawMesh(mContext* ctx, const mMesh* mesh, const mTransform* transform) {
    mMeshSubmit(ctx, mesh, NULL, transform);
}

void mAddMesh(mContext* ctx, const mMesh* mesh, vec3 pos, vec3 rotation, vec3 scale) {
    mTransform transform;
    glm_vec3_copy(pos, transform.position);
    glm_vec3_copy(rotation, transform.rotation);
    glm_vec3_copy(scale, transform.scale);
    mDrawMesh(ctx, mesh, &transform);
}

#include "Peach/graphics/Mesh.h"
#include "Peach/graphics/Renderer.h"

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
    if (!filePath || !outMesh) return 0;

    const struct aiScene* scene = aiImportFile(filePath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
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

    result.vertices = calloc(vertexCount, sizeof(mVertex));
    result.indices = malloc(indexCount * sizeof(unsigned int));
    if (!result.vertices || !result.indices) goto fail;
    result.vertexCount = (unsigned int)vertexCount;
    result.indexCount = (unsigned int)indexCount;

    unsigned int vertexOffset = 0, indexOffset = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const struct aiMesh* mesh = scene->mMeshes[i];
        if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)) continue;
        mColor materialColor = WHITE;
        if (mesh->mMaterialIndex < scene->mNumMaterials) {
            struct aiColor4D color;
            if (aiGetMaterialColor(scene->mMaterials[mesh->mMaterialIndex],
                                   AI_MATKEY_COLOR_DIFFUSE, &color) == aiReturn_SUCCESS)
                materialColor = (mColor){color.r, color.g, color.b, color.a};
        }
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
            dest->color = materialColor;
            if (mesh->mColors[0]) {
                struct aiColor4D color = mesh->mColors[0][v];
                dest->color = multiplyColor(materialColor, (mColor){color.r, color.g, color.b, color.a});
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
        vertexOffset += mesh->mNumVertices;
    }
    aiReleaseImport(scene);
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
    free(mesh->textures);
    memset(mesh, 0, sizeof(*mesh));
}

void mAddMesh(mContext* ctx, const mMesh* mesh, vec3 pos, vec3 rotation, vec3 scale) {
    if (!ctx || !mesh || !mesh->vertices || !mesh->indices ||
        !mesh->vertexCount || !mesh->indexCount || mesh->indexCount % 3 != 0) return;
    for (int axis = 0; axis < 3; ++axis) {
        if (!isfinite(pos[axis]) || !isfinite(rotation[axis]) ||
            !isfinite(scale[axis]) || scale[axis] == 0.0f) return;
    }
    for (unsigned int i = 0; i < mesh->indexCount; ++i)
        if (mesh->indices[i] >= mesh->vertexCount) return;
    if ((size_t)mesh->vertexCount > SIZE_MAX / sizeof(mVertex) ||
        (size_t)mesh->indexCount > SIZE_MAX / sizeof(unsigned int)) return;

    mVertex* vertices = malloc((size_t)mesh->vertexCount * sizeof(mVertex));
    unsigned int* flippedIndices = NULL;
    int mirrored = (scale[0] < 0) ^ (scale[1] < 0) ^ (scale[2] < 0);
    if (mirrored) flippedIndices = malloc((size_t)mesh->indexCount * sizeof(unsigned int));
    if (!vertices || (mirrored && !flippedIndices)) {
        free(vertices);
        free(flippedIndices);
        fprintf(stderr, "Peach: unable to allocate transformed mesh.\n");
        return;
    }
    mat4 rotationMatrix = GLM_MAT4_IDENTITY_INIT;
    glm_rotate_z(rotationMatrix, rotation[2], rotationMatrix);
    glm_rotate_y(rotationMatrix, rotation[1], rotationMatrix);
    glm_rotate_x(rotationMatrix, rotation[0], rotationMatrix);
    for (unsigned int i = 0; i < mesh->vertexCount; ++i) {
        vertices[i] = mesh->vertices[i];
        for (int axis = 0; axis < 3; ++axis) {
            vertices[i].position[axis] *= scale[axis];
            // Inverse transpose for nonuniform scale, followed by rotation.
            vertices[i].normal[axis] /= scale[axis];
        }
        glm_mat4_mulv3(rotationMatrix, vertices[i].position, 0, vertices[i].position);
        glm_vec3_add(vertices[i].position, pos, vertices[i].position);
        glm_mat4_mulv3(rotationMatrix, vertices[i].normal, 0, vertices[i].normal);
        glm_vec3_normalize(vertices[i].normal);
        vertices[i].color = multiplyColor(vertices[i].color, mesh->colors);
    }
    if (mirrored) {
        for (unsigned int i = 0; i < mesh->indexCount; i += 3) {
            flippedIndices[i] = mesh->indices[i];
            flippedIndices[i + 1] = mesh->indices[i + 2];
            flippedIndices[i + 2] = mesh->indices[i + 1];
        }
    }
    mAddVerticesIndexed(ctx, vertices, mesh->vertexCount,
                        mirrored ? flippedIndices : mesh->indices, mesh->indexCount);
    free(flippedIndices);
    free(vertices);
}

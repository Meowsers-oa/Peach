//
// Created by Štěpán Toman on 23.09.2026.
//

#ifndef PEACH_MESH_H
#define PEACH_MESH_H

#include "Peach/core/Structs.h"

typedef enum {
    mMeshImportDefault = 0,
    // For inside-out assets: reverse triangle winding AND negate normals.
    mMeshImportFlipFacing = 1 << 0
} mMeshImportFlags;

// Initialize outMesh to {0}. Requires a current GL context for material textures.
// Successful loads replace the old mesh; failures leave it untouched.
int mMeshLoad(const char* filePath, mMesh* outMesh);
int mMeshLoadEx(const char* filePath, mMesh* outMesh, unsigned int flags);
// Releases imported GPU textures too; call before mEnd, after queued draws finish.
void mMeshFree(mMesh* mesh);

// Draw using the imported material assigned to each part.
void mDrawMesh(mContext* ctx, const mMesh* mesh, const mTransform* transform);

// Shared submission implementation; NULL material uses imported materials.
void mMeshSubmit(mContext* ctx, const mMesh* mesh, const mMaterial* material, const mTransform* transform);

void mAddMesh(mContext* ctx, const mMesh* mesh, vec3 pos, vec3 rotation, vec3 scale);

#endif //PEACH_MESH_H

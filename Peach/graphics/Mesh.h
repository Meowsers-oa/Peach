//
// Created by Štěpán Toman on 23.09.2026.
//

#ifndef PEACH_MESH_H
#define PEACH_MESH_H

#include "Peach/core/Structs.h"

int mMeshLoad(const char* filePath, mMesh* outMesh);
void mMeshFree(mMesh* mesh);

void mAddMesh(mContext* ctx, const mMesh* mesh, vec3 pos, vec3 rotation, vec3 scale);

#endif //PEACH_MESH_H

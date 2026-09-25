#ifndef PEACH_MATERIAL_H
#define PEACH_MATERIAL_H

#include "Peach/core/Structs.h"
struct aiScene;
// Internal import helper. Textures are owned by mesh, materials borrow them.
int mMeshLoadMaterials(const struct aiScene* scene, const char* modelPath, mMesh* mesh);

#endif

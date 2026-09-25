#include "Peach/graphics/Material.h"
#include "Peach/graphics/Texture.h"
#include <assimp/material.h>
#include <assimp/scene.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* copyString(const char* text) {
    size_t size = strlen(text) + 1;
    char* copy = malloc(size);
    if (copy) memcpy(copy, text, size);
    return copy;
}

static char* relativePath(const char* file, const char* name) {
    char* normalized = copyString(name);
    if (!normalized) return NULL;
    for (char* c = normalized; *c; ++c) if (*c == '\\') *c = '/';
    if (normalized[0] == '/' || (strlen(normalized) > 1 && normalized[1] == ':')) return normalized;
    const char* slash = strrchr(file, '/');
    size_t prefix = slash ? (size_t)(slash - file + 1) : 0;
    size_t length = strlen(normalized);
    char* path = malloc(prefix + length + 1);
    if (path) { memcpy(path, file, prefix); memcpy(path + prefix, normalized, length + 1); }
    free(normalized);
    return path;
}

static char* directive(char* line, const char* keyword) {
    while (isspace((unsigned char)*line)) ++line;
    size_t n = strlen(keyword);
    if (strncmp(line, keyword, n) || !isspace((unsigned char)line[n])) return NULL;
    line += n;
    while (isspace((unsigned char)*line)) ++line;
    char* end = line + strlen(line);
    while (end > line && isspace((unsigned char)end[-1])) --end;
    *end = 0;
    if (*line == '"' && end > line + 1 && end[-1] == '"') { ++line; end[-1] = 0; }
    return line;
}

// Assimp exposes texture names but not the MTL file that defined each material.
// Find that origin so an MTL in a subdirectory can reference its own textures.
static char* materialOrigin(const char* model, const char* materialName) {
    const char* extension = strrchr(model, '.');
    if (!extension || (strcmp(extension, ".obj") && strcmp(extension, ".OBJ"))) return copyString(model);
    FILE* obj = fopen(model, "r");
    if (!obj) return copyString(model);
    char line[8192];
    char* origin = NULL;
    while (!origin && fgets(line, sizeof(line), obj)) {
        char* library = directive(line, "mtllib");
        if (!library) continue;
        char* path = relativePath(model, library);
        FILE* mtl = path ? fopen(path, "r") : NULL;
        if (mtl) {
            char entry[8192];
            while (fgets(entry, sizeof(entry), mtl)) {
                char* name = directive(entry, "newmtl");
                if (name && !strcmp(name, materialName)) { origin = path; path = NULL; break; }
            }
            fclose(mtl);
        }
        free(path);
    }
    fclose(obj);
    return origin ? origin : copyString(model);
}

static GLint wrapMode(enum aiTextureMapMode mode) {
    return mode == aiTextureMapMode_Clamp || mode == aiTextureMapMode_Decal ? GL_CLAMP_TO_EDGE :
           mode == aiTextureMapMode_Mirror ? GL_MIRRORED_REPEAT : GL_REPEAT;
}

int mMeshLoadMaterials(const struct aiScene* scene, const char* modelPath, mMesh* mesh) {
    if (!scene->mNumMaterials) return 1;
    mesh->materials = calloc(scene->mNumMaterials, sizeof(mMaterial));
    mesh->textures = calloc(scene->mNumMaterials, sizeof(mTexture));
    char** paths = calloc(scene->mNumMaterials, sizeof(char*));
    GLint* wraps = calloc((size_t)scene->mNumMaterials * 2, sizeof(GLint));
    int ok = mesh->materials && mesh->textures && paths && wraps;
    if (!ok) goto done;
    mesh->materialCount = scene->mNumMaterials;
    for (unsigned int i = 0; i < mesh->materialCount; ++i) {
        const struct aiMaterial* source = scene->mMaterials[i];
        mMaterial* material = &mesh->materials[i];
        *material = (mMaterial){.color = WHITE, .shininess = 1};
        struct aiColor4D color;
        if (aiGetMaterialColor(source, AI_MATKEY_COLOR_DIFFUSE, &color) == aiReturn_SUCCESS)
            material->color = (mColor){color.r, color.g, color.b, color.a};
        float opacity;
        if (aiGetMaterialFloatArray(source, AI_MATKEY_OPACITY, &opacity, NULL) == aiReturn_SUCCESS)
            material->color.a = isfinite(opacity) ? fminf(fmaxf(opacity, 0), 1) : 1;
        if (aiGetMaterialColor(source, AI_MATKEY_COLOR_SPECULAR, &color) == aiReturn_SUCCESS)
            material->specular = fmaxf(0, (color.r + color.g + color.b) / 3);
        float shininess;
        if (aiGetMaterialFloatArray(source, AI_MATKEY_SHININESS, &shininess, NULL) == aiReturn_SUCCESS)
            material->shininess = isfinite(shininess) ? fmaxf(shininess, 1) : 1;
        struct aiString textureName, materialName = {0};
        enum aiTextureMapMode modes[2] = {aiTextureMapMode_Wrap, aiTextureMapMode_Wrap};
        if (aiGetMaterialTexture(source, aiTextureType_DIFFUSE, 0, &textureName,
                                 NULL, NULL, NULL, NULL, modes, NULL) != aiReturn_SUCCESS) continue;
        aiGetMaterialString(source, AI_MATKEY_NAME, &materialName);
        if (!glfwGetCurrentContext()) {
            fprintf(stderr, "Peach: loading material textures requires a current OpenGL context.\n");
            ok = 0; break;
        }
        char* origin = materialOrigin(modelPath, materialName.data);
        char* path = origin ? relativePath(origin, textureName.data) : NULL;
        free(origin);
        if (!path) { ok = 0; break; }
        // Some exporters use OBJ-relative paths even when the MTL lives elsewhere.
        FILE* image = fopen(path, "rb");
        if (image) fclose(image);
        else {
            char* fallback = relativePath(modelPath, textureName.data);
            FILE* candidate = fallback ? fopen(fallback, "rb") : NULL;
            if (candidate) { fclose(candidate); free(path); path = fallback; }
            else free(fallback);
        }
        char* canonical = realpath(path, NULL);
        if (canonical) { free(path); path = canonical; }
        GLint wrapS = wrapMode(modes[0]), wrapT = wrapMode(modes[1]);
        unsigned int texture = 0;
        while (texture < mesh->textureCount &&
               (strcmp(paths[texture], path) || wraps[texture*2] != wrapS || wraps[texture*2+1] != wrapT)) ++texture;
        if (texture == mesh->textureCount) {
            mTexture loaded = mLoadTexture(path);
            if (!loaded.id) {
                fprintf(stderr, "Peach: material '%s' uses its color because texture '%s' could not be loaded.\n",
                        materialName.data, path);
                free(path); continue;
            }
            mSetTextureWrap(&loaded, wrapS, wrapT);
            mesh->textures[texture] = loaded;
            paths[texture] = path;
            wraps[texture*2] = wrapS; wraps[texture*2+1] = wrapT;
            ++mesh->textureCount;
        } else free(path);
        material->texture = mesh->textures[texture];
    }
done:
    if (paths) for (unsigned int i = 0; i < mesh->textureCount; ++i) free(paths[i]);
    free(paths); free(wraps);
    return ok;
}

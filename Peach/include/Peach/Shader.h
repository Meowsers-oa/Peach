#ifndef PEACH_SHADER_H
#define PEACH_SHADER_H

#include <Peach/Common.h>

// Initialize to {0}; requires a current OpenGL context.
int mShaderLoad(mShader* shader, const char* vertexPath, const char* fragmentPath);
void mShaderUse(const mShader* shader);
void mShaderDestroy(mShader* shader);

#endif //PEACH_SHADER_H

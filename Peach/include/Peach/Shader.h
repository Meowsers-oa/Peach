#ifndef PEACH_SHADER_H
#define PEACH_SHADER_H

#include <Peach/Common.h>

// Initialize shader to {0}. Requires a current OpenGL context.
// Reads, compiles and links vertex/fragment GLSL files. Paths are used as supplied.
// Returns M_FAILURE with diagnostics on stderr; failed loads leave shader unchanged.
// Shader files can be released or edited after loading; the program lives on the GPU.
int mShaderLoad(mShader* shader, const char* vertexPath, const char* fragmentPath);
// Bind a linked program, or pass NULL to unbind. Flush queued draws before switching.
void mShaderUse(const mShader* shader);
// Flush queued users before deletion. Unbinds the program if currently active.
void mShaderDestroy(mShader* shader);

#endif //PEACH_SHADER_H

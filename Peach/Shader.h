//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_SHADER_H
#define PEACH_SHADER_H

#include "Structs.h"
#include <cglm/cglm.h>

void mLoadShaders(mContext* ctx, const char* vertexPath, const char* fragmentPath);
void mUseProgram(mContext* ctx);
void mSetUniformInt(mContext* ctx, const char* name, int val);
void mSetUniformIntArray(mContext* ctx, const char* name, const int* values, int count);
void mSetUniformFloat(mContext* ctx, const char* name, float val);
void mSetUniformFloat2(mContext* ctx, const char* name, vec2 vec);
void mSetUniformFloat3(mContext* ctx, const char* name, vec3 vec);
void mSetUniformFloat4(mContext* ctx, const char* name, vec4 vec);
void mSetUniformMat4(mContext* ctx, const char* name, mat4 mat);

#endif //PEACH_SHADER_H

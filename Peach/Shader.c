//
// Created by Štěpán Toman on 20.09.2026.
//

#include "Shader.h"
#include "Utils.h"

void mLoadShaders(mContext *ctx, const char *vertexPath, const char *fragmentPath) {
    const char* vertexSource = mReadFromFile(vertexPath);
    const char* fragmentSource = mReadFromFile(fragmentPath);

    if (!vertexSource || !fragmentSource) {
        printf("ERROR: Could not load shader sources!\n");
        return;
    }

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR: Failed to compile vertex shader!: %s", infoLog);
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR: Failed to compile fragment shader!: %s", infoLog);
    }

    free((void*)vertexSource);
    free((void*)fragmentSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("ERROR: Failed to create/link shader program!: %s", infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    ctx->renderer.shaderProgram = program;

}

void mUseProgram(mContext *ctx) {
    glUseProgram(ctx->renderer.shaderProgram);
}

void mSetUniformInt(mContext *ctx, const char *name, int val) {
    glUniform1i(glGetUniformLocation(ctx->renderer.shaderProgram, name), val);
}

void mSetUniformFloat(mContext *ctx, const char *name, float val) {
    glUniform1f(glGetUniformLocation(ctx->renderer.shaderProgram, name), val);
}

void mSetUniformFloat2(mContext *ctx, const char *name, vec2 vec) {
    glUniform2f(glGetUniformLocation(ctx->renderer.shaderProgram, name), vec[0], vec[1]);
}

void mSetUniformFloat3(mContext *ctx, const char *name, vec3 vec) {
    glUniform3f(glGetUniformLocation(ctx->renderer.shaderProgram, name), vec[0], vec[1], vec[2]);
}

void mSetUniformFloat4(mContext *ctx, const char *name, vec4 vec) {
    glUniform4f(glGetUniformLocation(ctx->renderer.shaderProgram, name), vec[0], vec[1], vec[2], vec[3]);
}

void mSetUniformMat4(mContext *ctx, const char *name, mat4 mat) {
    glUniformMatrix4fv(glGetUniformLocation(ctx->renderer.shaderProgram, name), 1, GL_FALSE, (const float*)mat);
}

void mSetUniformIntArray(mContext *ctx, const char *name, const int *values, int count) {
    glUniform1iv(glGetUniformLocation(ctx->renderer.shaderProgram, name), count, values);
}

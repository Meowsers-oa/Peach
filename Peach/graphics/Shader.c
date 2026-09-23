//
// Created by Štěpán Toman on 20.09.2026.
//

#include "Peach/graphics/Shader.h"
#include "Peach/graphics/PostProcess.h"
#include "Peach/core/Utils.h"
#include <string.h>

#include "Peach/graphics/Renderer.h"

// CMake supplies the build's resource directory. Manual builds use source assets.
#ifndef PEACH_SHADER_DIR
#define PEACH_SHADER_DIR "assets/shaders"
#endif

static unsigned int compileShader(GLenum type, const char* path) {
    const char* source = mReadFromFile(path);
    if (!source) return 0;

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    free((void*)source);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Peach: failed to compile %s:\n%s\n", path, log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

unsigned int mCreateShaderProgram(const char* vertexPath, const char* fragmentPath) {
    if (!vertexPath || !fragmentPath) return 0;
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexPath);
    if (!vertex) return 0;
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentPath);
    if (!fragment) {
        glDeleteShader(vertex);
        return 0;
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[2048];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Peach: failed to link %s + %s:\n%s\n", vertexPath, fragmentPath, log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

unsigned int mCreateResourceShaderProgram(const char* vertexName, const char* fragmentName) {
    if (!vertexName || !fragmentName) return 0;
    size_t baseLength = strlen(PEACH_SHADER_DIR);
    char* vertexPath = malloc(baseLength + strlen(vertexName) + 2);
    char* fragmentPath = malloc(baseLength + strlen(fragmentName) + 2);
    if (!vertexPath || !fragmentPath) {
        free(vertexPath);
        free(fragmentPath);
        fprintf(stderr, "Peach: unable to allocate shader resource paths.\n");
        return 0;
    }
    sprintf(vertexPath, "%s/%s", PEACH_SHADER_DIR, vertexName);
    sprintf(fragmentPath, "%s/%s", PEACH_SHADER_DIR, fragmentName);
    unsigned int program = mCreateShaderProgram(vertexPath, fragmentPath);
    free(vertexPath);
    free(fragmentPath);
    return program;
}

unsigned int mCreatePostProcessShader(const char* fragmentPath) {
    return mCreateShaderProgram(PEACH_SHADER_DIR "/fullscreen.vert", fragmentPath);
}

static void useLoadedProgram(mContext* ctx, unsigned int program) {
    if (!program) return; // Preserve the working shader when a reload fails.
    if (ctx->renderer.isBatching) mRendererEnd(ctx);
    if (ctx->renderer.shaderProgram) glDeleteProgram(ctx->renderer.shaderProgram);
    ctx->renderer.shaderProgram = program;
}

void mLoadShaders(mContext* ctx, const char* vertexPath, const char* fragmentPath) {
    if (!ctx) return;
    useLoadedProgram(ctx, mCreateShaderProgram(vertexPath, fragmentPath));
}

void mLoadDefaultShaders(mContext* ctx) {
    if (!ctx) return;
    useLoadedProgram(ctx, mCreateResourceShaderProgram("scene.vert", "scene.frag"));
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

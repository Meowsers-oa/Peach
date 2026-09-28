#include <Peach/Shader.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static char* readShaderFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open shader: %s\n", path);
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek shader: %s\n", path);
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size <= 0 || size > INT_MAX || fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Invalid shader file size: %s\n", path);
        fclose(file);
        return NULL;
    }
    char* source = malloc((size_t)size + 1);
    if (source == NULL) {
        fprintf(stderr, "Failed to allocate shader source: %s\n", path);
        fclose(file);
        return NULL;
    }
    size_t readSize = fread(source, 1, (size_t)size, file);
    int failed = ferror(file);
    fclose(file);
    if (failed || readSize != (size_t)size) {
        fprintf(stderr, "Failed to read shader: %s\n", path);
        free(source);
        return NULL;
    }
    source[size] = '\0';
    return source;
}

static unsigned int compileShader(unsigned int type, const char* path) {
    char* source = readShaderFile(path);
    if (source == NULL) return 0;
    unsigned int handle = glCreateShader(type);
    if (handle == 0) {
        fprintf(stderr, "Failed to create shader: %s\n", path);
        free(source);
        return 0;
    }
    const char* text = source;
    glShaderSource(handle, 1, &text, NULL);
    glCompileShader(handle);
    free(source);

    int success;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[4096];
        glGetShaderInfoLog(handle, sizeof(log), NULL, log);
        fprintf(stderr, "Failed to compile shader %s:\n%s\n", path, log);
        glDeleteShader(handle);
        return 0;
    }
    return handle;
}

int mShaderLoad(mShader* shader, const char* vertexPath, const char* fragmentPath) {
    if (shader == NULL || shader->handle != 0 || vertexPath == NULL || fragmentPath == NULL) return M_FAILURE;
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexPath);
    if (vertex == 0) return M_FAILURE;
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentPath);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return M_FAILURE;
    }

    unsigned int program = glCreateProgram();
    if (program == 0) {
        fprintf(stderr, "Failed to create shader program: %s, %s\n", vertexPath, fragmentPath);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return M_FAILURE;
    }
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDetachShader(program, vertex);
    glDetachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[4096];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Failed to link shader program (%s, %s):\n%s\n", vertexPath, fragmentPath, log);
        glDeleteProgram(program);
        return M_FAILURE;
    }
    shader->handle = program;
    return M_SUCCESS;
}

void mShaderUse(const mShader* shader) {
    glUseProgram(shader != NULL ? shader->handle : 0);
}

void mShaderDestroy(mShader* shader) {
    if (shader == NULL || shader->handle == 0) return;
    int current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    if ((unsigned int)current == shader->handle) glUseProgram(0);
    glDeleteProgram(shader->handle);
    shader->handle = 0;
}

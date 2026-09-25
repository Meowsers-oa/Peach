//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_UTILS_H
#define PEACH_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "Peach/core/Structs.h"


static inline const char* mReadFromFile(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        printf("ERROR: Failed to open file: %s\n", filepath);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);

    // Allocate memory for the content plus a null-terminator
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        printf("ERROR: Failed to allocate memory for file: %s\n", filepath);
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead != (size_t)length) {
        printf("ERROR: Failed to read entire file: %s\n", filepath);
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Null-terminate the string
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

static inline void mTimeStart(mTime* time) {
    time->startTime = (float)glfwGetTime();
    time->timeSinceStart = 0.0f;
    time->currentTime = time->startTime;
    time->deltaTime = 0.0f;
}

static inline void mTimeUpdate(mTime* time) {
    float previousTime = time->currentTime;
    time->currentTime = (float)glfwGetTime();
    time->timeSinceStart = time->currentTime - time->startTime;
    time->deltaTime = time->currentTime - previousTime;
}

static inline float mGetDeltaTime(mContext* ctx) {
    return ctx->time.deltaTime;
}

static inline float mGetFPS(mContext* ctx) {
    return ctx && ctx->time.deltaTime > 0.0f ? 1.0f / ctx->time.deltaTime : 0.0f;
}

static inline mObject mMakeObject(mMesh* mesh, mTransform* transform) {
    mObject obj = {
        .mesh = mesh,
        .transform = transform,
    };

    return obj;
}


#endif //PEACH_UTILS_H

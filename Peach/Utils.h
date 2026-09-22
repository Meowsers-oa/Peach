//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_UTILS_H
#define PEACH_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static float lastTime;

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
    time->startTime = (float)clock() / CLOCKS_PER_SEC;
    time->timeSinceStart = 0.0f;
    time->currentTime = time->startTime;
    time->deltaTime = 0.0f;
    lastTime = time->startTime;
}

static inline void mTimeUpdate(mTime* time) {
    time->currentTime = (float)clock() / CLOCKS_PER_SEC;
    time->timeSinceStart = time->currentTime - time->startTime;
    time->deltaTime = time->currentTime - lastTime;
    lastTime = time->currentTime;
}

#endif //PEACH_UTILS_H

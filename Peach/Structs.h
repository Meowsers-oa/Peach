//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_STRUCTS_H
#define PEACH_STRUCTS_H

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

typedef struct {
    float r, g, b, a;
} Color;

#define WHITE       (Color){ 1.0f, 1.0f, 1.0f, 1.0f }
#define BLACK       (Color){ 0.0f, 0.0f, 0.0f, 1.0f }
#define RED         (Color){ 1.0f, 0.0f, 0.0f, 1.0f }
#define GREEN       (Color){ 0.0f, 1.0f, 0.0f, 1.0f }
#define BLUE        (Color){ 0.0f, 0.0f, 1.0f, 1.0f }
#define YELLOW      (Color){ 1.0f, 1.0f, 0.0f, 1.0f }
#define CYAN        (Color){ 0.0f, 1.0f, 1.0f, 1.0f }
#define MAGENTA     (Color){ 1.0f, 0.0f, 1.0f, 1.0f }

#define ORANGE      (Color){ 1.0f, 0.5f, 0.0f, 1.0f }
#define PURPLE      (Color){ 0.5f, 0.0f, 0.5f, 1.0f }
#define PINK        (Color){ 1.0f, 0.75f, 0.8f, 1.0f }
#define BROWN       (Color){ 0.6f, 0.4f, 0.2f, 1.0f }
#define LIME        (Color){ 0.75f, 1.0f, 0.0f, 1.0f }
#define TEAL        (Color){ 0.0f, 0.5f, 0.5f, 1.0f }
#define NAVY        (Color){ 0.1765f, 0.2353f, 0.3333f, 1.0f }
#define MAROON      (Color){ 0.5f, 0.0f, 0.0f, 1.0f }
#define OLIVE       (Color){ 0.5f, 0.5f, 0.0f, 1.0f }
#define GOLD        (Color){ 1.0f, 0.84f, 0.0f, 1.0f }
#define CORAL       (Color){ 1.0f, 0.5f, 0.31f, 1.0f }
#define INDIGO      (Color){ 0.29f, 0.0f, 0.51f, 1.0f }

#define GRAY        (Color){ 0.5f, 0.5f, 0.5f, 1.0f }
#define DARK_GRAY   (Color){ 0.25f, 0.25f, 0.25f, 1.0f }
#define LIGHT_GRAY  (Color){ 0.75f, 0.75f, 0.75f, 1.0f }
#define TRANSPARENT (Color){ 0.0f, 0.0f, 0.0f, 0.0f }

typedef struct {
    float deltaTime;
    float startTime;
    float timeSinceStart;
    float currentTime;
} mTime;

typedef struct {
    vec3 position;
    Color color;
    union {
        vec3 normals;
        vec3 normal;
    };
    vec2 texCoord;
    float texId;
} mVertex;

typedef mVertex Vertex;

typedef struct {
    unsigned int id;
    int width;
    int height;
    int channels;
} mTexture;

typedef mTexture Texture;

typedef enum {
    CAMERA_PERSPECTIVE = 0,
    CAMERA_ORTHOGRAPHIC = 1
} mCameraProjection;

typedef struct {
    vec3 position;
    vec3 target;
    vec3 up;
    float fov;
    float nearPlane;
    float farPlane;
    float aspect;
    mCameraProjection projection;
} mCamera3D;

typedef mCamera3D mCamera;
typedef mCamera3D Camera3D;
typedef mCamera3D Camera;

typedef struct {
    unsigned int drawCalls;
    unsigned int quadCount;
    unsigned int vertexCount;
    unsigned int indexCount;
} mRendererStats;

#define MAX_BATCH_QUADS 10000
#define MAX_BATCH_VERTICES (MAX_BATCH_QUADS * 4)
#define MAX_BATCH_INDICES (MAX_BATCH_QUADS * 6)
#define MAX_TEXTURE_SLOTS 16

typedef struct {
    GLFWwindow* handle;
    int width;
    int height;
    const char* title;
    int running;
    Color clearColor;
} mWindow;

typedef struct {
    unsigned int shaderProgram;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int whiteTexture;
    mVertex* vertexBuffer;
    mVertex* vertexBufferPtr;
    unsigned int* indexBuffer;
    unsigned int* indexBufferPtr;
    unsigned int indexCount;
    unsigned int vertexCount;
    unsigned int textureSlots[MAX_TEXTURE_SLOTS];
    unsigned int textureSlotIndex;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mRendererStats stats;
    int isBatching;
} mRenderer;

typedef struct {
    mWindow window;
    mRenderer renderer;
    mTime time;
} mContext;

#endif //PEACH_STRUCTS_H
//
// Created by Štěpán Toman on 27.09.2026.
//

#ifndef PEACH_COMMON_H
#define PEACH_COMMON_H

#include <glad/glad.h>
#include <cglm/types-struct.h>
#include <stdint.h>
#include <stddef.h>

#include <GLFW/glfw3.h>

#define M_SUCCESS 0
#define M_FAILURE (-1)
#define M_BOOL unsigned int
#define M_TRUE 1
#define M_FALSE 0
#define uint unsigned int

#define M_RENDERER_MAX_VERTICES 16384
#define M_RENDERER_MAX_INDICES 24576
#define M_RENDERER_MAX_TEXTURES 16

typedef enum {
    M_RESOURCE_VALUE,
    M_RESOURCE_TEXTURE,
    M_RESOURCE_SHADER,
    M_RESOURCE_SHAPE
}mResourceType;

typedef struct mMap mMap;

typedef struct {
    const char* key;
    void* value;
    size_t valueSize;
    mResourceType resourceType;
}mMapEntry;

typedef struct {
    const mMap* mMap;
    size_t index;
}mMapIter;

typedef struct {
    char* key;
    void* value;
    size_t valueSize;
    mResourceType resourceType;
    M_BOOL is_occupied;
    M_BOOL is_tombstone;
}mMapSlot;

struct mMap {
    mMapSlot* entries;
    size_t capacity;
    size_t count;
};

typedef struct {
    float r;
    float g;
    float b;
    float a;
}mColor;

// Color components are normalized floats. RGB/RGBA accept values from 0 to 255.
#define M_COLOR(r, g, b, a) ((mColor){(r), (g), (b), (a)})
#define M_RGBA(r, g, b, a) M_COLOR((r) / 255.0f, (g) / 255.0f, (b) / 255.0f, (a) / 255.0f)
#define M_RGB(r, g, b) M_RGBA(r, g, b, 255)

#define M_COLOR_TRANSPARENT M_RGBA(0, 0, 0, 0)
#define M_COLOR_BLACK M_RGBA(0, 0, 0, 255)
#define M_COLOR_WHITE M_RGBA(255, 255, 255, 255)
#define M_COLOR_RED M_RGBA(255, 0, 0, 255)
#define M_COLOR_GREEN M_RGBA(0, 255, 0, 255)
#define M_COLOR_BLUE M_RGBA(0, 0, 255, 255)
#define M_COLOR_YELLOW M_RGBA(255, 255, 0, 255)
#define M_COLOR_CYAN M_RGBA(0, 255, 255, 255)
#define M_COLOR_MAGENTA M_RGBA(255, 0, 255, 255)
#define M_COLOR_GRAY M_RGBA(128, 128, 128, 255)
#define M_COLOR_LIGHT_GRAY M_RGBA(192, 192, 192, 255)
#define M_COLOR_DARK_GRAY M_RGBA(64, 64, 64, 255)
#define M_COLOR_ORANGE M_RGBA(255, 165, 0, 255)
#define M_COLOR_PINK M_RGBA(255, 192, 203, 255)
#define M_COLOR_PURPLE M_RGBA(128, 0, 128, 255)
#define M_COLOR_BROWN M_RGBA(165, 42, 42, 255)
#define M_COLOR_LIME M_RGBA(191, 255, 0, 255)
#define M_COLOR_NAVY M_RGBA(0, 0, 128, 255)
#define M_COLOR_TEAL M_RGBA(0, 128, 128, 255)
#define M_COLOR_OLIVE M_RGBA(128, 128, 0, 255)
#define M_COLOR_GOLD M_RGBA(255, 215, 0, 255)
#define M_COLOR_PEACH M_RGBA(255, 218, 185, 255)

static uint64_t prng_state = 88172645463325252ULL;

typedef struct {
    int width;
    int height;
    char* title;
}mWindowInfo;

typedef struct {
    int running;
    mColor bgColor;
    GLFWwindow* handle;
}mWindow;

typedef struct {
    int keys[GLFW_KEY_LAST + 1];
    int keysPressed[GLFW_KEY_LAST + 1];
    int keysReleased[GLFW_KEY_LAST + 1];
    int mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
    int mouseButtonsPressed[GLFW_MOUSE_BUTTON_LAST + 1];
    int mouseButtonsReleased[GLFW_MOUSE_BUTTON_LAST + 1];
    double mouseX;
    double mouseY;
    double scrollX;
    double scrollY;
}mInput;

typedef struct {
    double deltaTime;
    double elapsedTime;
    double frameTime;
    double fps;
    double averageFPS;
    uint64_t frameCount;

    uint64_t lastTick;
    uint64_t frequency;
    double fpsElapsed;
    uint64_t fpsFrames;
}mTime;

typedef struct {
    vec3s position;
    vec3s rotation;
    vec3s scale;
}mTransform;

typedef struct {
    float x;
    float y;
    float rotation;
    float zoom;
    float width;
    float height;
    float view[16];
    float projection[16];
    float viewProjection[16];
}mCamera;

typedef struct {
    unsigned int handle;
    int width;
    int height;
}mTexture;

typedef struct {
    unsigned int handle;
}mShader;

typedef struct {
    float x;
    float y;
    float z;
    float u;
    float v;
    mColor color;
}mVertex;

typedef struct {
    mTransform transform;
    mVertex* vertices;
    unsigned int vertexCount;
    unsigned int* indices;
    unsigned int indexCount;
    const mTexture* texture;
}mShape;

typedef struct mRenderer mRenderer;

typedef struct {
    float position[4];
    float uv[2];
    mColor color;
    float textureSlot;
}mBatchVertex;

struct mRenderer {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    mShader shader;
    int viewProjectionLocation;
    float viewProjection[16];
    int customCamera;
    mTexture whiteTexture;
    unsigned int textures[M_RENDERER_MAX_TEXTURES];
    unsigned int textureCount;
    unsigned int textureLimit;
    unsigned int vertexCount;
    unsigned int indexCount;
    mBatchVertex vertices[M_RENDERER_MAX_VERTICES];
    unsigned int indices[M_RENDERER_MAX_INDICES];
};

typedef struct {
    mTexture texture;
    int x;
    int y;
    int width;
    int height;
    float scale;
    uint64_t id;
}mSprite;

typedef struct {
    mWindow window;
    mInput input;
    mTime time;
    mRenderer* renderer;
    mMap* sprites;
    mMap* resourcePool;
}mContext;

static inline uint64_t rand_ui64() {
    uint64_t x = prng_state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    prng_state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

#endif //PEACH_COMMON_H

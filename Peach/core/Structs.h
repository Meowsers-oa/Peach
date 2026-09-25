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
} mColor;

#define WHITE       (mColor){ 1.0f, 1.0f, 1.0f, 1.0f }
#define BLACK       (mColor){ 0.0f, 0.0f, 0.0f, 1.0f }
#define RED         (mColor){ 1.0f, 0.0f, 0.0f, 1.0f }
#define GREEN       (mColor){ 0.0f, 1.0f, 0.0f, 1.0f }
#define BLUE        (mColor){ 0.0f, 0.0f, 1.0f, 1.0f }
#define YELLOW      (mColor){ 1.0f, 1.0f, 0.0f, 1.0f }
#define CYAN        (mColor){ 0.0f, 1.0f, 1.0f, 1.0f }
#define MAGENTA     (mColor){ 1.0f, 0.0f, 1.0f, 1.0f }

#define ORANGE      (mColor){ 1.0f, 0.5f, 0.0f, 1.0f }
#define PURPLE      (mColor){ 0.5f, 0.0f, 0.5f, 1.0f }
#define PINK        (mColor){ 1.0f, 0.75f, 0.8f, 1.0f }
#define BROWN       (mColor){ 0.6f, 0.4f, 0.2f, 1.0f }
#define LIME        (mColor){ 0.75f, 1.0f, 0.0f, 1.0f }
#define TEAL        (mColor){ 0.0f, 0.5f, 0.5f, 1.0f }
#define NAVY        (mColor){ 0.1765f, 0.2353f, 0.3333f, 1.0f }
#define MAROON      (mColor){ 0.5f, 0.0f, 0.0f, 1.0f }
#define OLIVE       (mColor){ 0.5f, 0.5f, 0.0f, 1.0f }
#define GOLD        (mColor){ 1.0f, 0.84f, 0.0f, 1.0f }
#define CORAL       (mColor){ 1.0f, 0.5f, 0.31f, 1.0f }
#define INDIGO      (mColor){ 0.29f, 0.0f, 0.51f, 1.0f }

#define GRAY        (mColor){ 0.5f, 0.5f, 0.5f, 1.0f }
#define DARK_GRAY   (mColor){ 0.25f, 0.25f, 0.25f, 1.0f }
#define LIGHT_GRAY  (mColor){ 0.75f, 0.75f, 0.75f, 1.0f }
#define TRANSPARENT (mColor){ 0.0f, 0.0f, 0.0f, 0.0f }

typedef enum {
    mPointLight = 0,
    mSpotLight = 1
} mLightType;

typedef struct {
    vec3 position;
    float intensity;
    mColor color;
    mLightType type;
    vec3 direction;       // Spotlight direction in world space; defaults to down.
    float range;          // World units; <= 0 uses 25.
    float innerCone;      // Spotlight half angles in degrees; defaults to 20/30.
    float outerCone;
    int castsShadows;
    float shadowBias;     // World units; <= 0 uses 0.02.
} mLight;

typedef struct {
    float deltaTime;
    float startTime;
    float timeSinceStart;
    float currentTime;
} mTime;

typedef struct {
    vec3 position;
    mColor color;
    union {
        vec3 normals;
        vec3 normal;
    };
    vec2 texCoord;
    float texId;
} mVertex;

typedef struct {
    unsigned int id;
    int width;
    int height;
    int channels;
} mTexture;

typedef struct {
    unsigned int framebuffer;
    unsigned int depthBuffer;
    mTexture color;
    int active;
    int previousFramebuffer;
    int previousViewport[4];
} mRenderTarget;

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


typedef struct {
    unsigned int drawCalls;
    unsigned int culledDrawCalls; // Batch/pass combinations rejected by frustum culling.
    unsigned int quadCount;
    unsigned int vertexCount;
    unsigned int indexCount;
} mRendererStats;

#define MAX_BATCH_QUADS 10000
#define MAX_BATCH_VERTICES (MAX_BATCH_QUADS * 4)
#define MAX_BATCH_INDICES (MAX_BATCH_QUADS * 6)
// Reserve the sixteenth fragment texture unit for the shadow array (GL 4.1).
#define MAX_TEXTURE_SLOTS 15
#define MAX_LIGHTS 8
#define SHADOW_MAP_SIZE 512

typedef struct mRenderBatch mRenderBatch;

typedef struct {
    GLFWwindow* handle;
    int width;
    int height;
    const char* title;
    int running;
    mColor clearColor;
} mWindow;

typedef struct {
    unsigned int shaderProgram;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int whiteTexture;
    unsigned int fullscreenVao;
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
    mRendererStats stats; // Last completed frame; safe to read anywhere in the next frame.
    mRendererStats pendingStats; // Counters for work currently being submitted.
    int isBatching;
    int frustumCulling;
    mLight* lights[MAX_LIGHTS]; // Borrowed pointers; caller owns each light.
    unsigned int lightCount;
    float ambientLight;
    float materialSpecular;
    float materialShininess;
    unsigned int shadowTexture;
    unsigned int shadowFramebuffer;
    unsigned int shadowProgram;
    mat4 lightMatrices[MAX_LIGHTS][6];
    mRenderBatch* firstBatch;
    mRenderBatch* lastBatch;
} mRenderer;

typedef struct {
    mWindow window;
    mRenderer renderer;
    mTime time;
    struct {
        mRenderTarget scene;
        unsigned int postProcess;
        int active;
    } frame;
    struct {
        void* handle; // ImGui context, kept private to the UI implementation.
        int frameActive;
    } ui;
} mContext;

typedef struct {
    mColor color;
    mTexture texture; // Borrowed; id == 0 means plain color.
    float specular;
    float shininess;
} mMaterial;

typedef struct {
    unsigned int firstVertex, vertexCount;
    unsigned int firstIndex, indexCount;
    unsigned int materialIndex;
} mMeshPart;

typedef struct {
    mVertex* vertices;
    unsigned int* indices; // Global vertex indices, including for imported parts.
    unsigned int vertexCount;
    unsigned int indexCount;
    mTexture* textures; // Unique GPU textures owned by this imported mesh.
    mColor colors;     // Global tint; initialize manually constructed meshes to WHITE.
    unsigned int textureCount;
    mMaterial* materials;
    unsigned int materialCount;
    mMeshPart* parts;
    unsigned int partCount;
} mMesh;

typedef struct {
    vec3 position;
    vec3 rotation;
    vec3 scale;
} mTransform;

typedef struct {
    mMesh* mesh;
    mTransform* transform;
    mMaterial* material;
} mObject;

#endif //PEACH_STRUCTS_H

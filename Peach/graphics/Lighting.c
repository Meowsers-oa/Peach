#include "Peach/graphics/Lighting.h"
#include "Peach/graphics/Shader.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHADOW_NEAR 0.05f

mLight mCreatePointLight(vec3 position, mColor color, float intensity) {
    mLight light = {0};
    glm_vec3_copy(position, light.position);
    light.color = color;
    light.intensity = intensity;
    light.type = mPointLight;
    light.direction[1] = -1.0f;
    light.range = 25.0f;
    light.innerCone = 20.0f;
    light.outerCone = 30.0f;
    light.castsShadows = 1;
    light.shadowBias = 0.02f;
    return light;
}

mLight mCreateSpotLight(vec3 position, vec3 direction, mColor color, float intensity) {
    mLight light = mCreatePointLight(position, color, intensity);
    light.type = mSpotLight;
    glm_vec3_copy(direction, light.direction);
    return light;
}

int mRendererAddLight(mContext* ctx, mLight* light) {
    if (!ctx || !light || (light->type != mPointLight && light->type != mSpotLight)) return 0;
    for (unsigned int i = 0; i < ctx->renderer.lightCount; ++i)
        if (ctx->renderer.lights[i] == light) return 1;
    if (ctx->renderer.lightCount == MAX_LIGHTS) return 0;
    ctx->renderer.lights[ctx->renderer.lightCount++] = light;
    return 1;
}

void mRendererRemoveLight(mContext* ctx, mLight* light) {
    if (!ctx || !light) return;
    mRenderer* r = &ctx->renderer;
    for (unsigned int i = 0; i < r->lightCount; ++i) {
        if (r->lights[i] != light) continue;
        memmove(r->lights + i, r->lights + i + 1,
                (r->lightCount - i - 1) * sizeof(*r->lights));
        r->lights[--r->lightCount] = NULL;
        return;
    }
}

void addLight(mContext* ctx, mLight* light) {
    if (!mRendererAddLight(ctx, light))
        fprintf(stderr, "Peach: light registration failed (invalid light or MAX_LIGHTS reached).\n");
}
void removeLight(mContext* ctx, mLight* light) { mRendererRemoveLight(ctx, light); }

void mRendererSetAmbientLight(mContext* ctx, float intensity) {
    if (ctx) ctx->renderer.ambientLight = fmaxf(intensity, 0.0f);
}

static mLight resolvedLight(const mLight* source) {
    mLight l = *source;
    if (!(l.range > SHADOW_NEAR)) l.range = 25.0f;
    if (!(l.outerCone > 0.0f)) l.outerCone = 30.0f;
    l.outerCone = fminf(l.outerCone, 89.0f);
    if (!(l.innerCone > 0.0f)) l.innerCone = l.outerCone * (2.0f / 3.0f);
    l.innerCone = fminf(l.innerCone, l.outerCone - 0.01f);
    if (!(l.shadowBias > 0.0f)) l.shadowBias = 0.02f;
    l.intensity = fmaxf(l.intensity, 0.0f);
    if (glm_vec3_norm2(l.direction) < 1e-6f) {
        l.direction[0] = l.direction[2] = 0.0f;
        l.direction[1] = -1.0f;
    }
    glm_vec3_normalize(l.direction);
    return l;
}

static int initShadows(mRenderer* r) {
    if (r->shadowProgram && r->shadowTexture && r->shadowFramebuffer) return 1;
    r->shadowProgram = mCreateResourceShaderProgram("shadow.vert", "shadow.frag");
    if (!r->shadowProgram) return 0;
    glActiveTexture(GL_TEXTURE0 + MAX_TEXTURE_SLOTS);
    if (!r->shadowTexture) glGenTextures(1, &r->shadowTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, r->shadowTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24,
                 SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, MAX_LIGHTS * 6, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1, &r->shadowFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r->shadowFramebuffer);
    glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, r->shadowTexture, 0, 0);
    glDrawBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Peach: shadow framebuffer incomplete; rendering without shadows.\n");
        glDeleteFramebuffers(1, &r->shadowFramebuffer);
        glDeleteTextures(1, &r->shadowTexture);
        glDeleteProgram(r->shadowProgram);
        r->shadowFramebuffer = r->shadowTexture = r->shadowProgram = 0;
        return 0;
    }
    return 1;
}

void mLightingShutdown(mContext* ctx) {
    mRenderer* r = &ctx->renderer;
    if (r->shadowFramebuffer) glDeleteFramebuffers(1, &r->shadowFramebuffer);
    if (r->shadowTexture) glDeleteTextures(1, &r->shadowTexture);
    if (r->shadowProgram) glDeleteProgram(r->shadowProgram);
    r->shadowFramebuffer = r->shadowTexture = r->shadowProgram = 0;
    memset(r->lights, 0, sizeof(r->lights));
    r->lightCount = 0;
}

static void setMatrix(GLuint program, const char* name, mat4 matrix) {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, (float*)matrix);
}

static void bindTextures(GLuint program, mRenderBatch* batch) {
    int samplers[MAX_TEXTURE_SLOTS];
    for (unsigned int i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        samplers[i] = (int)i;
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, i < batch->textureCount ? batch->textures[i] : batch->textures[0]);
    }
    glUniform1iv(glGetUniformLocation(program, "uTextures"), MAX_TEXTURE_SLOTS, samplers);
}

static void uploadBatch(mRenderer* r, mRenderBatch* batch) {
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertexCount * sizeof(mVertex), batch->vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batch->indexCount * sizeof(unsigned int), batch->indices);
}

static void buildLightMatrices(mRenderer* r, unsigned int index, mLight* light) {
    static const float directions[6][3] = {
        {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
    };
    static const float ups[6][3] = {
        {0,-1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}, {0,-1,0}, {0,-1,0}
    };
    mat4 projection, view;
    glm_perspective(light->type == mSpotLight ? glm_rad(light->outerCone * 2.0f) : GLM_PI_2f,
                    1.0f, SHADOW_NEAR, light->range, projection);
    int faces = light->type == mSpotLight ? 1 : 6;
    for (int face = 0; face < faces; ++face) {
        vec3 target, up;
        if (light->type == mSpotLight) {
            glm_vec3_add(light->position, light->direction, target);
            glm_vec3_copy((vec3){0,1,0}, up);
            if (fabsf(light->direction[1]) > 0.99f) glm_vec3_copy((vec3){0,0,1}, up);
        } else {
            glm_vec3_add(light->position, (float*)directions[face], target);
            glm_vec3_copy((float*)ups[face], up);
        }
        glm_lookat(light->position, target, up, view);
        glm_mat4_mul(projection, view, r->lightMatrices[index][face]);
    }
}

static void uploadLights(mRenderer* r, GLuint program, mLight* lights, int shadows) {
    glUniform1i(glGetUniformLocation(program, "uLightCount"), (int)r->lightCount);
    glUniform1f(glGetUniformLocation(program, "uAmbientLight"), r->ambientLight);
    glUniform1i(glGetUniformLocation(program, "uShadowMaps"), MAX_TEXTURE_SLOTS);
    glActiveTexture(GL_TEXTURE0 + MAX_TEXTURE_SLOTS);
    glBindTexture(GL_TEXTURE_2D_ARRAY, r->shadowTexture);
    for (unsigned int i = 0; i < r->lightCount; ++i) {
        mLight* l = &lights[i];
        char name[80];
#define LOCATION(field) (snprintf(name, sizeof(name), "uLights[%u]." field, i), glGetUniformLocation(program, name))
        glUniform1i(LOCATION("type"), l->type);
        glUniform3fv(LOCATION("position"), 1, l->position);
        glUniform3fv(LOCATION("direction"), 1, l->direction);
        glUniform3f(LOCATION("color"), l->color.r, l->color.g, l->color.b);
        glUniform1f(LOCATION("intensity"), l->intensity);
        glUniform1f(LOCATION("range"), l->range);
        glUniform1f(LOCATION("innerCone"), cosf(glm_rad(l->innerCone)));
        glUniform1f(LOCATION("outerCone"), cosf(glm_rad(l->outerCone)));
        glUniform1f(LOCATION("bias"), l->shadowBias);
        glUniform1i(LOCATION("shadow"), shadows && l->castsShadows && l->intensity > 0.0f);
#undef LOCATION
        snprintf(name, sizeof(name), "uSpotMatrices[%u]", i);
        setMatrix(program, name, r->lightMatrices[i][0]);
    }
}

static void toggle(GLenum cap, int enabled) {
    if (enabled) glEnable(cap); else glDisable(cap);
}

void mLightingRenderScene(mContext* ctx) {
    mRenderer* r = &ctx->renderer;
    if (!r->firstBatch) return;
    // Shadow passes must not inherit the application's viewport, clipping or depth state.
    GLint framebuffer, viewport[4], program, vao, arrayBuffer, activeTexture;
    GLint depthFunc, cullMode, frontFace;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    GLboolean depthWrite;
    GLdouble clearDepth, depthRange[2];
    GLint polygonMode[2], textureBindings[MAX_TEXTURE_SLOTS], shadowBinding;
    for (int i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBindings[i]);
    }
    glActiveTexture(GL_TEXTURE0 + MAX_TEXTURE_SLOTS);
    glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &shadowBinding);
    int depth = glIsEnabled(GL_DEPTH_TEST), cull = glIsEnabled(GL_CULL_FACE);
    int blend = glIsEnabled(GL_BLEND), scissor = glIsEnabled(GL_SCISSOR_TEST);
    int polygonOffset = glIsEnabled(GL_POLYGON_OFFSET_FILL);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
    glGetIntegerv(GL_FRONT_FACE, &frontFace);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clearDepth);
    glGetDoublev(GL_DEPTH_RANGE, depthRange);
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);

    mLight lights[MAX_LIGHTS];
    int shadows = 0;
    for (unsigned int i = 0; i < r->lightCount; ++i) {
        lights[i] = resolvedLight(r->lights[i]);
        buildLightMatrices(r, i, &lights[i]);
        if (lights[i].castsShadows && lights[i].intensity > 0.0f) shadows = 1;
    }
    if (shadows) shadows = initShadows(r);
    if (shadows) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r->shadowFramebuffer);
        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glClearDepth(1.0);
        glDepthRange(0.0, 1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE); // Thin quads cast shadows from either side.
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glUseProgram(r->shadowProgram);
        for (unsigned int i = 0; i < r->lightCount; ++i) {
            if (!lights[i].castsShadows || lights[i].intensity <= 0.0f) continue;
            int faces = lights[i].type == mSpotLight ? 1 : 6;
            for (int face = 0; face < faces; ++face) {
                glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          r->shadowTexture, 0, (int)i * 6 + face);
                glClear(GL_DEPTH_BUFFER_BIT);
                setMatrix(r->shadowProgram, "uLightMatrix", r->lightMatrices[i][face]);
                for (mRenderBatch* b = r->firstBatch; b; b = b->next) {
                    uploadBatch(r, b);
                    setMatrix(r->shadowProgram, "uModel", b->model);
                    bindTextures(r->shadowProgram, b);
                    glDrawElements(GL_TRIANGLES, (GLsizei)b->indexCount, GL_UNSIGNED_INT, NULL);
                    r->stats.drawCalls++;
                }
            }
        }
    }
    toggle(GL_SCISSOR_TEST, scissor);
    toggle(GL_POLYGON_OFFSET_FILL, polygonOffset);
    glClearDepth(clearDepth);
    glDepthRange(depthRange[0], depthRange[1]);
    glPolygonMode(GL_FRONT_AND_BACK, (GLenum)polygonMode[0]);
    for (mRenderBatch* b = r->firstBatch; b; b = b->next) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b->framebuffer);
        glViewport(b->viewport[0], b->viewport[1], b->viewport[2], b->viewport[3]);
        toggle(GL_DEPTH_TEST, b->depthTest);
        toggle(GL_CULL_FACE, b->cullFace);
        toggle(GL_BLEND, b->blend);
        glDepthFunc(b->depthFunc);
        glDepthMask(b->depthWrite);
        glCullFace(b->cullMode);
        glFrontFace(b->frontFace);
        glUseProgram(b->shaderProgram);
        setMatrix(b->shaderProgram, "uView", b->view);
        setMatrix(b->shaderProgram, "uProjection", b->projection);
        setMatrix(b->shaderProgram, "uModel", b->model);
        bindTextures(b->shaderProgram, b);
        uploadLights(r, b->shaderProgram, lights, shadows);
        uploadBatch(r, b);
        glDrawElements(GL_TRIANGLES, (GLsizei)b->indexCount, GL_UNSIGNED_INT, NULL);
        r->stats.drawCalls++;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)framebuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    toggle(GL_DEPTH_TEST, depth);
    toggle(GL_CULL_FACE, cull);
    toggle(GL_BLEND, blend);
    glDepthFunc(depthFunc);
    glDepthMask(depthWrite);
    glCullFace(cullMode);
    glFrontFace(frontFace);
    glUseProgram((GLuint)program);
    glBindVertexArray((GLuint)vao);
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)arrayBuffer);
    for (int i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, (GLuint)textureBindings[i]);
    }
    glActiveTexture(GL_TEXTURE0 + MAX_TEXTURE_SLOTS);
    glBindTexture(GL_TEXTURE_2D_ARRAY, (GLuint)shadowBinding);
    glActiveTexture((GLenum)activeTexture);
}

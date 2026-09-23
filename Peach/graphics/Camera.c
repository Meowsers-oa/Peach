//
// Created by Štěpán Toman on 20.09.2026.
//

#include "Peach/graphics/Camera.h"
#include "Peach/platform/Input.h"
#include <math.h>
#include <string.h>

mCamera3D mCreateCamera3D(vec3 position, vec3 target, vec3 up, float fov) {
    mCamera3D camera = {0};
    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(target, camera.target);
    glm_vec3_copy(up, camera.up);
    camera.fov = fov > 0.0f ? fov : 45.0f;
    camera.nearPlane = 0.1f;
    camera.farPlane = 1000.0f;
    camera.aspect = 0.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}

mCamera3D mCreateCameraPerspective(vec3 position, vec3 target, float fov) {
    vec3 defaultUp = { 0.0f, 1.0f, 0.0f };
    return mCreateCamera3D(position, target, defaultUp, fov);
}

mCamera3D mCreateCamera(vec3 position, vec3 target, vec3 up, float fov) {
    return mCreateCamera3D(position, target, up, fov);
}

void mCameraGetViewMatrix(const mCamera3D* camera, mat4 dest) {
    if (!camera) {
        glm_mat4_identity(dest);
        return;
    }
    vec3 up;
    glm_vec3_copy((float*)camera->up, up);
    if (glm_vec3_norm2(up) < 1e-6f) {
        up[0] = 0.0f; up[1] = 1.0f; up[2] = 0.0f;
    }
    glm_lookat((float*)camera->position, (float*)camera->target, up, dest);
}

void mCameraGetProjectionMatrix(const mCamera3D* camera, float aspect, mat4 dest) {
    if (!camera) {
        glm_mat4_identity(dest);
        return;
    }
    float actualAspect = aspect;
    if (actualAspect <= 0.0f) {
        actualAspect = camera->aspect > 0.0f ? camera->aspect : (800.0f / 600.0f);
    }

    float nearVal = camera->nearPlane > 0.0f ? camera->nearPlane : 0.1f;
    float farVal = camera->farPlane > nearVal ? camera->farPlane : 1000.0f;
    float fovDeg = camera->fov > 0.0f ? camera->fov : 45.0f;

    if (camera->projection == CAMERA_ORTHOGRAPHIC) {
        float halfH = tanf(glm_rad(fovDeg) * 0.5f) * 10.0f;
        float halfW = halfH * actualAspect;
        glm_ortho(-halfW, halfW, -halfH, halfH, nearVal, farVal, dest);
    } else {
        glm_perspective(glm_rad(fovDeg), actualAspect, nearVal, farVal, dest);
    }
}

void mCameraGetViewProjectionMatrix(const mCamera3D* camera, float aspect, mat4 dest) {
    mat4 view, proj;
    mCameraGetViewMatrix(camera, view);
    mCameraGetProjectionMatrix(camera, aspect, proj);
    glm_mat4_mul(proj, view, dest);
}

void mCameraGetForward(const mCamera3D* camera, vec3 dest) {
    if (!camera) {
        dest[0] = 0.0f; dest[1] = 0.0f; dest[2] = -1.0f;
        return;
    }
    glm_vec3_sub((float*)camera->target, (float*)camera->position, dest);
    if (glm_vec3_norm2(dest) < 1e-6f) {
        dest[0] = 0.0f; dest[1] = 0.0f; dest[2] = -1.0f;
    } else {
        glm_vec3_normalize(dest);
    }
}

void mCameraGetRight(const mCamera3D* camera, vec3 dest) {
    if (!camera) {
        dest[0] = 1.0f; dest[1] = 0.0f; dest[2] = 0.0f;
        return;
    }
    vec3 forward, up;
    mCameraGetForward(camera, forward);
    glm_vec3_copy((float*)camera->up, up);
    if (glm_vec3_norm2(up) < 1e-6f) {
        up[0] = 0.0f; up[1] = 1.0f; up[2] = 0.0f;
    }
    glm_vec3_cross(forward, up, dest);
    if (glm_vec3_norm2(dest) < 1e-6f) {
        dest[0] = 1.0f; dest[1] = 0.0f; dest[2] = 0.0f;
    } else {
        glm_vec3_normalize(dest);
    }
}

void mCameraGetUp(const mCamera3D* camera, vec3 dest) {
    if (!camera) {
        dest[0] = 0.0f; dest[1] = 1.0f; dest[2] = 0.0f;
        return;
    }
    vec3 forward, right;
    mCameraGetForward(camera, forward);
    mCameraGetRight(camera, right);
    glm_vec3_cross(right, forward, dest);
    glm_vec3_normalize(dest);
}

void mCameraMoveForward(mCamera3D* camera, float distance, int moveInWorldPlane) {
    if (!camera) return;
    vec3 forward;
    mCameraGetForward(camera, forward);
    if (moveInWorldPlane) {
        forward[1] = 0.0f;
        if (glm_vec3_norm2(forward) > 1e-6f) {
            glm_vec3_normalize(forward);
        }
    }
    vec3 move;
    glm_vec3_scale(forward, distance, move);
    glm_vec3_add(camera->position, move, camera->position);
    glm_vec3_add(camera->target, move, camera->target);
}

void mCameraMoveRight(mCamera3D* camera, float distance, int moveInWorldPlane) {
    if (!camera) return;
    vec3 right;
    mCameraGetRight(camera, right);
    if (moveInWorldPlane) {
        right[1] = 0.0f;
        if (glm_vec3_norm2(right) > 1e-6f) {
            glm_vec3_normalize(right);
        }
    }
    vec3 move;
    glm_vec3_scale(right, distance, move);
    glm_vec3_add(camera->position, move, camera->position);
    glm_vec3_add(camera->target, move, camera->target);
}

void mCameraMoveUp(mCamera3D* camera, float distance) {
    if (!camera) return;
    vec3 up;
    mCameraGetUp(camera, up);
    vec3 move;
    glm_vec3_scale(up, distance, move);
    glm_vec3_add(camera->position, move, camera->position);
    glm_vec3_add(camera->target, move, camera->target);
}

void mCameraRotateYaw(mCamera3D* camera, float angleRad, int rotateAroundTarget) {
    if (!camera) return;
    vec3 up = { 0.0f, 1.0f, 0.0f };
    vec3 direction;

    if (rotateAroundTarget) {
        glm_vec3_sub(camera->position, camera->target, direction);
        glm_vec3_rotate(direction, angleRad, up);
        glm_vec3_add(camera->target, direction, camera->position);
    } else {
        glm_vec3_sub(camera->target, camera->position, direction);
        glm_vec3_rotate(direction, angleRad, up);
        glm_vec3_add(camera->position, direction, camera->target);
    }
}

void mCameraRotatePitch(mCamera3D* camera, float angleRad, int lockView, int rotateAroundTarget, int rotateUp) {
    if (!camera) return;
    vec3 right;
    mCameraGetRight(camera, right);
    vec3 direction;

    if (rotateAroundTarget) {
        glm_vec3_sub(camera->position, camera->target, direction);
        glm_vec3_rotate(direction, angleRad, right);
        glm_vec3_add(camera->target, direction, camera->position);
    } else {
        glm_vec3_sub(camera->target, camera->position, direction);
        glm_vec3_rotate(direction, angleRad, right);
        glm_vec3_add(camera->position, direction, camera->target);
    }

    if (rotateUp) {
        glm_vec3_rotate(camera->up, angleRad, right);
    }
}

void mCameraUpdateFree(mCamera3D* camera, float deltaTime, float moveSpeed, float mouseSensitivity) {
    if (!camera) return;

    float speed = moveSpeed * deltaTime;


    if (mIsKeyDown(KEY_LEFT_SHIFT)) {
        speed *= 3.0f;
    }

    if (mIsKeyDown(KEY_W)) {
        mCameraMoveForward(camera, speed, 0);
    }
    if (mIsKeyDown(KEY_S)) {
        mCameraMoveForward(camera, -speed, 0);
    }
    if (mIsKeyDown(KEY_D)) {
        mCameraMoveRight(camera, speed, 0);
    }
    if (mIsKeyDown(KEY_A)) {
        mCameraMoveRight(camera, -speed, 0);
    }
    if (mIsKeyDown(KEY_E)) {
        mCameraMoveUp(camera, speed);
    }
    if (mIsKeyDown(KEY_Q)) {
        mCameraMoveUp(camera, -speed);
    }

    static double lastMouseX = 0, lastMouseY = 0;
    static int wasRightMouseDown = 0;

    int isRightMouseDown = mIsMouseButtonDown(MOUSE_RIGHT);
    double mouseX, mouseY;
    mGetMousePosition(&mouseX, &mouseY);

    if (isRightMouseDown) {
        if (!wasRightMouseDown) {
            // RMB was just pressed: hide/lock cursor and sync mouse coordinates to prevent camera jumps
            mDisableCursor();
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        float xOffset = (float)(mouseX - lastMouseX) * mouseSensitivity;
        float yOffset = (float)(lastMouseY - mouseY) * mouseSensitivity;

        mCameraRotateYaw(camera, -glm_rad(xOffset), 0);
        mCameraRotatePitch(camera, glm_rad(yOffset), 1, 0, 0);

        lastMouseX = mouseX;
        lastMouseY = mouseY;
    } else if (wasRightMouseDown) {
        mEnableCursor();
    }

    wasRightMouseDown = isRightMouseDown;
}

void mCameraUpdateOrbit(mCamera3D* camera, vec3 target, float radius, float speed, float deltaTime) {
    if (!camera) return;
    static float angle = 0.0f;
    angle += speed * deltaTime;

    glm_vec3_copy(target, camera->target);
    camera->position[0] = target[0] + radius * sinf(angle);
    camera->position[1] = target[1] + radius * 0.5f;
    camera->position[2] = target[2] + radius * cosf(angle);
}
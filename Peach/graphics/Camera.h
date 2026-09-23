//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_CAMERA_H
#define PEACH_CAMERA_H

#include "Peach/core/Structs.h"

// Camera creation helpers
mCamera3D mCreateCamera3D(vec3 position, vec3 target, vec3 up, float fov);
mCamera3D mCreateCameraPerspective(vec3 position, vec3 target, float fov);
mCamera3D mCreateCamera(vec3 position, vec3 target, vec3 up, float fov);

// Camera matrix calculations
void mCameraGetViewMatrix(const mCamera3D* camera, mat4 dest);
void mCameraGetProjectionMatrix(const mCamera3D* camera, float aspect, mat4 dest);
void mCameraGetViewProjectionMatrix(const mCamera3D* camera, float aspect, mat4 dest);

// Direction vectors
void mCameraGetForward(const mCamera3D* camera, vec3 dest);
void mCameraGetRight(const mCamera3D* camera, vec3 dest);
void mCameraGetUp(const mCamera3D* camera, vec3 dest);

// Transformations / Movements
void mCameraMoveForward(mCamera3D* camera, float distance, int moveInWorldPlane);
void mCameraMoveRight(mCamera3D* camera, float distance, int moveInWorldPlane);
void mCameraMoveUp(mCamera3D* camera, float distance);
void mCameraRotateYaw(mCamera3D* camera, float angleRad, int rotateAroundTarget);
void mCameraRotatePitch(mCamera3D* camera, float angleRad, int lockView, int rotateAroundTarget, int rotateUp);

// Camera controllers
void mCameraUpdateFree(mCamera3D* camera, float deltaTime, float moveSpeed, float mouseSensitivity);
void mCameraUpdateOrbit(mCamera3D* camera, vec3 target, float radius, float speed, float deltaTime);

#endif //PEACH_CAMERA_H
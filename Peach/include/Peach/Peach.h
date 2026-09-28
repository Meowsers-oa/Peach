//
// Created by Štěpán Toman on 27.09.2026.
//

#ifndef PEACH_PEACH_H
#define PEACH_PEACH_H

#include <Peach/Common.h>
#include <Peach/mWindow.h>
#include <Peach/Input.h>
#include <Peach/Camera.h>
#include <Peach/Texture.h>
#include <Peach/Renderer.h>
#include <Peach/Transform.h>
#include <Peach/Shader.h>
#include <Peach/Time.h>
#include <Peach/Shape.h>
#include <Peach/Map.h>
#include <Peach/Resource.h>
#include <Peach/Sprite.h>

mContext mContextCreate();
void mUpdate(mContext* ctx);
// Releases all registered sprites/resources, renderer, maps and window, then GLFW.
// Safe after partial setup and safe to call again. Unregistered resources stay caller-owned.
void mEnd(mContext* ctx);
// Compatibility alias for mEnd.
void mDestroy(mContext* ctx);



#endif //PEACH_PEACH_H

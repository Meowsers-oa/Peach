//
// Created by Štěpán Toman on 27.09.2026.
//

#ifndef PEACH_PEACH_H
#define PEACH_PEACH_H

#include <stdio.h>

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
#include <Peach/SpriteSheet.h>
#include <Peach/Animation.h>
#include <Peach/Light.h>
#include <Peach/Utils.h>
#include <Peach/Particles.h>
#include <Peach/List.h>

mContext mContextCreate();
void mUpdate(mContext* ctx);
void mEnd(mContext* ctx);
void mDestroy(mContext* ctx);

#endif //PEACH_PEACH_H

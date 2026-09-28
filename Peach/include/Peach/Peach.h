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

mContext mContextCreate();
void mUpdate(mContext* ctx);
void mDestroy(mContext* ctx);

#endif //PEACH_PEACH_H

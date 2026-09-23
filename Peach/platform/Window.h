//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_WINDOW_H
#define PEACH_WINDOW_H

#include "Peach/core/Structs.h"

int mWindowCreate(mContext* context, int width, int height, const char* title);
void mOnWindowResize(GLFWwindow* window, int width, int height);
void mWindowUpdate(mContext* context);
void mWindowStop(mContext* context);
void mWindowEnd(mContext* context);

#endif //PEACH_WINDOW_H

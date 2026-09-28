//
// Created by Štěpán Toman on 27.09.2026.
//

#ifndef PEACH_MWINDOW_H
#define PEACH_MWINDOW_H

#include <Peach/Common.h>
#include <stdio.h>

void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
int mWindowCreate(mContext* ctx, mWindowInfo* info);
void mWindowUpdate(mContext* ctx);
void mWindowDestroy(mContext* ctx);

#endif //PEACH_MWINDOW_H

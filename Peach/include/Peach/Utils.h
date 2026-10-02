//
// Created by Štěpán Toman on 29.09.2026.
//

#ifndef PEACH_UTILS_H
#define PEACH_UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <Peach/Common.h>

static const char* mFormatString(char *out_buf, size_t buf_size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(out_buf, buf_size, fmt, args);
    va_end(args);
    return out_buf;
}

static float mLerp(float a, float b, float t) {
    return a * (1 - t) + b * t;
}

static mColor mColorBlend(mColor* a, mColor* b, float t) {
    return (mColor){.r = mLerp(a->r, b->r, t), .g = mLerp(a->g, b->g, t), .b = mLerp(a->b, b->b, t), .a = mLerp(a->a, b->a, t)};
}

#endif //PEACH_UTILS_H

//
// Created by Štěpán Toman on 27.09.2026.
//

#ifndef PEACH_COMMON_H
#define PEACH_COMMON_H

#include <glad/glad.h>
#include <Peach/Time.h>

#include <GLFW/glfw3.h>

#define M_SUCCESS 0
#define M_FAILURE (-1)
#define M_TRUE 1
#define M_FALSE 0

typedef struct {
    float r;
    float g;
    float b;
    float a;
}mColor;

// Color components are normalized floats. RGB/RGBA accept values from 0 to 255.
#define M_COLOR(r, g, b, a) ((mColor){(r), (g), (b), (a)})
#define M_RGBA(r, g, b, a) M_COLOR((r) / 255.0f, (g) / 255.0f, (b) / 255.0f, (a) / 255.0f)
#define M_RGB(r, g, b) M_RGBA(r, g, b, 255)

#define M_COLOR_TRANSPARENT M_RGBA(0, 0, 0, 0)
#define M_COLOR_BLACK M_RGBA(0, 0, 0, 255)
#define M_COLOR_WHITE M_RGBA(255, 255, 255, 255)
#define M_COLOR_RED M_RGBA(255, 0, 0, 255)
#define M_COLOR_GREEN M_RGBA(0, 255, 0, 255)
#define M_COLOR_BLUE M_RGBA(0, 0, 255, 255)
#define M_COLOR_YELLOW M_RGBA(255, 255, 0, 255)
#define M_COLOR_CYAN M_RGBA(0, 255, 255, 255)
#define M_COLOR_MAGENTA M_RGBA(255, 0, 255, 255)
#define M_COLOR_GRAY M_RGBA(128, 128, 128, 255)
#define M_COLOR_LIGHT_GRAY M_RGBA(192, 192, 192, 255)
#define M_COLOR_DARK_GRAY M_RGBA(64, 64, 64, 255)
#define M_COLOR_ORANGE M_RGBA(255, 165, 0, 255)
#define M_COLOR_PINK M_RGBA(255, 192, 203, 255)
#define M_COLOR_PURPLE M_RGBA(128, 0, 128, 255)
#define M_COLOR_BROWN M_RGBA(165, 42, 42, 255)
#define M_COLOR_LIME M_RGBA(191, 255, 0, 255)
#define M_COLOR_NAVY M_RGBA(0, 0, 128, 255)
#define M_COLOR_TEAL M_RGBA(0, 128, 128, 255)
#define M_COLOR_OLIVE M_RGBA(128, 128, 0, 255)
#define M_COLOR_GOLD M_RGBA(255, 215, 0, 255)
#define M_COLOR_PEACH M_RGBA(255, 218, 185, 255)

typedef struct {
    int width;
    int height;
    char* title;
}mWindowInfo;

typedef struct {
    int running;
    mColor bgColor;
    GLFWwindow* handle;
}mWindow;

typedef struct {
    int keys[GLFW_KEY_LAST + 1];
    int keysPressed[GLFW_KEY_LAST + 1];
    int keysReleased[GLFW_KEY_LAST + 1];
    int mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
    int mouseButtonsPressed[GLFW_MOUSE_BUTTON_LAST + 1];
    int mouseButtonsReleased[GLFW_MOUSE_BUTTON_LAST + 1];
    double mouseX;
    double mouseY;
    double scrollX;
    double scrollY;
}mInput;

typedef struct mRenderer mRenderer;

typedef struct {
    mWindow window;
    mInput input;
    mTime time;
    mRenderer* renderer;
}mContext;

#endif //PEACH_COMMON_H

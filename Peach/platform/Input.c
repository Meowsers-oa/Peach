//
// Created by Štěpán Toman on 20.09.2026.
//

#include "Peach/platform/Input.h"
#include <string.h>

#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8

typedef struct {
    GLFWwindow* window;
    unsigned char keys[MAX_KEYS];
    unsigned char keysLast[MAX_KEYS];
    unsigned char mouseButtons[MAX_MOUSE_BUTTONS];
    unsigned char mouseButtonsLast[MAX_MOUSE_BUTTONS];
    double mouseX, mouseY;
    double scrollX, scrollY;
} mInputContext;

static mInputContext inputCtx = {0};

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    inputCtx.scrollX = xoffset;
    inputCtx.scrollY = yoffset;
}

void mInputStart(GLFWwindow* window) {
    inputCtx.window = window;
    memset(inputCtx.keys, 0, sizeof(inputCtx.keys));
    memset(inputCtx.keysLast, 0, sizeof(inputCtx.keysLast));
    memset(inputCtx.mouseButtons, 0, sizeof(inputCtx.mouseButtons));
    memset(inputCtx.mouseButtonsLast, 0, sizeof(inputCtx.mouseButtonsLast));

    glfwSetScrollCallback(window, scrollCallback);
}

void mInputUpdate(void) {
    memcpy(inputCtx.keysLast, inputCtx.keys, sizeof(inputCtx.keys));
    memcpy(inputCtx.mouseButtonsLast, inputCtx.mouseButtons, sizeof(inputCtx.mouseButtons));

    inputCtx.scrollX = 0.0;
    inputCtx.scrollY = 0.0;

    if (!inputCtx.window) return;

    for (int i = 0; i < MAX_KEYS; i++) {
        inputCtx.keys[i] = (unsigned char)(glfwGetKey(inputCtx.window, i) == GLFW_PRESS);
    }

    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
        inputCtx.mouseButtons[i] = (unsigned char)(glfwGetMouseButton(inputCtx.window, i) == GLFW_PRESS);
    }

    glfwGetCursorPos(inputCtx.window, &inputCtx.mouseX, &inputCtx.mouseY);
}

int mIsKeyDown(mKey key) {
    if (key < 0 || key >= MAX_KEYS) return 0;
    return inputCtx.keys[key];
}

int mIsKeyPressed(mKey key) {
    if (key < 0 || key >= MAX_KEYS) return 0;
    return inputCtx.keys[key] && !inputCtx.keysLast[key];
}

int mIsKeyReleased(mKey key) {
    if (key < 0 || key >= MAX_KEYS) return 0;
    return !inputCtx.keys[key] && inputCtx.keysLast[key];
}

int mIsMouseButtonDown(mMouseButton button) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return 0;
    return inputCtx.mouseButtons[button];
}

int mIsMouseButtonPressed(mMouseButton button) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return 0;
    return inputCtx.mouseButtons[button] && !inputCtx.mouseButtonsLast[button];
}

int mIsMouseButtonReleased(mMouseButton button) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return 0;
    return !inputCtx.mouseButtons[button] && inputCtx.mouseButtonsLast[button];
}

void mGetMousePosition(double* x, double* y) {
    if (x) *x = inputCtx.mouseX;
    if (y) *y = inputCtx.mouseY;
}

void mGetMouseScroll(double* xoffset, double* yoffset) {
    if (xoffset) *xoffset = inputCtx.scrollX;
    if (yoffset) *yoffset = inputCtx.scrollY;
}

void mDisableCursor(void) {
    if (inputCtx.window) {
        glfwSetInputMode(inputCtx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void mEnableCursor(void) {
    if (inputCtx.window) {
        glfwSetInputMode(inputCtx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
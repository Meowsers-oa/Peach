#include <Peach/Input.h>
#include <Peach/Renderer.h>
#include <string.h>

static void updateMousePosition(mContext* ctx) {
    double x, y;
    glfwGetCursorPos(ctx->window.handle, &x, &y);
    mRendererWindowToScreen(ctx, x, y, &x, &y);
    ctx->input.mouseX = x;
    ctx->input.mouseY = y;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    mContext* ctx = glfwGetWindowUserPointer(window);
    (void)scancode;
    (void)mods;

    if (ctx == NULL || key < 0 || key > M_KEY_LAST) return;

    if (action == GLFW_PRESS) {
        ctx->input.keys[key] = M_TRUE;
        ctx->input.keysPressed[key] = M_TRUE;
    } else if (action == GLFW_RELEASE) {
        ctx->input.keys[key] = M_FALSE;
        ctx->input.keysReleased[key] = M_TRUE;
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    mContext* ctx = glfwGetWindowUserPointer(window);
    (void)mods;

    if (ctx == NULL || button < 0 || button > M_MOUSE_BUTTON_LAST) return;

    if (action == GLFW_PRESS) {
        ctx->input.mouseButtons[button] = M_TRUE;
        ctx->input.mouseButtonsPressed[button] = M_TRUE;
    } else if (action == GLFW_RELEASE) {
        ctx->input.mouseButtons[button] = M_FALSE;
        ctx->input.mouseButtonsReleased[button] = M_TRUE;
    }
}

static void scrollCallback(GLFWwindow* window, double x, double y) {
    mContext* ctx = glfwGetWindowUserPointer(window);
    if (ctx == NULL) return;

    ctx->input.scrollX += x;
    ctx->input.scrollY += y;
}

void mInputInit(mContext* ctx) {
    ctx->input = (mInput){0};
    glfwSetWindowUserPointer(ctx->window.handle, ctx);
    glfwSetKeyCallback(ctx->window.handle, keyCallback);
    glfwSetMouseButtonCallback(ctx->window.handle, mouseButtonCallback);
    glfwSetScrollCallback(ctx->window.handle, scrollCallback);
    updateMousePosition(ctx);
}

void mInputUpdate(mContext* ctx) {
    memset(ctx->input.keysPressed, 0, sizeof(ctx->input.keysPressed));
    memset(ctx->input.keysReleased, 0, sizeof(ctx->input.keysReleased));
    memset(ctx->input.mouseButtonsPressed, 0, sizeof(ctx->input.mouseButtonsPressed));
    memset(ctx->input.mouseButtonsReleased, 0, sizeof(ctx->input.mouseButtonsReleased));
    ctx->input.scrollX = 0.0;
    ctx->input.scrollY = 0.0;

    glfwPollEvents();
    updateMousePosition(ctx);
}

int mKeyDown(mContext* ctx, mKey key) {
    if (key < 0 || key > M_KEY_LAST) return M_FALSE;
    return ctx->input.keys[key];
}

int mKeyPressed(mContext* ctx, mKey key) {
    if (key < 0 || key > M_KEY_LAST) return M_FALSE;
    return ctx->input.keysPressed[key];
}

int mKeyReleased(mContext* ctx, mKey key) {
    if (key < 0 || key > M_KEY_LAST) return M_FALSE;
    return ctx->input.keysReleased[key];
}

int mMouseButtonDown(mContext* ctx, mMouseButton button) {
    if (button < 0 || button > M_MOUSE_BUTTON_LAST) return M_FALSE;
    return ctx->input.mouseButtons[button];
}

int mMouseButtonPressed(mContext* ctx, mMouseButton button) {
    if (button < 0 || button > M_MOUSE_BUTTON_LAST) return M_FALSE;
    return ctx->input.mouseButtonsPressed[button];
}

int mMouseButtonReleased(mContext* ctx, mMouseButton button) {
    if (button < 0 || button > M_MOUSE_BUTTON_LAST) return M_FALSE;
    return ctx->input.mouseButtonsReleased[button];
}

void mMousePosition(mContext* ctx, double* x, double* y) {
    updateMousePosition(ctx);
    if (x != NULL) *x = ctx->input.mouseX;
    if (y != NULL) *y = ctx->input.mouseY;
}

void mMouseScroll(mContext* ctx, double* x, double* y) {
    if (x != NULL) *x = ctx->input.scrollX;
    if (y != NULL) *y = ctx->input.scrollY;
}

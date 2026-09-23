//
// Created by Štěpán Toman on 20.09.2026.
//

#ifndef PEACH_INPUT_H
#define PEACH_INPUT_H

#include "Peach/core/Structs.h"

typedef enum {
    KEY_UNKNOWN      = -1,
    KEY_SPACE        = 32,
    KEY_APOSTROPHE   = 39,
    KEY_COMMA        = 44,
    KEY_MINUS        = 45,
    KEY_PERIOD       = 46,
    KEY_SLASH        = 47,
    KEY_0            = 48,
    KEY_1            = 49,
    KEY_2            = 50,
    KEY_3            = 51,
    KEY_4            = 52,
    KEY_5            = 53,
    KEY_6            = 54,
    KEY_7            = 55,
    KEY_8            = 56,
    KEY_9            = 57,
    KEY_SEMICOLON    = 59,
    KEY_EQUAL        = 61,
    KEY_A            = 65,
    KEY_B            = 66,
    KEY_C            = 67,
    KEY_D            = 68,
    KEY_E            = 69,
    KEY_F            = 70,
    KEY_G            = 71,
    KEY_H            = 72,
    KEY_I            = 73,
    KEY_J            = 74,
    KEY_K            = 75,
    KEY_L            = 76,
    KEY_M            = 77,
    KEY_N            = 78,
    KEY_O            = 79,
    KEY_P            = 80,
    KEY_Q            = 81,
    KEY_R            = 82,
    KEY_S            = 83,
    KEY_T            = 84,
    KEY_U            = 85,
    KEY_V            = 86,
    KEY_W            = 87,
    KEY_X            = 88,
    KEY_Y            = 89,
    KEY_Z            = 90,
    KEY_ESCAPE       = 256,
    KEY_ENTER        = 257,
    KEY_TAB          = 258,
    KEY_BACKSPACE    = 259,
    KEY_INSERT       = 260,
    KEY_DELETE       = 261,
    KEY_RIGHT        = 262,
    KEY_LEFT         = 263,
    KEY_DOWN         = 264,
    KEY_UP           = 265,
    KEY_LEFT_SHIFT   = 340,
    KEY_LEFT_CONTROL = 341,
    KEY_LEFT_ALT     = 342
} mKey;
typedef enum {
    MOUSE_LEFT   = 0,
    MOUSE_RIGHT  = 1,
    MOUSE_MIDDLE = 2
} mMouseButton;

void mInputStart(GLFWwindow* window);
void mInputUpdate(void);

int mIsKeyDown(mKey key);
int mIsKeyPressed(mKey key);
int mIsKeyReleased(mKey key);

int mIsMouseButtonDown(mMouseButton button);
int mIsMouseButtonPressed(mMouseButton button);
int mIsMouseButtonReleased(mMouseButton button);

void mGetMousePosition(double* x, double* y);
void mGetMouseScroll(double* xoffset, double* yoffset);

void mDisableCursor(void);
void mEnableCursor(void);

#endif //PEACH_INPUT_H
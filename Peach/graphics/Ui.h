#ifndef PEACH_UI_H
#define PEACH_UI_H

#include "Peach/core/Structs.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    mUiWindowNone = 0,
    mUiWindowAutoResize = 1 << 0,
    mUiWindowNoTitleBar = 1 << 1,
    mUiWindowNoResize = 1 << 2,
    mUiWindowNoMove = 1 << 3,
    mUiWindowNoCollapse = 1 << 4
} mUiWindowFlags;


int mUiStart(mContext* ctx);
void mUiUpdate(mContext* ctx);
void mUiRender(mContext* ctx);
void mUiEnd(mContext* ctx);
bool mUiWantsMouse(mContext* ctx);
bool mUiWantsKeyboard(mContext* ctx);


bool mUiBeginWindow(const char* title, bool* open, int flags);
void mUiEndWindow(void);
void mUiSetNextWindowPosition(float x, float y, bool firstUseOnly);
void mUiSetNextWindowSize(float width, float height, bool firstUseOnly);

void mUiText(const char* text);
void mUiTextf(const char* format, ...);
void mUiTextWrapped(const char* text);

bool mUiButton(const char* label);
bool mUiButtonSized(const char* label, float width, float height);
bool mUiCheckbox(const char* label, bool* value);
bool mUiSliderFloat(const char* label, float* value, float min, float max);
bool mUiSliderInt(const char* label, int* value, int min, int max);
bool mUiDragFloat(const char* label, float* value, float speed, float min, float max);
bool mUiDragFloat2(const char* label, float values[2], float speed, float min, float max);
bool mUiDragFloat3(const char* label, float values[3], float speed, float min, float max);
bool mUiDragFloat4(const char* label, float values[4], float speed, float min, float max);
bool mUiInputFloat(const char* label, float* value, float step);
bool mUiInputInt(const char* label, int* value, int step);
bool mUiInputText(const char* label, char* buffer, size_t capacity);
bool mUiInputTextMultiline(const char* label, char* buffer, size_t capacity, float width, float height);
bool mUiCombo(const char* label, int* selected, const char* const items[], int count);
bool mUiColorEdit(const char* label, mColor* color);
bool mUiCollapsingHeader(const char* label);

void mUiSameLine(void);
void mUiSeparator(void);
void mUiSpacing(void);
void mUiPushID(int id);
void mUiPopID(void);
void mUiBeginDisabled(bool disabled);
void mUiEndDisabled(void);

#ifdef __cplusplus
}
#endif
#endif

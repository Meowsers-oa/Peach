#ifndef PEACH_UI_H
#define PEACH_UI_H

#include "Peach/core/Structs.h"
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


typedef enum { mGizmoTranslate, mGizmoRotate, mGizmoScale } mGizmoOperation;
typedef enum { mGizmoLocal, mGizmoWorld } mGizmoMode;

// Call once per object per frame, outside UI windows. IDs must be unique.
// Matrices use the same column-major layout as the renderer.
int mUiGizmo(mContext* ctx, const mCamera3D* camera, int id, mat4 transform,
             mGizmoOperation operation, mGizmoMode mode);
int mUiGizmoPosition(mContext* ctx, const mCamera3D* camera, int id, vec3 position);
int mUiGizmoIsUsing(void);
int mUiGizmoIsOver(void);

int mUiStart(mContext* ctx);
void mUiUpdate(mContext* ctx);
void mUiRender(mContext* ctx);
void mUiEnd(mContext* ctx);
int mUiWantsMouse(mContext* ctx);
int mUiWantsKeyboard(mContext* ctx);


// Always pair with mUiEndWindow, even when the return value is 0.
int mUiBeginWindow(const char* title);
int mUiBeginWindowEx(const char* title, int* open, int flags);
void mUiEndWindow(void);
void mUiSetNextWindowPosition(float x, float y, int firstUseOnly);
void mUiSetNextWindowSize(float width, float height, int firstUseOnly);

void mUiText(const char* text);
void mUiTextf(const char* format, ...);
void mUiTextWrapped(const char* text);

int mUiButton(const char* label);
int mUiButtonSized(const char* label, float width, float height);
int mUiCheckbox(const char* label, int* value);
int mUiSliderFloat(const char* label, float* value, float min, float max);
int mUiSliderInt(const char* label, int* value, int min, int max);
int mUiDragFloat(const char* label, float* value, float speed, float min, float max);
int mUiDragFloat2(const char* label, float values[2], float speed, float min, float max);
int mUiDragFloat3(const char* label, float values[3], float speed, float min, float max);
int mUiDragFloat4(const char* label, float values[4], float speed, float min, float max);
int mUiInputFloat(const char* label, float* value, float step);
int mUiInputInt(const char* label, int* value, int step);
int mUiInputText(const char* label, char* buffer, size_t capacity);
int mUiInputTextMultiline(const char* label, char* buffer, size_t capacity, float width, float height);
int mUiCombo(const char* label, int* selected, const char* const items[], int count);
int mUiColorEdit(const char* label, mColor* color);
int mUiCollapsingHeader(const char* label);

void mUiSameLine(void);
void mUiSeparator(void);
void mUiSpacing(void);
void mUiPushID(int id);
void mUiPopID(void);
void mUiBeginDisabled(int disabled);
void mUiEndDisabled(void);

void mUiStatsPanel(mContext* ctx);

#ifdef __cplusplus
}
#endif
#endif

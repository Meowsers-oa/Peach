#include "Peach/graphics/Ui.h"
#include "Peach/graphics/Camera.h"
#include <stdarg.h>
#include <stdbool.h>

// Request the C definitions instead of the original C++ library types.
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>
#include <cimguizmo.h>

#include "Peach/core/Utils.h"

static int selectUi(mContext* ctx) {
    if (!ctx || !ctx->ui.handle) return 0;
    igSetCurrentContext((ImGuiContext*)ctx->ui.handle);
    return 1;
}

int mUiStart(mContext* ctx) {
    if (!ctx || !ctx->window.handle || glfwGetCurrentContext() != ctx->window.handle)
        return 0;
    if (ctx->ui.handle) return 1;
    ImGuiContext* previous = igGetCurrentContext();
    ctx->ui.handle = igCreateContext(NULL);
    if (!ctx->ui.handle) return 0;
    selectUi(ctx);
    igGetIO_Nil()->IniFilename = NULL;
    igGetIO_Nil()->ConfigDragClickToInputText = 1;
    igStyleColorsDark(NULL);
    ImGuizmo_SetImGuiContext((ImGuiContext*)ctx->ui.handle);
    if (!ImGui_ImplGlfw_InitForOpenGL(ctx->window.handle, 1)) {
        igDestroyContext((ImGuiContext*)ctx->ui.handle);
        ctx->ui.handle = NULL;
        igSetCurrentContext(previous);
        return 0;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 410 core")) {
        ImGui_ImplGlfw_Shutdown();
        igDestroyContext((ImGuiContext*)ctx->ui.handle);
        ctx->ui.handle = NULL;
        igSetCurrentContext(previous);
        return 0;
    }
    mUiUpdate(ctx);
    return 1;
}

void mUiUpdate(mContext* ctx) {
    if (!selectUi(ctx) || ctx->ui.frameActive) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
    ImGuizmo_BeginFrame();
    ctx->ui.frameActive = 1;
}

void mUiRender(mContext* ctx) {
    if (!selectUi(ctx) || !ctx->ui.frameActive) return;
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
    ctx->ui.frameActive = 0;
}

void mUiEnd(mContext* ctx) {
    if (!ctx || !ctx->ui.handle) return;
    ImGuiContext* previous = igGetCurrentContext();
    selectUi(ctx);
    if (ctx->ui.frameActive) igEndFrame();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext((ImGuiContext*)ctx->ui.handle);
    if (previous != ctx->ui.handle) igSetCurrentContext(previous);
    ctx->ui.handle = NULL;
    ctx->ui.frameActive = 0;
}

int mUiWantsMouse(mContext* ctx) {
    return selectUi(ctx) && (igGetIO_Nil()->WantCaptureMouse || ImGuizmo_IsUsingAny() || ImGuizmo_IsOver_Nil());
}
int mUiWantsKeyboard(mContext* ctx) { return selectUi(ctx) && igGetIO_Nil()->WantCaptureKeyboard; }

int mUiBeginWindow(const char* title) { return mUiBeginWindowEx(title, NULL, 0); }

int mUiBeginWindowEx(const char* title, int* open, int flags) {
    ImGuiWindowFlags native = 0;
    if (flags & mUiWindowAutoResize) native |= ImGuiWindowFlags_AlwaysAutoResize;
    if (flags & mUiWindowNoTitleBar) native |= ImGuiWindowFlags_NoTitleBar;
    if (flags & mUiWindowNoResize) native |= ImGuiWindowFlags_NoResize;
    if (flags & mUiWindowNoMove) native |= ImGuiWindowFlags_NoMove;
    if (flags & mUiWindowNoCollapse) native |= ImGuiWindowFlags_NoCollapse;
    bool nativeOpen = open && *open != 0;
    int visible = igBegin(title, open ? &nativeOpen : NULL, native);
    if (open) *open = nativeOpen ? 1 : 0;
    return visible;
}
void mUiEndWindow(void) { igEnd(); }
void mUiSetNextWindowPosition(float x, float y, int firstUseOnly) {
    igSetNextWindowPos((ImVec2){x, y}, firstUseOnly ? ImGuiCond_FirstUseEver : ImGuiCond_Always, (ImVec2){0, 0});
}
void mUiSetNextWindowSize(float width, float height, int firstUseOnly) {
    igSetNextWindowSize((ImVec2){width, height}, firstUseOnly ? ImGuiCond_FirstUseEver : ImGuiCond_Always);
}
void mUiText(const char* text) { igTextUnformatted(text ? text : "", NULL); }
void mUiTextf(const char* format, ...) {
    if (!format) return;
    va_list args;
    va_start(args, format);
    igTextV(format, args);
    va_end(args);
}
void mUiTextWrapped(const char* text) { igTextWrapped("%s", text ? text : ""); }
int mUiButton(const char* label) { return igButton(label, (ImVec2){0, 0}); }
int mUiButtonSized(const char* label, float width, float height) { return igButton(label, (ImVec2){width, height}); }
int mUiCheckbox(const char* label, int* value) {
    if (!value) return 0;
    bool nativeValue = *value != 0;
    int changed = igCheckbox(label, &nativeValue);
    *value = nativeValue ? 1 : 0;
    return changed;
}
int mUiSliderFloat(const char* label, float* value, float min, float max) {
    return value && igSliderFloat(label, value, min, max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
}
int mUiSliderInt(const char* label, int* value, int min, int max) {
    return value && igSliderInt(label, value, min, max, "%d", ImGuiSliderFlags_AlwaysClamp);
}
int mUiDragFloat(const char* label, float* value, float speed, float min, float max) {
    return value && igDragFloat(label, value, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
int mUiDragFloat2(const char* label, float values[2], float speed, float min, float max) {
    return values && igDragFloat2(label, values, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
int mUiDragFloat3(const char* label, float values[3], float speed, float min, float max) {
    return values && igDragFloat3(label, values, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
int mUiDragFloat4(const char* label, float values[4], float speed, float min, float max) {
    return values && igDragFloat4(label, values, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
int mUiInputFloat(const char* label, float* value, float step) {
    return value && igInputFloat(label, value, step, 0, "%.3f", 0);
}
int mUiInputInt(const char* label, int* value, int step) {
    return value && igInputInt(label, value, step, step, 0);
}
int mUiInputText(const char* label, char* buffer, size_t capacity) {
    return buffer && capacity && igInputText(label, buffer, capacity, 0, NULL, NULL);
}
int mUiInputTextMultiline(const char* label, char* buffer, size_t capacity, float width, float height) {
    return buffer && capacity && igInputTextMultiline(label, buffer, capacity, (ImVec2){width, height}, 0, NULL, NULL);
}
int mUiCombo(const char* label, int* selected, const char* const items[], int count) {
    if (!selected || !items || count <= 0) return 0;
    for (int i = 0; i < count; ++i) if (!items[i]) return 0;
    return igCombo_Str_arr(label, selected, items, count, -1);
}
int mUiColorEdit(const char* label, mColor* color) {
    if (!color) return 0;
    float values[] = {color->r, color->g, color->b, color->a};
    int changed = igColorEdit4(label, values, 0);
    if (changed) *color = (mColor){values[0], values[1], values[2], values[3]};
    return changed;
}
int mUiCollapsingHeader(const char* label) { return igCollapsingHeader_TreeNodeFlags(label, 0); }
void mUiSameLine(void) { igSameLine(0, -1); }
void mUiSeparator(void) { igSeparator(); }
void mUiSpacing(void) { igSpacing(); }
void mUiPushID(int id) { igPushID_Int(id); }
void mUiPopID(void) { igPopID(); }
void mUiBeginDisabled(int disabled) { igBeginDisabled(disabled); }
void mUiEndDisabled(void) { igEndDisabled(); }

int mUiGizmo(mContext* ctx, const mCamera3D* camera, int id, mat4 transform,
             mGizmoOperation operation, mGizmoMode mode) {
    if (!camera || !transform || !selectUi(ctx) || !ctx->ui.frameActive) return 0;
    if (operation < mGizmoTranslate || operation > mGizmoScale) return 0;
    ImGuiIO* io = igGetIO_Nil();
    if (io->DisplaySize.x <= 0 || io->DisplaySize.y <= 0) return 0;
    mat4 view, projection;
    mCameraGetViewMatrix(camera, view);
    mCameraGetProjectionMatrix(camera, io->DisplaySize.x / io->DisplaySize.y, projection);
    const OPERATION operations[] = {TRANSLATE, ROTATE, SCALE};
    ImGuizmo_PushID_Int(id);
    ImGuizmo_SetOrthographic(camera->projection == CAMERA_ORTHOGRAPHIC);
    ImGuizmo_SetDrawlist(igGetBackgroundDrawList_Nil());
    ImGuizmo_SetRect(0, 0, io->DisplaySize.x, io->DisplaySize.y);
    int changed = ImGuizmo_Manipulate(&view[0][0], &projection[0][0], operations[operation],
                              mode == mGizmoWorld ? WORLD : LOCAL,
                              &transform[0][0], NULL, NULL, NULL, NULL);
    ImGuizmo_PopID();
    return changed;
}

int mUiGizmoPosition(mContext* ctx, const mCamera3D* camera, int id, vec3 position) {
    if (!position) return 0;
    mat4 transform = GLM_MAT4_IDENTITY_INIT;
    glm_translate(transform, position);
    int changed = mUiGizmo(ctx, camera, id, transform, mGizmoTranslate, mGizmoWorld);
    if (changed) glm_vec3_copy(transform[3], position);
    return changed;
}
int mUiGizmoIsUsing(void) { return igGetCurrentContext() && ImGuizmo_IsUsingAny(); }
int mUiGizmoIsOver(void) { return igGetCurrentContext() && ImGuizmo_IsOver_Nil(); }

void mUiStatsPanel(mContext *ctx) {
    mUiSetNextWindowPosition(10, 10, 1);
    mUiSetNextWindowSize(150, 100, 1);
    mUiBeginWindow("Stats");
    mUiTextf("Draw Calls: %u", ctx->renderer.stats.drawCalls);
    mUiTextf("FPS: %.1f", mGetFPS(ctx));
    mUiEndWindow();
}

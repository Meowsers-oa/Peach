#include "Peach/graphics/Ui.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cimgui.h>
#include <cimgui_impl.h>
#include <cstdarg>

static bool selectUi(mContext* ctx) {
    if (!ctx || !ctx->ui.handle) return false;
    igSetCurrentContext(static_cast<ImGuiContext*>(ctx->ui.handle));
    return true;
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
    igStyleColorsDark(NULL);
    if (!ImGui_ImplGlfw_InitForOpenGL(ctx->window.handle, true)) {
        igDestroyContext(static_cast<ImGuiContext*>(ctx->ui.handle));
        ctx->ui.handle = NULL;
        igSetCurrentContext(previous);
        return 0;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 410 core")) {
        ImGui_ImplGlfw_Shutdown();
        igDestroyContext(static_cast<ImGuiContext*>(ctx->ui.handle));
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
    igDestroyContext(static_cast<ImGuiContext*>(ctx->ui.handle));
    if (previous != ctx->ui.handle) igSetCurrentContext(previous);
    ctx->ui.handle = NULL;
    ctx->ui.frameActive = 0;
}

bool mUiWantsMouse(mContext* ctx) { return selectUi(ctx) && igGetIO_Nil()->WantCaptureMouse; }
bool mUiWantsKeyboard(mContext* ctx) { return selectUi(ctx) && igGetIO_Nil()->WantCaptureKeyboard; }

bool mUiBeginWindow(const char* title, bool* open, int flags) {
    ImGuiWindowFlags native = 0;
    if (flags & mUiWindowAutoResize) native |= ImGuiWindowFlags_AlwaysAutoResize;
    if (flags & mUiWindowNoTitleBar) native |= ImGuiWindowFlags_NoTitleBar;
    if (flags & mUiWindowNoResize) native |= ImGuiWindowFlags_NoResize;
    if (flags & mUiWindowNoMove) native |= ImGuiWindowFlags_NoMove;
    if (flags & mUiWindowNoCollapse) native |= ImGuiWindowFlags_NoCollapse;
    return igBegin(title, open, native);
}
void mUiEndWindow(void) { igEnd(); }
void mUiSetNextWindowPosition(float x, float y, bool firstUseOnly) {
    igSetNextWindowPos(ImVec2(x, y), firstUseOnly ? ImGuiCond_FirstUseEver : ImGuiCond_Always, ImVec2(0, 0));
}
void mUiSetNextWindowSize(float width, float height, bool firstUseOnly) {
    igSetNextWindowSize(ImVec2(width, height), firstUseOnly ? ImGuiCond_FirstUseEver : ImGuiCond_Always);
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
bool mUiButton(const char* label) { return igButton(label, ImVec2(0, 0)); }
bool mUiButtonSized(const char* label, float width, float height) { return igButton(label, ImVec2(width, height)); }
bool mUiCheckbox(const char* label, bool* value) { return value && igCheckbox(label, value); }
bool mUiSliderFloat(const char* label, float* value, float min, float max) {
    return value && igSliderFloat(label, value, min, max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
}
bool mUiSliderInt(const char* label, int* value, int min, int max) {
    return value && igSliderInt(label, value, min, max, "%d", ImGuiSliderFlags_AlwaysClamp);
}
bool mUiDragFloat(const char* label, float* value, float speed, float min, float max) {
    return value && igDragFloat(label, value, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
bool mUiDragFloat2(const char* label, float values[2], float speed, float min, float max) {
    return values && igDragFloat2(label, values, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
bool mUiDragFloat3(const char* label, float values[3], float speed, float min, float max) {
    return values && igDragFloat3(label, values, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
bool mUiDragFloat4(const char* label, float values[4], float speed, float min, float max) {
    return values && igDragFloat4(label, values, speed, min, max, "%.3f", min < max ? ImGuiSliderFlags_AlwaysClamp : 0);
}
bool mUiInputFloat(const char* label, float* value, float step) {
    return value && igInputFloat(label, value, step, 0, "%.3f", 0);
}
bool mUiInputInt(const char* label, int* value, int step) {
    return value && igInputInt(label, value, step, step, 0);
}
bool mUiInputText(const char* label, char* buffer, size_t capacity) {
    return buffer && capacity && igInputText(label, buffer, capacity, 0, NULL, NULL);
}
bool mUiInputTextMultiline(const char* label, char* buffer, size_t capacity, float width, float height) {
    return buffer && capacity && igInputTextMultiline(label, buffer, capacity, ImVec2(width, height), 0, NULL, NULL);
}
bool mUiCombo(const char* label, int* selected, const char* const items[], int count) {
    if (!selected || !items || count <= 0) return false;
    for (int i = 0; i < count; ++i) if (!items[i]) return false;
    return igCombo_Str_arr(label, selected, items, count, -1);
}
bool mUiColorEdit(const char* label, mColor* color) {
    if (!color) return false;
    float values[] = {color->r, color->g, color->b, color->a};
    bool changed = igColorEdit4(label, values, 0);
    if (changed) *color = {values[0], values[1], values[2], values[3]};
    return changed;
}
bool mUiCollapsingHeader(const char* label) { return igCollapsingHeader_TreeNodeFlags(label, 0); }
void mUiSameLine(void) { igSameLine(0, -1); }
void mUiSeparator(void) { igSeparator(); }
void mUiSpacing(void) { igSpacing(); }
void mUiPushID(int id) { igPushID_Int(id); }
void mUiPopID(void) { igPopID(); }
void mUiBeginDisabled(bool disabled) { igBeginDisabled(disabled); }
void mUiEndDisabled(void) { igEndDisabled(); }

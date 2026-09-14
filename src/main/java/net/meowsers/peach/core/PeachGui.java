package net.meowsers.peach.core;

import imgui.ImGui;
import imgui.gl3.ImGuiImplGl3;
import imgui.glfw.ImGuiImplGlfw;

/** Empty Dear ImGui frame; levels may add widgets during update if desired. */
public class PeachGui {
    private ImGuiImplGlfw glfw;
    private ImGuiImplGl3 gl3;
    private boolean context;

    public void init(long windowHandle) {
        if (context) throw new IllegalStateException("GUI already initialized");
        ImGui.createContext();
        context = true;
        try {
            ImGui.getIO().setIniFilename(null);
            glfw = new ImGuiImplGlfw();
            if (!glfw.init(windowHandle, true)) throw new IllegalStateException("ImGui GLFW initialization failed");
            gl3 = new ImGuiImplGl3();
            if (!gl3.init("#version 410 core")) throw new IllegalStateException("ImGui OpenGL initialization failed");
        } catch (RuntimeException | Error e) { shutdown(); throw e; }
    }

    public void newFrame() {
        gl3.newFrame();
        glfw.newFrame();
        ImGui.newFrame();
    }

    public void render() {
        ImGui.render();
        gl3.renderDrawData(ImGui.getDrawData());
    }

    public void shutdown() {
        if (gl3 != null) gl3.shutdown();
        if (glfw != null) glfw.shutdown();
        if (context) ImGui.destroyContext();
        gl3 = null; glfw = null; context = false;
    }
}

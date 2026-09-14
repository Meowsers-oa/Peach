package net.meowsers.peach.core;

import net.meowsers.peach.structures.Color;
import net.meowsers.peach.structures.WindowParams;
import net.meowsers.peach.utils.Log;
import org.lwjgl.opengl.GL;
import org.lwjgl.glfw.Callbacks;
import org.lwjgl.system.MemoryStack;

import java.nio.IntBuffer;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;

public class Window {

    private long handle;
    private boolean running;
    private int framebufferWidth, framebufferHeight;

    private String lastTitle = WindowParams.title;

    private Color clearColor = Color.White;


    public void create() {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

        handle = glfwCreateWindow(WindowParams.width, WindowParams.height, WindowParams.title, 0, 0);
        if (handle == 0) {
            Log.fatalGlfw();
        }

        glfwSetFramebufferSizeCallback(handle, this::onFramebufferResize);

        running = true;
    }
    public void use() {
        glfwMakeContextCurrent(handle);
        GL.createCapabilities();
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer fWidth = stack.mallocInt(1);
            IntBuffer fHeight = stack.mallocInt(1);
            glfwGetFramebufferSize(handle, fWidth, fHeight);
            framebufferWidth = fWidth.get(0);
            framebufferHeight = fHeight.get(0);
            glViewport(0, 0, framebufferWidth, framebufferHeight);
        }
        glfwSwapInterval(0);
        glfwShowWindow(handle);
    }
    public void update() {
        if (!WindowParams.title.equals(lastTitle)) {
            lastTitle = WindowParams.title;
            glfwSetWindowTitle(handle, lastTitle);
        }
        glfwPollEvents();
        running = !glfwWindowShouldClose(handle);
    }

    public void present() {
        glfwSwapBuffers(handle);
    }

    public void end() {
        if (handle != 0) {
            Callbacks.glfwFreeCallbacks(handle);
            glfwDestroyWindow(handle);
            handle = 0;
        }
        running = false;
        GL.setCapabilities(null);
    }

    public void stop() {
        glfwSetWindowShouldClose(handle, true);
    }

    public void updateProperties() {
        lastTitle = WindowParams.title;
        glfwSetWindowSize(handle, WindowParams.width, WindowParams.height);
        glfwSetWindowTitle(handle, lastTitle);
    }

    private void onFramebufferResize(long handle, int width, int height) {
        framebufferWidth = width;
        framebufferHeight = height;
    }



    public int getFramebufferWidth() { return framebufferWidth; }
    public int getFramebufferHeight() { return framebufferHeight; }

    public long getHandle() {
        return handle;
    }
    public boolean isRunning() {
        return running;
    }
    public Color getClearColor() {
        return clearColor;
    }
    public void setClearColor(Color clearColor) {
        this.clearColor = clearColor;
    }
}
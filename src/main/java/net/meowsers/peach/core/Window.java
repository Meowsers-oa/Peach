package net.meowsers.peach.core;

import net.meowsers.peach.structures.Color;
import net.meowsers.peach.structures.WindowParams;
import net.meowsers.peach.utils.Log;
import org.lwjgl.BufferUtils;
import org.lwjgl.opengl.GL;
import org.lwjgl.glfw.GLFWFramebufferSizeCallback;

import java.nio.IntBuffer;
import java.util.Objects;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;

public class Window {
    private long handle;
    private boolean running;
    private Color lastclearColor;
    private String lastTitle;

    public void start() {
        handle = glfwCreateWindow(WindowParams.width, WindowParams.height, WindowParams.title, 0, 0);
        if(handle == 0) {Log.fatalGlfw();}
        running = true;
        lastTitle = WindowParams.title;
        glfwMakeContextCurrent(handle);
        GL.createCapabilities();
        IntBuffer fWidth = BufferUtils.createIntBuffer(1);
        IntBuffer fHeight = BufferUtils.createIntBuffer(1);
        glfwGetFramebufferSize(handle, fWidth, fHeight);
        glViewport(0, 0, fWidth.get(0), fHeight.get(0));
        WindowParams.width = fWidth.get(0);
        WindowParams.height = fHeight.get(0);
        glfwSetFramebufferSizeCallback(handle, this::onResize);
        glfwSwapInterval(0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    public void update() {
        if(!Objects.equals(WindowParams.title, lastTitle)) {
            lastTitle = WindowParams.title;
            glfwSetWindowTitle(handle, lastTitle);
        }
        if(!WindowParams.clearColor.equals(lastclearColor)) {
            lastclearColor = WindowParams.clearColor;
        }
        glfwPollEvents();
        running = !glfwWindowShouldClose(handle);
        clear();
    }

    public void present() {
        glfwSwapBuffers(handle);
    }

    public void end() {
        if (handle == 0) return;
        GLFWFramebufferSizeCallback callback = glfwSetFramebufferSizeCallback(handle, null);
        if (callback != null) callback.free();
        glfwDestroyWindow(handle);
        handle = 0;
        running = false;
        GL.setCapabilities(null);
    }

    private void clear() {
        glClearColor(lastclearColor.r, lastclearColor.g, lastclearColor.b, lastclearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    public void stop() {
        glfwSetWindowShouldClose(handle, true);
    }

    private void onResize(long handle, int width, int height) {
        glViewport(0, 0, width, height);
        WindowParams.width = width;
        WindowParams.height = height;
    }

    public long getHandle() {
        return handle;
    }
    public boolean isRunning() {
        return running;
    }

}

package net.meowsers.peach.core;

import net.meowsers.peach.rendering.Renderer;
import net.meowsers.peach.utils.Input;
import net.meowsers.peach.utils.Log;
import net.meowsers.peach.utils.Time;

import static org.lwjgl.glfw.GLFW.*;

public class Peach {
    private Window window;
    private PeachGame game;

    private float deltaTime;

    public void run() {
        try {
            start();
            update();
        } finally {
            end();
        }
    }

    private void start() {
        initGlfw();
        window = new Window();
        window.start();
        Time.start();
        Input.start(window.getHandle());
        Renderer.start();

        game.start();
    }

    private void update() {
        while(window.isRunning()) {
            window.update();
            if (!window.isRunning()) break;
            Time.update();
            deltaTime = Time.getDeltaTime();
            game.update(deltaTime);
            Renderer.flush();
            window.present();

            Input.endFrame();
        }
    }

    private void end() {
        try {
            if (game != null) game.end();
        } finally {
            Renderer.end();
            Input.cleanup();
            if (window != null) window.end();
            glfwTerminate();
        }
    }

    private void initGlfw() {
        if(!glfwInit()) { Log.fatalGlfw(); }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    }

    public PeachGame getGame() {
        return game;
    }
    public void setGame(PeachGame game) {
        this.game = game;
    }
}

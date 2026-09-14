package net.meowsers.peach.core;

import net.meowsers.peach.rendering.Renderer;
import net.meowsers.peach.utils.Log;

import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.glfw.GLFW.*;

public class Peach {
    private final Window window = new Window();
    private final PeachGui gui = new PeachGui();
    private PeachProgram program = new PeachProgram() { };
    private float deltaTime;
    private boolean running;

    public void start(PeachLevel... initialLevels) {
        PeachProgram game = new PeachProgram() { };
        for (PeachLevel level : initialLevels) game.addLevel(level);
        start(game);
    }

    public void start(PeachProgram program) {
        if (running) throw new IllegalStateException("Peach is already running");
        this.program = java.util.Objects.requireNonNull(program);
        program.attach(this);
        if (!glfwInit()) Log.fatalGlfw();
        running = true;
        Throwable failure = null;
        try {
            window.create();
            window.use();
            Input.start(window.getHandle());
            Renderer.init();
            gui.init(window.getHandle());
            program.start();
            Time.start();
            while (window.isRunning()) {
                window.update();
                if (!window.isRunning()) break;
                Time.update();
                deltaTime = Time.getDeltaTime();
                if (window.getFramebufferWidth() > 0 && window.getFramebufferHeight() > 0) {
                    Renderer.beginFrame(window.getFramebufferWidth(), window.getFramebufferHeight(), window.getClearColor());
                    gui.newFrame();
                    program.update(deltaTime);
                    Renderer.endFrame();
                    gui.render();
                    window.present();
                } else {
                    glfwWaitEventsTimeout(0.05);
                }
                Input.endFrame();
            }
        } catch (RuntimeException | Error e) { failure = e; throw e; }
        finally {
            running = false;
            try { end(); }
            catch (RuntimeException | Error cleanup) {
                if (failure != null) failure.addSuppressed(cleanup); else throw cleanup;
            }
        }
    }

    public void addLevel(PeachLevel level) { program.addLevel(level); }
    public void removeLevel(PeachLevel level) { program.removeLevel(level); }

    private void end() {
        // Cleanup continues even if a level's end hook fails. The context outlives all GL objects.
        List<Runnable> cleanup = new ArrayList<>();
        cleanup.add(program::end);
        cleanup.add(Renderer::dispose);
        cleanup.add(gui::shutdown);
        cleanup.add(Input::cleanup);
        cleanup.add(window::end);
        cleanup.add(() -> glfwTerminate());
        Throwable failure = null;
        for (Runnable action : cleanup) {
            try { action.run(); }
            catch (RuntimeException | Error e) {
                if (failure == null) failure = e; else failure.addSuppressed(e);
            }
        }
        if (failure instanceof Error error) throw error;
        if (failure instanceof RuntimeException exception) throw exception;
    }

    public List<PeachLevel> getLevels() { return program.getLevels(); }
    public Window getWindow() { return window; }
    public float getDeltaTime() { return deltaTime; }
    public void stop() { window.stop(); }
}

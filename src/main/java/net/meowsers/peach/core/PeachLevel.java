package net.meowsers.peach.core;

public abstract class PeachLevel {

    private Window window;

    public void start() {

    }
    public void update(float dt) {

    }
    public void end() {

    }

    public Window getWindow() {
        return window;
    }
    public void setWindow(Window window) {
        this.window = window;
    }
}

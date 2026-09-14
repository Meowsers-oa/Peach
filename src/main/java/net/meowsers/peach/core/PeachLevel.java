package net.meowsers.peach.core;

/** Override the hooks your level needs. No separate thread or while-loop is needed. */
public abstract class PeachLevel {
    private Peach peach;

    final void attach(Peach peach) { this.peach = peach; }
    protected final Peach getPeach() {
        if (peach == null) throw new IllegalStateException("Level is not attached to a running game");
        return peach;
    }
    protected final Window getWindow() { return getPeach().getWindow(); }

    public void start() { }
    public void update(float deltaTime) { }
    public void end() { }
}

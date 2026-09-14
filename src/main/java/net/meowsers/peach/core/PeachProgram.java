package net.meowsers.peach.core;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/** A game owns levels. The engine drives this hierarchy once per frame. */
public abstract class PeachProgram {
    private final List<PeachLevel> levels = new ArrayList<>();
    private final List<PeachLevel> pendingAdd = new ArrayList<>(), pendingRemove = new ArrayList<>();
    private boolean running;
    private Peach peach;

    public final void addLevel(PeachLevel level) {
        Objects.requireNonNull(level, "level");
        pendingRemove.remove(level);
        if (!levels.contains(level) && !pendingAdd.contains(level)) pendingAdd.add(level);
    }

    public final void removeLevel(PeachLevel level) {
        pendingAdd.remove(level);
        if (levels.contains(level) && !pendingRemove.contains(level)) pendingRemove.add(level);
    }

    public final List<PeachLevel> getLevels() { return List.copyOf(levels); }

    final void attach(Peach peach) { this.peach = peach; }

    public final void start() {
        if (running) throw new IllegalStateException("Game is already running");
        running = true;
        applyChanges();
    }

    public final void update(float dt) {
        if (!running) throw new IllegalStateException("Game has not started");
        applyChanges();
        for (PeachLevel level : List.copyOf(levels)) level.update(dt);
    }

    private void applyChanges() {
        List<PeachLevel> additions = List.copyOf(pendingAdd), removals = List.copyOf(pendingRemove);
        pendingAdd.clear(); pendingRemove.clear();
        for (PeachLevel level : removals) {
            if (levels.remove(level)) {
                try { level.end(); } finally { level.attach(null); }
            }
        }
        for (PeachLevel level : additions) {
            levels.add(level); // Include partially started levels in exception cleanup.
            level.attach(peach);
            level.start();
        }
    }

    public final void end() {
        Throwable failure = null;
        for (PeachLevel level : levels.reversed()) {
            try { level.end(); }
            catch (RuntimeException | Error e) {
                if (failure == null) failure = e; else failure.addSuppressed(e);
            } finally { level.attach(null); }
        }
        levels.clear(); pendingAdd.clear(); pendingRemove.clear();
        running = false; peach = null;
        if (failure instanceof Error error) throw error;
        if (failure instanceof RuntimeException exception) throw exception;
    }
}

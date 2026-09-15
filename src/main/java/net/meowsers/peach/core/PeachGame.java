package net.meowsers.peach.core;

import java.util.ArrayList;
import java.util.List;

public abstract class PeachGame {

    private List<PeachLevel> levels = new ArrayList<>();
    private Window window;

    public void start() {

    }
    public void update(float dt) {

    }
    public void end() {

    }

    public List<PeachLevel> getLevels() {
        return levels;
    }

    public void startLevels() {
        if(!levels.isEmpty()) {
            levels.forEach(PeachLevel::start);
        }
    }
    public void updateLevels(float dt) {
        if(!levels.isEmpty()) {
            levels.forEach(peachLevel -> peachLevel.update(dt));
        }
    }
    public void endLevels() {
        if(!levels.isEmpty()) {
            levels.forEach(PeachLevel::end);
        }
    }

    public void setWindowInLevels() {
        if(!levels.isEmpty()) {
            levels.forEach(peachLevel -> peachLevel.setWindow(window));
        }
    }

    public void addLevel(PeachLevel level) {
        if(!levels.contains(level)) levels.add(level);
    }

    public void removeLevel(PeachLevel level) {
        levels.remove(level);
    }

    public void setWindow(Window window) {
        this.window = window;
    }
}

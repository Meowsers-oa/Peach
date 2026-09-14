package net.meowsers;

import net.meowsers.peach.core.PeachProgram;

public class Game extends PeachProgram {
    public Game() {
        addLevel(new MyLevel());
    }
}

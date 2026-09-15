package net.meowsers;

import net.meowsers.peach.core.PeachGame;

public class MyGame extends PeachGame {

    @Override
    public void start() {
        addLevel(new MyLevel());
    }
}

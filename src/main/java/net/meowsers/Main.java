package net.meowsers;

import net.meowsers.peach.core.Peach;

public abstract class Main {
    public static void main(String[] args) {

        Peach peach = new Peach();
        peach.start(new Game());

    }
}

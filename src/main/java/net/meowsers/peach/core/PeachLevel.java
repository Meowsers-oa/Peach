package net.meowsers.peach.core;

import net.meowsers.peach.scene.GameObject;

import java.util.ArrayList;
import java.util.List;

public abstract class PeachLevel {
    private Window window;
    private List<GameObject> gameObjects = new ArrayList<>();

    public void start() {
        startGameObjects();

    }
    public void update(float dt) {
        updateGameObjects(dt);

    }
    public void end() {
        endGameObjects();

    }

    private void startGameObjects() {
        if(!gameObjects.isEmpty()) {
            for(GameObject go : gameObjects) {
                go.start();
            }
        }
    }
    private void updateGameObjects(float dt) {
        if(!gameObjects.isEmpty()) {
            for(GameObject go : gameObjects) {
                go.update(dt);
            }
        }
    }
    private void endGameObjects() {
        if(!gameObjects.isEmpty()) {
            for(GameObject go : gameObjects) {
                go.end();
            }
        }
    }

    public void addGameObject(GameObject go) {
        if(!gameObjects.contains(go)) {
            gameObjects.add(go);
        }
    }



    public void addGameObjects(GameObject... gameObjects) {
        for(GameObject go : gameObjects) {
            addGameObject(go);
        }
    }

    public void removeGame(GameObject go) {
        gameObjects.remove(go);
    }

    public Window getWindow() {
        return window;
    }
    public void setWindow(Window window) {
        this.window = window;
    }
}

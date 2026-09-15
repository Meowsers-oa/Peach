package net.meowsers.peach.scene;

public abstract class Component {

    private GameObject parent = new GameObject();

    public void onAttach() {

    }
    public void update(float dt) {

    }
    public void onDetach() {

    }

    public GameObject getParent() {
        return parent;
    }
    public void setParent(GameObject parent) {
        this.parent = parent;
    }
}

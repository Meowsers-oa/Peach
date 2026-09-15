package net.meowsers.peach.scene;

import net.meowsers.peach.core.Peach;
import net.meowsers.peach.structures.Transform;

import java.util.ArrayList;
import java.util.List;

public class GameObject {

    private List<Component> components = new ArrayList<>();
    private String name;
    public Transform transform = new Transform();

    public GameObject(String name) {
        this.name = name;
    }

    public GameObject() {
        this.name = "GameObject";
    }

    public void start() {
        attachComponents();

    }
    public void update(float dt) {
        updateComponents(dt);

    }
    public void end() {
        detachComponents();

    }

    public boolean hasComponent(Class<? extends Component> componentClass) {
        return components.stream().anyMatch(componentClass::isInstance);
    }

    @SuppressWarnings("unchecked")
    public <T extends Component> T getComponent(Class<T> componentClass) {
        if (componentClass == null) return null;
        for (Component c : components) {
            if (componentClass.isInstance(c)) {
                return (T) c;
            }
        }
        return null;
    }
    public void addComponent(Component component) {
        if(!components.contains(component)) {
            components.add(component);
        }
    }
    public void removeComponent(Component component) {
        components.remove(component);
    }

    public void attachComponents() {
        if(!components.isEmpty()) {
            for(Component c : components) {
                c.setParent(this);
                c.onAttach();
            }
        }
    }
    public void updateComponents(float dt) {
        if(!components.isEmpty()) {
            for(Component c : components) {
                c.update(dt);
            }
        }
    }
    public void detachComponents() {
        if(!components.isEmpty()) {
            for(Component c : components) {
                c.onDetach();
            }
        }
    }

    public List<Component> getComponents() {
        return components;
    }
}

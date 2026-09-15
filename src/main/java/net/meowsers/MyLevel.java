package net.meowsers;

import net.meowsers.peach.core.PeachLevel;
import net.meowsers.peach.rendering.Camera;
import net.meowsers.peach.rendering.Renderer;
import net.meowsers.peach.scene.GameObject;
import net.meowsers.peach.scene.components.CameraComponent;
import net.meowsers.peach.scene.components.MeshRendererComponent;
import net.meowsers.peach.structures.Mesh;
import net.meowsers.peach.structures.WindowParams;
import net.meowsers.peach.utils.Model;
import org.joml.Vector3f;

public class MyLevel extends PeachLevel {
    GameObject cam = new GameObject("Camera");
    GameObject yoshi = new GameObject("Yoshi");

    @Override
    public void start() {
        yoshi.addComponent(new MeshRendererComponent(Model.loadModel("src/main/resources/models/yoshi.glb", false)));
        yoshi.transform.scale.set(0.05f);

        cam.addComponent(new CameraComponent());

        addGameObjects(yoshi, cam);
        super.start();
    }

    @Override
    public void update(float dt) {
        super.update(dt);
    }

    @Override
    public void end() {
        super.end();
    }
}

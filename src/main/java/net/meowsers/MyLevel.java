package net.meowsers;

import net.meowsers.peach.core.Camera;
import net.meowsers.peach.core.Input;
import net.meowsers.peach.core.PeachLevel;
import net.meowsers.peach.rendering.Model;
import net.meowsers.peach.rendering.Renderer;
import net.meowsers.peach.rendering.post.BloomPass;
import net.meowsers.peach.rendering.post.ToneMapPass;
import net.meowsers.peach.structures.Color;
import net.meowsers.peach.structures.Key;
import net.meowsers.peach.structures.Light;
import net.meowsers.peach.structures.LightType;
import net.meowsers.peach.structures.WindowParams;
import org.joml.Vector3f;

/** The user's scene: a lit cube, a ground plane, and a glowing spotlight. */
public class MyLevel extends PeachLevel {
    private Model plane, cube, lamp;
    private Light light;
    private BloomPass bloom;
    private ToneMapPass toneMap;
    private final Vector3f focus = new Vector3f(0, 1, 0);

    @Override public void start() {
        WindowParams.title = "Peach Engine | Slang, shadows & bloom | RMB + WASD/QE: fly | Shift: fast | Space: pause | Esc: quit";
        getWindow().setClearColor(new Color(0.012f, 0.018f, 0.03f));
        plane = Model.fromResource("/models/plane.obj");
        cube = Model.fromResource("/models/cube.obj");
        lamp = Model.fromResource("/models/cube.obj");
        cube.transform.position.set(0, 1, 0);
        cube.transform.rotation.y = 20;
        lamp.transform.scale.set(0.18f);
        plane.getMeshes().forEach(mesh -> mesh.material.color.set(0.24f, 0.30f, 0.37f, 1));
        cube.getMeshes().forEach(mesh -> mesh.material.color.set(0.82f, 0.26f, 0.09f, 1));
        lamp.getMeshes().forEach(mesh -> {
            mesh.material.color.set(0, 0, 0, 1);
            mesh.material.emission.set(14, 10, 5);
            mesh.material.castsShadow = false;
        });
        light = new Light(LightType.SPOT);
        light.position.set(-3.5f, 6, 2.5f);
        light.direction.set(0, 0, 0).sub(light.position).normalize();
        light.color.set(1, 0.84f, 0.62f);
        light.intensity = 7;
        light.innerCutoff = 38; light.outerCutoff = 55;
        light.constant = 1; light.linear = 0.015f; light.quadratic = 0.012f;
        light.castsShadows = true;
        Renderer.addLight(light);
        Renderer.ambient.set(0.10f, 0.13f, 0.18f);
        bloom = new BloomPass(12);
        bloom.threshold = 1; bloom.intensity = 0.35f;
        toneMap = new ToneMapPass();
        toneMap.exposure = 1;
        Renderer.addPostProcessing(bloom);
        Renderer.addPostProcessing(toneMap);
        Camera camera = Renderer.getCamera();
        camera.position.set(7.73f, 7.5f, 9.18f);
        camera.fov = 52;
        camera.lookAt(focus);
    }

    private boolean paused;
    @Override public void update(float dt) {
        if (Input.isKeyPressed(Key.ESCAPE)) getPeach().stop();
        if (Input.isKeyPressed(Key.SPACE)) paused = !paused;
        if (!paused) cube.transform.rotation.y += dt * 12.6f;
        Renderer.getCamera().handleCameraMovement();
        Renderer.addModel(plane);
        lamp.transform.position.set(light.position);
        Renderer.addModel(cube);
        Renderer.addModel(lamp);
    }

    @Override public void end() {
        Renderer.getCamera().releaseCameraMovement();
        if (plane != null) plane.dispose();
        if (cube != null) cube.dispose();
        if (lamp != null) lamp.dispose();
        if (light != null) Renderer.removeLight(light);
        if (bloom != null) Renderer.removePostProcessing(bloom);
        if (toneMap != null) Renderer.removePostProcessing(toneMap);
    }
}

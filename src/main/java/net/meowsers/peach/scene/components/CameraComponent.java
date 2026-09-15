package net.meowsers.peach.scene.components;

import net.meowsers.peach.rendering.Camera;
import net.meowsers.peach.scene.Component;
import net.meowsers.peach.structures.WindowParams;

public class CameraComponent extends Component {
    private Camera camera;
    private float fov = 80, near = 0.01f, far = 100f, aspectRatio = (float) WindowParams.width / WindowParams.height;

    private boolean handleMovement = true;
    private float moveSpeed = 10, mouseSensitivity = .1f;

    public CameraComponent() {
        camera = new Camera(aspectRatio, far, near, fov);
        camera.moveSpeed = moveSpeed;
        camera.mouseSensitivity = mouseSensitivity;
    }

    @Override
    public void update(float dt) {
        super.update(dt);
        if(handleMovement) camera.handleCameraMovement(dt);
    }

    public float getFov() {
        return fov;
    }
    public void setFov(float fov) {
        this.fov = fov;
        camera.setFov(fov);
    }
    public float getNear() {
        return near;
    }
    public void setNear(float near) {
        this.near = near;
        camera.setNearPlane(near);
    }
    public float getFar() {
        return far;
    }
    public void setFar(float far) {
        this.far = far;
        camera.setFarPlane(far);
    }
    public float getAspectRatio() {
        return aspectRatio;
    }
    public void setAspectRatio(float aspectRatio) {
        this.aspectRatio = aspectRatio;
        camera.setAspectRatio(aspectRatio);
    }

    public void setMoveSpeed(float moveSpeed) {camera.moveSpeed = moveSpeed;}
    public void mouseSensitivity(float mouseSensitivity) {camera.mouseSensitivity = mouseSensitivity;}
}

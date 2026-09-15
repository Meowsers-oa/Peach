package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Transform;
import net.meowsers.peach.structures.Key;
import net.meowsers.peach.structures.MouseButton;
import net.meowsers.peach.utils.Input;
import org.joml.Matrix4f;
import org.joml.Vector3f;

public class Camera {
    public Transform transform = new Transform(new Vector3f(0, 0, -3), new Vector3f(1), new Vector3f(0, 180, 0));
    public float moveSpeed = 10.f, mouseSensitivity = 0.15f, sprintMultiplier = 3.f;
    private Matrix4f projection = new Matrix4f(), view = new Matrix4f();
    private Vector3f up = new Vector3f(0, 1, 0), right = new Vector3f(), forward = new Vector3f();
    private Vector3f lookingAt = new Vector3f(0, 0, 0);
    private float fov, nearPlane, farPlane, aspectRatio;
    private final Vector3f movement = new Vector3f();
    private double lastMouseX, lastMouseY;
    private boolean looking, cursorWasLocked;

    public Camera(float aspectRatio, float farPlane, float nearPlane, float fov) {
        this.aspectRatio = aspectRatio;
        this.farPlane = farPlane;
        this.nearPlane = nearPlane;
        this.fov = (float) Math.toRadians(fov);

        updateDirections();
        updateView();
    }


    public void handleCameraMovement(float dt) {
        boolean look = Input.isMouseButtonDown(MouseButton.RIGHT);
        if (look && !looking) {
            cursorWasLocked = Input.isCursorLocked();
            Input.lockCursor();
            lastMouseX = Input.getMouseX();
            lastMouseY = Input.getMouseY();
        } else if (!look && looking) {
            Input.setCursorLocked(cursorWasLocked);
        }
        looking = look;

        if (looking) {
            double mouseX = Input.getMouseX(), mouseY = Input.getMouseY();
            transform.rotation.y -= (float) (mouseX - lastMouseX) * mouseSensitivity;
            transform.rotation.x -= (float) (mouseY - lastMouseY) * mouseSensitivity;
            transform.rotation.x = Math.max(-89.f, Math.min(89.f, transform.rotation.x));
            transform.rotation.y %= 360.f;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        updateDirections();
        if (looking) {
            movement.zero();
            if (Input.isKeyDown(Key.W)) movement.add(forward);
            if (Input.isKeyDown(Key.S)) movement.sub(forward);
            if (Input.isKeyDown(Key.D)) movement.add(right);
            if (Input.isKeyDown(Key.A)) movement.sub(right);
            if (Input.isKeyDown(Key.E)) movement.add(up);
            if (Input.isKeyDown(Key.Q)) movement.sub(up);
            if (movement.lengthSquared() > 0 && Float.isFinite(dt) && dt > 0) {
                float speed = moveSpeed * (Input.isShiftPressed() ? sprintMultiplier : 1.f);
                transform.position.fma(speed * dt, movement.normalize());
            }
        }
        updateView();
    }

    private void updateDirections() {
        float pitch = (float) Math.toRadians(transform.rotation.x);
        float yaw = (float) Math.toRadians(transform.rotation.y);
        forward.set(0, 0, -1).rotateX(pitch).rotateY(yaw);
        forward.cross(up, right).normalize();
    }

    private void updateView() {
        lookingAt.set(transform.position).add(forward);
        view.setLookAt(transform.position, lookingAt, up);
        projection.setPerspective(fov, aspectRatio, nearPlane, farPlane);
        Renderer.setCamera(view, projection);
    }

    public Matrix4f getProjection() {
        return projection;
    }
    public Matrix4f getView() {
        return view;
    }
    public Vector3f getUp() {
        return up;
    }
    public Vector3f getRight() {
        return right;
    }
    public Vector3f getForward() {
        return forward;
    }
    public Vector3f getLookingAt() {
        return lookingAt;
    }
    public float getAspectRatio() {
        return aspectRatio;
    }
    public void setAspectRatio(float aspectRatio) {
        this.aspectRatio = aspectRatio;
    }
    public float getFarPlane() {
        return farPlane;
    }
    public void setFarPlane(float farPlane) {
        this.farPlane = farPlane;
    }
    public float getNearPlane() {
        return nearPlane;
    }
    public void setNearPlane(float nearPlane) {
        this.nearPlane = nearPlane;
    }
    public float getFov() {
        return fov;
    }
    public void setFov(float fov) {
        this.fov = fov;
    }



}

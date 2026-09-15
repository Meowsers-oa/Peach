package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Transform;
import org.joml.Matrix4f;
import org.joml.Vector3f;

public class Camera {
    public Transform transform = new Transform(new Vector3f(0, 0, -3), new Vector3f(0), new Vector3f(1));
    private Matrix4f projection = new Matrix4f(), view = new Matrix4f();
    private Vector3f up = new Vector3f(0, 1, 0), right, forward;
    private Vector3f lookingAt = new Vector3f(0, 0, 0);
    private float fov, nearPlane, farPlane, aspectRatio;

    public Camera(float aspectRatio, float farPlane, float nearPlane, float fov) {
        this.aspectRatio = aspectRatio;
        this.farPlane = farPlane;
        this.nearPlane = nearPlane;
        this.fov = (float) Math.toRadians(fov);

        projection.perspective(fov, aspectRatio, nearPlane, farPlane);
        view.lookAt(transform.position, lookingAt, up);
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

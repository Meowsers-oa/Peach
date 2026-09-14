package net.meowsers.peach.core;

import net.meowsers.peach.structures.Key;
import net.meowsers.peach.structures.MouseButton;
import org.lwjgl.system.MemoryStack;
import org.joml.FrustumIntersection;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;

import static org.lwjgl.glfw.GLFW.*;

public class Camera {
    public final Vector3f position = new Vector3f(0, 0, 3);
    // Degrees. Zero rotation looks down -Z; positive yaw turns toward -X.
    public float pitch, yaw, roll;
    public float fov = 60, near = 0.1f, far = 1000;
    public float aspect = 1;

    public float movementSpeed = 5;
    public float mouseSensitivity = 0.15f;
    public float fastMultiplier = 3;
    public float panSensitivity = 0.01f;
    public float scrollSensitivity = 1;
    private long movementWindow;
    private boolean restoreCursorLock;
    private double previousMouseX, previousMouseY;

    /**
     * Call once per update. Hold RMB to look and fly with WASD, Q/E down/up,
     * Shift to move faster. MMB drags the view; scroll dollies, or adjusts fly
     * speed while RMB is held. Mouse motion is independent of frame time.
     */
    public void handleCameraMovement() {
        handleCameraMovement(Time.getDeltaTime());
    }

    public void handleCameraMovement(float deltaTime) {
        if (!Float.isFinite(deltaTime) || deltaTime < 0) throw new IllegalArgumentException("Invalid camera delta time");
        long window = Input.getWindowHandle();
        if (window == 0 || glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_FALSE) {
            releaseCameraMovement();
            return;
        }
        boolean flying = Input.isMouseButtonDown(MouseButton.RIGHT);
        boolean panning = Input.isMouseButtonDown(MouseButton.MIDDLE);
        if (flying || panning) {
            boolean firstFrame = movementWindow != window || !Input.isCursorLocked()
                    || Input.isMouseButtonClicked(flying ? MouseButton.RIGHT : MouseButton.MIDDLE);
            if (movementWindow != window) {
                releaseCameraMovement();
                restoreCursorLock = Input.isCursorLocked();
                movementWindow = window;
                Input.lockCursor();
            } else if (!Input.isCursorLocked()) Input.lockCursor();
            // Read after capture: GLFW can change cursor coordinates when its mode changes.
            try (MemoryStack stack = MemoryStack.stackPush()) {
                var x = stack.mallocDouble(1);
                var y = stack.mallocDouble(1);
                glfwGetCursorPos(window, x, y);
                float dx = firstFrame ? 0 : (float) (x.get(0) - previousMouseX);
                float dy = firstFrame ? 0 : (float) (y.get(0) - previousMouseY);
                previousMouseX = x.get(0); previousMouseY = y.get(0);
                if (flying) rotateCamera(dx, dy);
                else position.add(getViewMatrix().invert().transformDirection(new Vector3f(-dx, dy, 0)).mul(panSensitivity));
            }
        } else releaseCameraMovement();

        float scroll = (float) Input.getMouseScrollY();
        if (flying) {
            if (scroll != 0) movementSpeed = Math.clamp(movementSpeed * (float) Math.pow(1.2, scroll), 0.1f, 200);
            moveCamera(deltaTime, axis(Key.D, Key.A), axis(Key.E, Key.Q), axis(Key.W, Key.S), Input.isShiftPressed());
        } else if (scroll != 0) {
            position.add(forward().mul(scroll * scrollSensitivity));
        }
    }

    /** Releases cursor capture when this camera stops receiving updates. */
    public void releaseCameraMovement() {
        if (movementWindow != 0 && movementWindow == Input.getWindowHandle() && !restoreCursorLock) Input.unlockCursor();
        movementWindow = 0;
    }

    private static float axis(Key positive, Key negative) {
        return (Input.isKeyDown(positive) ? 1 : 0) - (Input.isKeyDown(negative) ? 1 : 0);
    }

    void rotateCamera(float dx, float dy) {
        yaw = (yaw - dx * mouseSensitivity) % 360;
        pitch = Math.clamp(pitch - dy * mouseSensitivity, -89, 89);
        roll = 0;
    }

    private Vector3f forward() {
        float y = (float) Math.toRadians(yaw), p = (float) Math.toRadians(pitch);
        return new Vector3f(-(float) Math.sin(y) * (float) Math.cos(p), (float) Math.sin(p),
                -(float) Math.cos(y) * (float) Math.cos(p));
    }

    void moveCamera(float dt, float right, float up, float ahead, boolean fast) {
        float y = (float) Math.toRadians(yaw);
        Vector3f movement = forward().mul(ahead).add((float) Math.cos(y) * right, up, -(float) Math.sin(y) * right);
        if (movement.lengthSquared() > 1e-10f) {
            movement.normalize().mul(movementSpeed * (fast ? fastMultiplier : 1) * dt);
            position.add(movement);
        }
    }

    public void lookAt(Vector3f target) {
        Vector3f direction = new Vector3f(target).sub(position);
        if (direction.lengthSquared() < 1e-10f) throw new IllegalArgumentException("Camera target equals position");
        direction.normalize();
        pitch = (float) Math.toDegrees(Math.asin(direction.y));
        yaw = (float) Math.toDegrees(Math.atan2(-direction.x, -direction.z));
        roll = 0;
    }

    public Matrix4f getViewMatrix() {
        return new Matrix4f().rotateZ((float) Math.toRadians(-roll))
                .rotateX((float) Math.toRadians(-pitch))
                .rotateY((float) Math.toRadians(-yaw)).translate(-position.x, -position.y, -position.z);
    }

    public Matrix4f getProjectionMatrix() {
        if (!(aspect > 0 && near > 0 && far > near && fov > 0 && fov < 180)) {
            throw new IllegalArgumentException("Invalid camera perspective");
        }
        return new Matrix4f().perspective((float) Math.toRadians(fov), aspect, near, far);
    }

    public Matrix4f getViewProjectionMatrix() {
        return getProjectionMatrix().mul(getViewMatrix());
    }

    public FrustumIntersection getFrustum() {
        return new FrustumIntersection(getViewProjectionMatrix());
    }

    public Vector4f[] getFrustumPlanes() {
        Matrix4f matrix = getViewProjectionMatrix();
        Vector4f[] planes = new Vector4f[6];
        for (int i = 0; i < planes.length; i++) planes[i] = matrix.frustumPlane(i, new Vector4f());
        return planes;
    }
}

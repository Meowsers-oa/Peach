package net.meowsers.peach.core;

import org.joml.Vector3f;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class CameraMovementTest {
    @Test void movementFollowsViewAndIsFrameRateIndependent() {
        Camera camera = new Camera();
        camera.position.zero(); camera.yaw = -90; camera.pitch = 30;
        camera.moveCamera(1, 0, 0, 1, false);
        Vector3f expected = camera.getViewMatrix().invert().transformDirection(new Vector3f(0, 0, -1)).mul(camera.movementSpeed);
        assertTrue(camera.position.equals(expected, 0.0001f));
        Camera split = new Camera();
        split.position.zero(); split.yaw = camera.yaw; split.pitch = camera.pitch;
        for (int i = 0; i < 100; i++) split.moveCamera(0.01f, 0, 0, 1, false);
        assertTrue(split.position.equals(camera.position, 0.0001f));
    }

    @Test void diagonalSpeedBoostAndWorldUp() {
        Camera camera = new Camera(); camera.position.zero();
        camera.moveCamera(1, 1, 0, 1, false);
        assertEquals(camera.movementSpeed, camera.position.length(), 0.0001);
        camera.position.zero(); camera.pitch = 60;
        camera.moveCamera(1, 0, 1, 0, true);
        assertEquals(new Vector3f(0, camera.movementSpeed * camera.fastMultiplier, 0), camera.position);
        camera.position.zero(); camera.moveCamera(0, 1, 1, 1, true);
        assertEquals(new Vector3f(), camera.position);
    }

    @Test void mouseLookTurnsRightAndClampsPitch() {
        Camera camera = new Camera();
        camera.rotateCamera(100, -100);
        assertTrue(camera.yaw < 0); assertTrue(camera.pitch > 0);
        camera.rotateCamera(0, -100000);
        assertEquals(89, camera.pitch);
        camera.rotateCamera(0, 100000);
        assertEquals(-89, camera.pitch);
        assertThrows(IllegalArgumentException.class, () -> camera.handleCameraMovement(Float.NaN));
    }
}

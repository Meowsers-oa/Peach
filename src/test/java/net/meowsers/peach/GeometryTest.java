package net.meowsers.peach;

import net.meowsers.peach.core.Camera;
import net.meowsers.peach.rendering.Mesh;
import net.meowsers.peach.rendering.Model;
import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.utils.SetupSlang;
import org.joml.Matrix4f;
import net.meowsers.peach.structures.Transform;
import org.joml.Vector3f;
import org.junit.jupiter.api.Test;
import org.lwjgl.assimp.AIMatrix4x4;

import static org.junit.jupiter.api.Assertions.*;

class GeometryTest {
    private Mesh triangle() {
        return new Mesh(new Vertex[]{new Vertex(new Vector3f(-1, -1, 0)),
                new Vertex(new Vector3f(1, -1, 0)), new Vertex(new Vector3f(0, 1, 0))}, new int[]{0, 1, 2});
    }

    @Test void cameraAndTransformedBounds() {
        Camera camera = new Camera();
        Mesh mesh = triangle();
        assertTrue(mesh.isVisible(camera.getFrustum(), new Transform()));
        assertFalse(mesh.isVisible(camera.getFrustum(), new Transform(new Vector3f(0, 0, 10), new Vector3f(1), new Vector3f())));
        assertFalse(mesh.isVisible(camera.getFrustum(), new Transform(new Vector3f(100, 0, 0), new Vector3f(1), new Vector3f())));
        assertTrue(mesh.isVisible(camera.getFrustum(), new Transform(new Vector3f(), new Vector3f(-2, 0.5f, 1), new Vector3f(0, 0, 34.377f))));
        assertEquals(6, camera.getFrustumPlanes().length);
        assertEquals(0, camera.getViewMatrix().transformPosition(new Vector3f(camera.position)).length(), 0.0001);
        camera.yaw = 180;
        assertFalse(mesh.isVisible(camera.getFrustum(), new Transform()));
        camera.near = 0;
        assertThrows(IllegalArgumentException.class, camera::getProjectionMatrix);
    }

    @Test void rejectsMalformedTrianglesAndUpdatesBounds() {
        Mesh mesh = triangle();
        assertThrows(IllegalArgumentException.class, () -> new Mesh(mesh.getVertices(), new int[]{0, 1}));
        assertThrows(IllegalArgumentException.class, () -> new Mesh(mesh.getVertices(), new int[]{0, 1, 3}));
        mesh.getVertices()[0].position.x = -5;
        mesh.updateBounds();
        assertEquals(-5, mesh.getMin().x);
    }

    @Test void assimpMatrixConversionPreservesTranslationAndRotation() {
        try (AIMatrix4x4 matrix = AIMatrix4x4.calloc()) {
            matrix.a2(-1).b1(1).c3(1).d4(1).a4(3).b4(4).c4(5);
            Vector3f result = Model.toMatrix(matrix).transformPosition(new Vector3f(1, 0, 0));
            assertEquals(new Vector3f(3, 5, 5), result);
        }
    }

    @Test void slangCompatibilityRejectsNewerFeatures() {
        String converted = SetupSlang.toGlsl410("#version 460\nlayout(row_major) buffer;\nlayout(binding = 0, set = 0) uniform sampler2D imageSampler;\n");
        assertTrue(converted.startsWith("#version 410 core"));
        assertFalse(converted.contains("binding"));
        assertThrows(RuntimeException.class, () -> SetupSlang.toGlsl410("#version 460\nlayout(std430) buffer Storage { float x; };"));
        assertThrows(RuntimeException.class, () -> SetupSlang.toGlsl410("#version 460\n#extension GL_ARB_compute_shader : require"));
    }

    @Test void slangArchitectureSelection() {
        assertEquals("macos-aarch64", SetupSlang.platform("Mac OS X", "aarch64"));
        assertEquals("macos-aarch64", SetupSlang.platform("Mac OS X", "arm64"));
        assertEquals("macos-x86_64", SetupSlang.platform("Mac OS X", "x86_64"));
        assertEquals("macos-x86_64", SetupSlang.platform("Mac OS X", "amd64"));
        assertThrows(IllegalArgumentException.class, () -> SetupSlang.platform("Linux", "amd64"));
        assertThrows(IllegalArgumentException.class, () -> SetupSlang.platform("Mac OS X", "sparc"));
    }
}

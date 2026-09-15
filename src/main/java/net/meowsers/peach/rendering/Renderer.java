package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Mesh;
import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.utils.Log;
import org.joml.Matrix4f;
import org.joml.FrustumIntersection;
import org.joml.Vector3f;
import org.lwjgl.system.MemoryStack;

import java.nio.ByteBuffer;
import java.util.Map;
import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

import static org.lwjgl.opengl.GL20.*;


public class Renderer {
    private static Shader defaultShader, shader;
    private static Texture whiteTexture;
    private static final List<RenderBatch> batches = new ArrayList<>();
    private static int currentBatch;
    private static int textureSlots;
    private static final Matrix4f viewProjection = new Matrix4f();
    private static final Matrix4f meshTransform = new Matrix4f();
    private static final Matrix4f clipTransform = new Matrix4f();
    private static final FrustumIntersection frustum = new FrustumIntersection();
    private static final Vector3f boundsMin = new Vector3f(), boundsMax = new Vector3f();
    private static final Triangulator triangulator = new Triangulator();
    private static int[] vertexIndices = new int[0];

    public static void start() {
        if (!batches.isEmpty()) return;
        textureSlots = glGetInteger(GL_MAX_TEXTURE_IMAGE_UNITS);
        try {
            StringBuilder cases = new StringBuilder();
            for (int i = 0; i < textureSlots; i++) {
                cases.append("        case ").append(i).append(": return textureGrad(uTextures[")
                        .append(i).append("], vUv, dx, dy);\n");
            }
            defaultShader = new Shader("shaders/default.vert", "shaders/default.frag", Map.of(
                    "{{TEXTURE_SLOTS}}", Integer.toString(textureSlots),
                    "{{TEXTURE_CASES}}", cases.toString()));
            shader = defaultShader;
            try (MemoryStack stack = MemoryStack.stackPush()) {
                ByteBuffer pixel = stack.malloc(4);
                pixel.put((byte) 255).put((byte) 255).put((byte) 255).put((byte) 255).flip();
                whiteTexture = new Texture(1, 1, pixel);
            }
            batches.add(new RenderBatch(textureSlots, whiteTexture));
            viewProjection.identity();
            setSamplers();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            Log.message(glGetString(GL_RENDERER) + ": " + textureSlots + " texture slots (slot 0 reserved for white)");
        } catch (RuntimeException e) {
            end();
            throw e;
        }
    }

    /** One planar face, with vertices ordered around its outline. Indices are generated automatically. */
    public static void addVertices(Vertex... vertices) {
        addVertices(vertices, null, null, null);
    }
    public static void addVertices(Vertex[] vertices, Texture texture) {
        addVertices(vertices, null, texture, null);
    }
    public static void addVertices(Vertex[] vertices, int[] indices) {
        addVertices(vertices, indices, null, null);
    }
    public static void addVertices(Vertex[] vertices, int[] indices, Texture texture) {
        addVertices(vertices, indices, texture, null);
    }
    /** Null indices triangulate one ordered planar face. Explicit triangle indices describe arbitrary meshes. */
    public static void addVertices(Vertex[] vertices, int[] indices, Texture texture, Matrix4f model) {
        requireStarted();
        validateVertices(vertices);
        if (texture != null && texture.getId() == 0) throw new IllegalStateException("Texture has ended");
        if (indices != null) validateIndices(vertices.length, indices);
        if (vertices.length == 0) return;
        if (!isVisible(vertices, model)) return;
        if (indices == null) indices = triangulator.triangulate(vertices);
        submitVertices(vertices, indices, texture, model, 0, indices.length);
    }

    private static void validateVertices(Vertex[] vertices) {
        for (Vertex vertex : vertices) {
            if (vertex == null || vertex.position == null || vertex.color == null || vertex.uv == null) {
                throw new IllegalArgumentException("Each vertex needs a position, color and UV");
            }
        }
    }

    private static void validateIndices(int vertexCount, int[] indices) {
        if (indices.length % 3 != 0) throw new IllegalArgumentException("Explicit triangle indices must come in groups of three");
        for (int index : indices) {
            if (index < 0 || index >= vertexCount) throw new IllegalArgumentException("Vertex index out of bounds: " + index);
        }
    }

    private static void submitVertices(Vertex[] vertices, int[] indices, Texture texture, Matrix4f model, int offset, int end) {
        if (vertexIndices.length < vertices.length) {
            vertexIndices = new int[vertices.length];
            Arrays.fill(vertexIndices, -1);
        }
        while (offset < end) {
            offset = batches.get(currentBatch).addVertices(vertices, indices, texture, model, offset, end, vertexIndices);
            if (offset < end) {
                currentBatch++;
                if (currentBatch == batches.size()) batches.add(new RenderBatch(textureSlots, whiteTexture));
            }
        }
    }

    /** Mesh textures: none = white, one = shared, otherwise one entry per triangle face. */
    public static void addMesh(Mesh mesh) {
        addMesh(mesh, null);
    }

    /** Parent transform is combined with the mesh's local transform without modifying either. */
    public static void addMesh(Mesh mesh, Matrix4f parentTransform) {
        requireStarted();
        Vertex[] vertices = mesh.getVertices().toArray(Vertex[]::new);
        validateVertices(vertices);
        if (vertices.length == 0) return;
        mesh.transform.toMatrix(meshTransform);
        if (parentTransform != null) parentTransform.mul(meshTransform, meshTransform);
        if (!isVisible(vertices, meshTransform)) return;
        int[] indices = mesh.getIndices().stream().mapToInt(Integer::intValue).toArray();
        validateIndices(vertices.length, indices);
        if (indices.length == 0) indices = triangulator.triangulate(vertices);

        List<Texture> textures = mesh.getTextures();
        if (textures.size() > 1 && textures.size() != indices.length / 3) {
            throw new IllegalArgumentException("Mesh needs no textures, one shared texture, or one texture per triangle face");
        }
        for (Texture texture : textures) {
            if (texture != null && texture.getId() == 0) throw new IllegalStateException("Texture has ended");
        }
        if (textures.size() <= 1) {
            submitVertices(vertices, indices, textures.isEmpty() ? null : textures.get(0), meshTransform, 0, indices.length);
            return;
        }

        // Submit adjacent faces sharing a texture together, without copying their index ranges.
        int offset = 0;
        while (offset < indices.length) {
            Texture texture = textures.get(offset / 3);
            int end = offset + 3;
            while (end < indices.length && textures.get(end / 3) == texture) end += 3;
            submitVertices(vertices, indices, texture, meshTransform, offset, end);
            offset = end;
        }
    }

    private static boolean isVisible(Vertex[] vertices, Matrix4f model) {
        // Recompute bounds because vertex positions are publicly editable.
        boundsMin.set(Float.POSITIVE_INFINITY);
        boundsMax.set(Float.NEGATIVE_INFINITY);
        for (Vertex vertex : vertices) {
            boundsMin.min(vertex.position);
            boundsMax.max(vertex.position);
        }
        if (model == null) clipTransform.set(viewProjection);
        else viewProjection.mul(model, clipTransform);
        // Test in local space against the transformed frustum: handles rotation and nonuniform scale.
        return frustum.set(clipTransform).testAab(boundsMin, boundsMax);
    }


    public static void setCamera(Matrix4f view, Matrix4f projection) {
        flush();
        projection.mul(view, viewProjection);
    }

    public static void setViewProjection(Matrix4f matrix) {
        flush();
        viewProjection.set(matrix);
    }

    /** Custom shaders use the batch vertex layout, uViewProjection and uTextures. Null restores the default. */
    public static void setShader(Shader customShader) {
        flush();
        shader = customShader == null ? defaultShader : customShader;
        setSamplers();
    }

    private static void setSamplers() {
        int[] slots = new int[textureSlots];
        for (int i = 0; i < slots.length; i++) slots[i] = i;
        shader.bind();
        shader.setUniform("uTextures", slots);
    }

    /** Call before changing GL state or uniforms, or ending a texture used by queued vertices. */
    public static void flush() {
        requireStarted();
        shader.bind();
        shader.setUniform("uViewProjection", viewProjection);
        for (int i = 0; i <= currentBatch; i++) batches.get(i).flush();
        currentBatch = 0;
    }

    private static void requireStarted() {
        if (batches.isEmpty()) throw new IllegalStateException("Renderer.start() must be called with a current OpenGL context");
    }

    public static void end() {
        for (RenderBatch batch : batches) batch.end();
        if (whiteTexture != null) whiteTexture.end();
        if (defaultShader != null) defaultShader.end();
        batches.clear();
        vertexIndices = new int[0];
        currentBatch = 0;
        whiteTexture = null;
        defaultShader = shader = null;
        textureSlots = 0;
    }

    public static int getTextureSlots() {
        return textureSlots;
    }
}

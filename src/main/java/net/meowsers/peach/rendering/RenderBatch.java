package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.structures.Transform;
import org.joml.Matrix3f;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.lwjgl.system.MemoryUtil;
import org.lwjgl.system.MemoryStack;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static org.lwjgl.opengl.GL41.*;

/** A streaming triangle batch. The caller binds its shader and target before submission. */
public class RenderBatch implements AutoCloseable {
    private static final int FLOATS_PER_VERTEX = 16;
    private final int maxVertices, maxIndices, maxTextures;
    private int vao, vbo, ebo;
    private FloatBuffer vertexData;
    private IntBuffer indexData;
    private int vertexCount;
    private final List<Texture> textures = new ArrayList<>();
    private final Vector3f position = new Vector3f(), normal = new Vector3f();
    private int drawCalls;
    private Texture whiteTexture;

    public RenderBatch(int maxVertices, int maxIndices, int maxTextures) {
        if (maxVertices < 3 || maxIndices < 3 || maxTextures < 1
                || maxTextures > glGetInteger(GL_MAX_TEXTURE_IMAGE_UNITS)) throw new IllegalArgumentException("Invalid batch capacity");
        this.maxVertices = maxVertices;
        this.maxIndices = maxIndices;
        this.maxTextures = maxTextures;
        try {
            try (MemoryStack stack = MemoryStack.stackPush()) {
                whiteTexture = new Texture(stack.bytes((byte) 255, (byte) 255, (byte) 255, (byte) 255),
                        1, 1, 4, Texture.Filter.NEAREST, false);
            }
            vertexData = MemoryUtil.memAllocFloat(Math.multiplyExact(maxVertices, FLOATS_PER_VERTEX));
            indexData = MemoryUtil.memAllocInt(maxIndices);
            vao = glGenVertexArrays();
            vbo = glGenBuffers();
            ebo = glGenBuffers();
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, (long) maxVertices * FLOATS_PER_VERTEX * Float.BYTES, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long) maxIndices * Integer.BYTES, GL_DYNAMIC_DRAW);
            int[] sizes = {3, 4, 2, 3, 1, 3};
            int offset = 0;
            for (int i = 0; i < sizes.length; i++) {
                glEnableVertexAttribArray(i);
                glVertexAttribPointer(i, sizes[i], GL_FLOAT, false, FLOATS_PER_VERTEX * Float.BYTES, (long) offset * Float.BYTES);
                offset += sizes[i];
            }
        } catch (RuntimeException e) { dispose(); throw e; }
        finally { glBindVertexArray(0); glBindBuffer(GL_ARRAY_BUFFER, 0); }
    }

    public void add(Vertex[] vertices, int[] indices, Texture texture, Vector4f tint, Transform transform) {
        add(vertices, indices, texture, tint, transform, new Vector3f());
    }

    public void add(Vertex[] vertices, int[] indices, Texture texture, Vector4f tint, Transform transform, Vector3f emission) {
        addTransformed(vertices, indices, texture, tint, transform.toMatrix(), emission);
    }

    void addTransformed(Vertex[] vertices, int[] indices, Texture texture, Vector4f tint, Matrix4f transform, Vector3f emission) {
        if (vao == 0) throw new IllegalStateException("Batch is disposed");
        Mesh.validate(vertices, indices);
        if (indices.length == 0) return;
        if (!transform.isFinite() || !transform.isAffine() || Math.abs(transform.determinant3x3()) < 1e-10f) {
            throw new IllegalArgumentException("Mesh transform must be affine and invertible");
        }
        Matrix3f normalMatrix = transform.normal(new Matrix3f());
        if (vertices.length <= maxVertices && indices.length <= maxIndices) {
            if (vertexCount + vertices.length > maxVertices || indexData.position() + indices.length > maxIndices) flush();
            int slot = textureSlot(texture);
            int base = vertexCount;
            for (Vertex vertex : vertices) put(vertex, tint, transform, normalMatrix, slot, emission);
            for (int index : indices) indexData.put(base + index);
            return;
        }
        // Split oversized meshes on triangle boundaries, preserving indexed sharing within each chunk.
        int[] remap = new int[vertices.length];
        Arrays.fill(remap, -1);
        int slot = textureSlot(texture);
        for (int i = 0; i < indices.length; i += 3) {
            if (vertexCount + 3 > maxVertices || indexData.position() + 3 > maxIndices) {
                flush();
                Arrays.fill(remap, -1);
                slot = textureSlot(texture);
            }
            for (int j = 0; j < 3; j++) {
                int index = indices[i + j];
                if (remap[index] == -1) {
                    remap[index] = vertexCount;
                    put(vertices[index], tint, transform, normalMatrix, slot, emission);
                }
                indexData.put(remap[index]);
            }
        }
    }

    private int textureSlot(Texture texture) {
        if (texture == null) return -1;
        if (texture.getId() == 0) throw new IllegalStateException("Cannot submit a disposed texture");
        int slot = textures.indexOf(texture);
        if (slot >= 0) return slot;
        if (textures.size() == maxTextures) flush();
        textures.add(texture);
        return textures.size() - 1;
    }

    private void put(Vertex vertex, Vector4f tint, Matrix4f transform, Matrix3f normalMatrix, int slot, Vector3f emission) {
        transform.transformPosition(vertex.position, position);
        normalMatrix.transform(vertex.normal, normal);
        if (normal.lengthSquared() > 0) normal.normalize();
        vertexData.put(position.x).put(position.y).put(position.z);
        vertexData.put(vertex.color.x * tint.x).put(vertex.color.y * tint.y).put(vertex.color.z * tint.z).put(vertex.color.w * tint.w);
        vertexData.put(vertex.uv.x).put(vertex.uv.y);
        vertexData.put(normal.x).put(normal.y).put(normal.z).put(slot);
        vertexData.put(emission.x).put(emission.y).put(emission.z);
        vertexCount++;
    }

    public void flush() {
        if (indexData == null || indexData.position() == 0) return;
        vertexData.flip();
        indexData.flip();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // Orphan the previous storage to avoid waiting for the GPU's last draw.
        glBufferData(GL_ARRAY_BUFFER, (long) maxVertices * FLOATS_PER_VERTEX * Float.BYTES, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long) maxIndices * Integer.BYTES, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexData);
        for (int i = 0; i < maxTextures; i++) {
            (i < textures.size() ? textures.get(i) : whiteTexture).bind(i);
        }
        glDrawElements(GL_TRIANGLES, indexData.remaining(), GL_UNSIGNED_INT, 0L);
        drawCalls++;
        vertexData.clear();
        indexData.clear();
        vertexCount = 0;
        textures.clear();
    }

    public int getDrawCalls() { return drawCalls; }
    public void resetStats() { drawCalls = 0; }
    public void dispose() {
        if (whiteTexture != null) whiteTexture.dispose();
        whiteTexture = null;
        if (vao != 0) glDeleteVertexArrays(vao);
        if (vbo != 0) glDeleteBuffers(vbo);
        if (ebo != 0) glDeleteBuffers(ebo);
        vao = vbo = ebo = 0;
        MemoryUtil.memFree(vertexData);
        MemoryUtil.memFree(indexData);
        vertexData = null;
        indexData = null;
        textures.clear();
    }
    public void cleanup() { dispose(); }
    @Override public void close() { dispose(); }
}

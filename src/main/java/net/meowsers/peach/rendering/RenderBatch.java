package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Vertex;
import org.joml.Matrix4f;
import org.joml.Vector3f;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.Arrays;

import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.system.MemoryUtil.*;

public class RenderBatch {
    // Position (3), color (4), UV (2), texture slot (1).
    private static final int VERTEX_SIZE = 10;

    private int vao, vbo, ebo;
    private FloatBuffer vertices;
    private IntBuffer indices;
    private int vertexCount;
    private final Texture[] textures;
    private int textureCount = 1;
    private final Vector3f position = new Vector3f();

    public RenderBatch(int textureSlots, Texture whiteTexture) {
        if (textureSlots < 2) throw new IllegalArgumentException("At least two texture slots are required");
        textures = new Texture[textureSlots];
        textures[0] = whiteTexture;
        vertices = memAllocFloat(1024 * VERTEX_SIZE);
        indices = memAllocInt(3072);
        vao = glGenVertexArrays();
        vbo = glGenBuffers();
        ebo = glGenBuffers();

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        attribute(0, 3, 0);
        attribute(1, 4, 3);
        attribute(2, 2, 7);
        attribute(3, 1, 9);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    private void attribute(int location, int size, int offset) {
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, size, GL_FLOAT, false, VERTEX_SIZE * Float.BYTES, (long) offset * Float.BYTES);
    }

    /** Returns false when a new texture needs a flush. Null indices mean consecutive triangles. */
    public boolean addVertices(Vertex[] addedVertices, int[] addedIndices, Texture texture, Matrix4f model) {
        if (vao == 0) throw new IllegalStateException("Render batch has ended");
        int count = addedIndices == null ? addedVertices.length : addedIndices.length;
        if (count % 3 != 0) throw new IllegalArgumentException("Triangle indices must come in groups of three");
        if (addedIndices != null) {
            for (int index : addedIndices) {
                if (index < 0 || index >= addedVertices.length) throw new IllegalArgumentException("Vertex index out of bounds: " + index);
            }
        }
        for (Vertex vertex : addedVertices) {
            if (vertex == null || vertex.position == null || vertex.color == null || vertex.uv == null) {
                throw new IllegalArgumentException("Each vertex needs a position, color and UV");
            }
        }
        if (texture != null && texture.getId() == 0) throw new IllegalStateException("Texture has ended");
        if (count == 0) return true;

        int slot = texture == null ? 0 : findTexture(texture);
        if (slot == -1) {
            if (textureCount == textures.length) return false;
            slot = textureCount++;
            textures[slot] = texture;
        }

        int requiredVertices = Math.addExact(vertices.position(), Math.multiplyExact(addedVertices.length, VERTEX_SIZE));
        int requiredIndices = Math.addExact(indices.position(), count);
        if (requiredVertices > vertices.capacity()) vertices = memRealloc(vertices, Math.max(requiredVertices, vertices.capacity() * 2));
        if (requiredIndices > indices.capacity()) indices = memRealloc(indices, Math.max(requiredIndices, indices.capacity() * 2));

        for (Vertex vertex : addedVertices) {
            position.set(vertex.position);
            if (model != null) model.transformPosition(position);
            vertices.put(position.x).put(position.y).put(position.z);
            vertices.put(vertex.color.r).put(vertex.color.g).put(vertex.color.b).put(vertex.color.a);
            vertices.put(vertex.uv.x).put(vertex.uv.y).put(slot);
        }
        for (int i = 0; i < count; i++) {
            indices.put(vertexCount + (addedIndices == null ? i : addedIndices[i]));
        }
        vertexCount += addedVertices.length;
        return true;
    }

    private int findTexture(Texture texture) {
        for (int i = 0; i < textureCount; i++) {
            if (textures[i] == texture) return i;
        }
        return -1;
    }

    /** Draws with the currently bound shader, then reuses the buffers for the next batch. */
    public void flush() {
        if (vao == 0) throw new IllegalStateException("Render batch has ended");
        if (indices.position() == 0) return;
        // Every sampler needs a complete texture, including unused slots on Apple's driver.
        for (int i = 0; i < textures.length; i++) {
            (i < textureCount ? textures[i] : textures[0]).bind(i);
        }
        glActiveTexture(GL_TEXTURE0);

        vertices.flip();
        indices.flip();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLES, indices.remaining(), GL_UNSIGNED_INT, 0L);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        vertices.clear();
        indices.clear();
        vertexCount = 0;
        Arrays.fill(textures, 1, textureCount, null);
        textureCount = 1;
    }

    public void end() {
        if (vao == 0) return;
        glDeleteVertexArrays(vao);
        glDeleteBuffers(vbo);
        glDeleteBuffers(ebo);
        memFree(vertices);
        memFree(indices);
        vertices = null;
        indices = null;
        Arrays.fill(textures, null);
        vao = vbo = ebo = 0;
    }
}

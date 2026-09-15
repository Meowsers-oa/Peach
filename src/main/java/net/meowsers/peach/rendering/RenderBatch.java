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
    private static final int MAX_VERTICES = 10_000;
    private static final int MAX_INDICES = 30_000;

    // Position (3), color (4), UV (2), texture slot (1).
    private static final int VERTEX_SIZE = 10;

    private int vao, vbo, ebo;
    private FloatBuffer vertices;
    private IntBuffer indices;
    private int vertexCount;
    private final Texture[] textures;
    private int textureCount = 1;
    private final Vector3f position = new Vector3f();
    private final int[] mappedVertices = new int[MAX_VERTICES];
    private final int[] triangle = new int[3];

    public RenderBatch(int textureSlots, Texture whiteTexture) {
        if (textureSlots < 2) throw new IllegalArgumentException("At least two texture slots are required");
        textures = new Texture[textureSlots];
        textures[0] = whiteTexture;
        vertices = memAllocFloat(MAX_VERTICES * VERTEX_SIZE);
        indices = memAllocInt(MAX_INDICES);
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

    /** Adds complete triangles until full, returning the next index offset to submit. */
    int addVertices(Vertex[] addedVertices, int[] addedIndices, Texture texture, Matrix4f model, int offset, int end, int[] vertexIndices) {
        if (vao == 0) throw new IllegalStateException("Render batch has ended");
        int slot = texture == null ? 0 : findTexture(texture);
        if (slot == -1) {
            if (textureCount == textures.length) return offset;
            slot = textureCount++;
            textures[slot] = texture;
        }

        // Source indices are remapped locally so shared vertices survive a batch boundary.
        int mappedCount = 0;
        boolean mirrored = model != null && model.determinant3x3() < 0;
        while (offset < end) {
            int needed = 0;
            for (int i = 0; i < 3; i++) {
                triangle[i] = addedIndices[offset + i];
                if (vertexIndices[triangle[i]] == -1
                        && (i < 1 || triangle[i] != triangle[0])
                        && (i < 2 || triangle[i] != triangle[1])) needed++;
            }
            if (needed > MAX_VERTICES - vertexCount || indices.remaining() < 3) break;
            if (mirrored) {
                int second = triangle[1];
                triangle[1] = triangle[2];
                triangle[2] = second;
            }

            for (int index : triangle) {
                int localIndex = vertexIndices[index];
                if (localIndex == -1) {
                    Vertex vertex = addedVertices[index];
                    position.set(vertex.position);
                    if (model != null) model.transformPosition(position);
                    vertices.put(position.x).put(position.y).put(position.z);
                    vertices.put(vertex.color.r).put(vertex.color.g).put(vertex.color.b).put(vertex.color.a);
                    vertices.put(vertex.uv.x).put(vertex.uv.y).put(slot);
                    localIndex = vertexCount++;
                    vertexIndices[index] = localIndex;
                    mappedVertices[mappedCount++] = index;
                }
                indices.put(localIndex);
            }
            offset += 3;
        }
        for (int i = 0; i < mappedCount; i++) vertexIndices[mappedVertices[i]] = -1;
        return offset;
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

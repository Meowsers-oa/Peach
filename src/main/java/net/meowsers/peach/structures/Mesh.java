package net.meowsers.peach.structures;

import net.meowsers.peach.rendering.Texture;
import net.meowsers.peach.rendering.Triangulator;
import org.joml.Matrix4f;
import org.joml.Vector2f;
import org.joml.Vector3f;

import java.util.List;
import java.util.ArrayList;
import java.util.HashSet;

public class Mesh {
    private final List<Vertex> vertices = new ArrayList<>();
    private final List<Integer> indices = new ArrayList<>();
    // Empty = white; one = shared; otherwise one texture per triangle face. Null also uses white.
    private final List<Texture> textures = new ArrayList<>();

    public Transform transform = new Transform();

    /** Copies another mesh into this mesh's local space, baking its transform and preserving face textures. */
    public void addMesh(Mesh mesh) {
        if (mesh == this) throw new IllegalArgumentException("A mesh cannot merge itself");
        if (mesh.vertices.isEmpty()) return;
        int[] currentIndices = triangleIndices();
        int[] addedIndices = mesh.triangleIndices();
        List<Texture> faceTextures = faceTextures(currentIndices.length / 3);
        faceTextures.addAll(mesh.faceTextures(addedIndices.length / 3));

        Matrix4f model = mesh.transform.toMatrix();
        List<Vertex> addedVertices = new ArrayList<>(mesh.vertices.size());
        for (Vertex vertex : mesh.vertices) {
            Vector3f position = model.transformPosition(new Vector3f(vertex.position));
            Color color = new Color(vertex.color.r, vertex.color.g, vertex.color.b, vertex.color.a);
            addedVertices.add(new Vertex(position, color, new Vector2f(vertex.uv)));
        }
        int offset = vertices.size();
        vertices.addAll(addedVertices);
        indices.clear();
        for (int index : currentIndices) indices.add(index);
        for (int index : addedIndices) indices.add(offset + index);
        textures.clear();
        textures.addAll(faceTextures);
    }

    private int[] triangleIndices() {
        for (Vertex vertex : vertices) {
            if (vertex == null || vertex.position == null || vertex.color == null || vertex.uv == null) {
                throw new IllegalArgumentException("Each vertex needs a position, color and UV");
            }
        }
        int[] result = indices.stream().mapToInt(Integer::intValue).toArray();
        if (result.length % 3 != 0) throw new IllegalArgumentException("Explicit triangle indices must come in groups of three");
        for (int index : result) {
            if (index < 0 || index >= vertices.size()) throw new IllegalArgumentException("Vertex index out of bounds: " + index);
        }
        if (result.length == 0 && !vertices.isEmpty()) {
            return new Triangulator().triangulate(vertices.toArray(Vertex[]::new));
        }
        return result;
    }

    private List<Texture> faceTextures(int count) {
        if (textures.size() > 1 && textures.size() != count) {
            throw new IllegalArgumentException("Mesh needs no textures, one shared texture, or one texture per triangle face");
        }
        List<Texture> result = new ArrayList<>(count);
        for (int i = 0; i < count; i++) {
            result.add(textures.isEmpty() ? null : textures.get(textures.size() == 1 ? 0 : i));
        }
        return result;
    }

    public void addVertices(List<Vertex> vertices) {
        this.vertices.addAll(vertices);
    }
    public void addIndices(List<Integer> indices) {
        this.indices.addAll(indices);
    }
    public void addTextures(List<Texture> textures) {
        this.textures.addAll(textures);
    }

    public List<Vertex> getVertices() {
        return vertices;
    }
    public List<Integer> getIndices() {
        return indices;
    }
    /** Empty uses white, one entry covers the mesh, otherwise entries match the triangle faces. */
    public List<Texture> getTextures() {
        return textures;
    }

    /** Releases geometry and textures. Flush first, and finish any other meshes sharing these textures. */
    public void end() {
        for (Texture texture : new HashSet<>(textures)) {
            if (texture != null) texture.end();
        }
        vertices.clear();
        indices.clear();
        textures.clear();
    }
}

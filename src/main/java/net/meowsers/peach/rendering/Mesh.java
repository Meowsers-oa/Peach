package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Material;
import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.structures.Transform;
import org.joml.FrustumIntersection;
import org.joml.Matrix4f;
import org.joml.Vector3f;

import java.util.List;

/** CPU geometry. Textures are borrowed; the model or caller owns their lifetime. */
public class Mesh {
    private final Vertex[] vertices;
    private final int[] indices;
    private final List<Texture> textures;
    private final Vector3f min = new Vector3f(), max = new Vector3f();
    public final Material material;
    public final Transform transform = new Transform();

    public Mesh(Vertex[] vertices, int[] indices) {
        this(vertices, indices, new Material(), List.of());
    }

    public Mesh(Vertex[] vertices, int[] indices, Material material, List<Texture> textures) {
        validate(vertices, indices);
        this.vertices = vertices.clone();
        this.indices = indices.clone();
        this.material = material;
        this.textures = List.copyOf(textures);
        updateBounds();
    }

    public static void validate(Vertex[] vertices, int[] indices) {
        if (indices.length % 3 != 0) throw new IllegalArgumentException("Indices must describe triangles");
        for (Vertex vertex : vertices) {
            if (vertex == null || !vertex.position.isFinite()) throw new IllegalArgumentException("Invalid vertex");
        }
        for (int index : indices) {
            if (index < 0 || index >= vertices.length) throw new IllegalArgumentException("Vertex index out of bounds: " + index);
        }
    }

    /** Call after changing vertex positions. */
    public void updateBounds() {
        min.set(Float.POSITIVE_INFINITY);
        max.set(Float.NEGATIVE_INFINITY);
        for (Vertex vertex : vertices) { min.min(vertex.position); max.max(vertex.position); }
        if (vertices.length == 0) { min.zero(); max.zero(); }
    }

    /** Conservative world AABB, valid for rotation, nonuniform scale, reflection and shear. */
    public boolean isVisible(FrustumIntersection frustum, Transform parent) {
        return isVisible(frustum, parent.toMatrix().mul(transform.toMatrix()));
    }

    boolean isVisible(FrustumIntersection frustum, Matrix4f world) {
        Vector3f center = new Vector3f(min).add(max).mul(0.5f);
        Vector3f extent = new Vector3f(max).sub(min).mul(0.5f);
        world.transformPosition(center);
        float x = Math.abs(world.m00()) * extent.x + Math.abs(world.m10()) * extent.y + Math.abs(world.m20()) * extent.z;
        float y = Math.abs(world.m01()) * extent.x + Math.abs(world.m11()) * extent.y + Math.abs(world.m21()) * extent.z;
        float z = Math.abs(world.m02()) * extent.x + Math.abs(world.m12()) * extent.y + Math.abs(world.m22()) * extent.z;
        return frustum.testAab(center.x - x, center.y - y, center.z - z, center.x + x, center.y + y, center.z + z);
    }

    public Vertex[] getVertices() { return vertices; }
    public int[] getIndices() { return indices; }
    public List<Texture> getTextures() { return textures; }
    public Texture getTexture() { return textures.isEmpty() ? null : textures.getFirst(); }
    public Vector3f getMin() { return new Vector3f(min); }
    public Vector3f getMax() { return new Vector3f(max); }
}

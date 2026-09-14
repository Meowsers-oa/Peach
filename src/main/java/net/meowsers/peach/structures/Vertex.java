package net.meowsers.peach.structures;

import org.joml.Vector2f;
import org.joml.Vector3f;
import org.joml.Vector4f;

public class Vertex {
    public final Vector3f position;
    public final Vector4f color;
    public final Vector2f uv;
    public final Vector3f normal;

    public Vertex(Vector3f position) {
        this(position, new Vector4f(1), new Vector2f(), new Vector3f(0, 0, 1));
    }

    public Vertex(Vector3f position, Vector4f color, Vector2f uv, Vector3f normal) {
        this.position = new Vector3f(position);
        this.color = new Vector4f(color);
        this.uv = new Vector2f(uv);
        this.normal = new Vector3f(normal);
    }
}

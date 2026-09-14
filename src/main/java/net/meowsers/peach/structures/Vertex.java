package net.meowsers.peach.structures;

import org.joml.Vector2f;
import org.joml.Vector3f;

public class Vertex {
    public Vector3f position;
    public Color color;
    public Vector2f uv;

    public Vertex(Vector3f position, Color color, Vector2f uv) {
        this.position = position;
        this.color = color;
        this.uv = uv;
    }

    public Vertex(Vector3f position, Vector2f uv) {
        this(position, new Color(1.f), uv);
    }

    public Vertex(Vector3f position, Color color) {
        this(position, color, new Vector2f());
    }

    public Vertex(float x, float y, float z) {
        this(new Vector3f(x, y, z), new Color(1.f));
    }
}

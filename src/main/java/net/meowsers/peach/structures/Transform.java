package net.meowsers.peach.structures;

import org.joml.Matrix4f;
import org.joml.Vector2f;
import org.joml.Vector3f;

/** Scene pose: position, XYZ Euler rotation in degrees, and scale. */
public class Transform {
    public Vector3f position, scale, rotation;

    public Transform(Vector3f position, Vector3f scale, Vector3f rotation) {
        this.position = position;
        this.scale = scale;
        this.rotation = rotation;
    }

    /** Compatibility with the original two-axis constructor; Z rotation starts at zero. */
    public Transform(Vector3f position, Vector3f scale, Vector2f rotation) {
        this(position, scale, new Vector3f(rotation.x, rotation.y, 0));
    }

    public Transform() {
        this(new Vector3f(), new Vector3f(1), new Vector3f());
    }

    public Transform(Transform other) {
        this(new Vector3f(other.position), new Vector3f(other.scale), new Vector3f(other.rotation));
    }

    /** Matrix conversion at the rendering boundary: T * Rx * Ry * Rz * S. */
    public Matrix4f toMatrix() { return toMatrix(new Matrix4f()); }

    public Matrix4f toMatrix(Matrix4f destination) {
        if (position == null || rotation == null || scale == null || !position.isFinite()
                || !rotation.isFinite() || !scale.isFinite() || scale.x == 0 || scale.y == 0 || scale.z == 0) {
            throw new IllegalArgumentException("Transform must have finite position/rotation and nonzero scale");
        }
        return destination.translation(position).rotateXYZ((float) Math.toRadians(rotation.x),
                (float) Math.toRadians(rotation.y), (float) Math.toRadians(rotation.z)).scale(scale);
    }
}

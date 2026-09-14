package net.meowsers.peach.structures;

import org.joml.Vector4f;
import org.joml.Vector3f;

public class Material {
    public String name = "Material";
    public final Vector4f color = new Vector4f(1);
    public final Vector3f emission = new Vector3f();
    public boolean castsShadow = true;
    public float shininess = 32;
}

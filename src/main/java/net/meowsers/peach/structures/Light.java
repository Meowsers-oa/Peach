package net.meowsers.peach.structures;

import org.joml.Vector3f;

public class Light {
    public LightType type;
    public final Vector3f position = new Vector3f();
    public final Vector3f direction = new Vector3f(0, -1, 0);
    public final Vector3f color = new Vector3f(1);
    public float intensity = 1;
    // One directional or spot light can cast shadows in the current renderer.
    public boolean castsShadows;
    public float shadowNear = 0.1f, shadowFar = 50, shadowExtent = 12;
    public float shadowBias = 0.0015f;
    // Cone half-angles, in degrees.
    public float innerCutoff = 12.5f, outerCutoff = 17.5f;
    public float constant = 1, linear = 0.09f, quadratic = 0.032f;

    public Light(LightType type) {
        this.type = type;
    }
}

package net.meowsers;

import net.meowsers.peach.core.PeachLevel;
import net.meowsers.peach.rendering.Camera;
import net.meowsers.peach.rendering.Renderer;
import net.meowsers.peach.structures.Color;
import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.structures.WindowParams;
import org.joml.Vector3f;

public class MyLevel extends PeachLevel {
    Camera cam = new Camera((float) WindowParams.width / WindowParams.height, 100.f, .01f, 80);
    private final Vertex[] quad = {
            new Vertex(new Vector3f(-1, -1, 0), Color.Cornflower),
            new Vertex(new Vector3f(1, -1, 0), Color.Cornflower),
            new Vertex(new Vector3f(1, 1, 0), Color.Cornflower),
            new Vertex(new Vector3f(-1, 1, 0), Color.Cornflower)
    };

    @Override
    public void update(float dt) {
        Renderer.addVertices(quad);
    }
}

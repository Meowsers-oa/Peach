package net.meowsers.peach.rendering.post;

import net.meowsers.peach.rendering.Framebuffer;
import net.meowsers.peach.rendering.Shader;

public class InvertPass implements PostProcessingPass {
    private Shader shader;
    private FullscreenQuad quad;

    @Override public void render(Framebuffer input, Framebuffer output) {
        if (shader == null) {
            shader = Shader.fromResources("/shaders/fullscreen.vert", "/shaders/invert.frag");
            shader.set("scene", 0);
            quad = new FullscreenQuad();
        }
        quad.render(shader, input, output);
    }

    @Override public void dispose() {
        if (shader != null) shader.dispose();
        if (quad != null) quad.dispose();
        shader = null; quad = null;
    }
}

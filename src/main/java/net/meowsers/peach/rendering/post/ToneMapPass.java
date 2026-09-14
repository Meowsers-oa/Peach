package net.meowsers.peach.rendering.post;

import net.meowsers.peach.rendering.Framebuffer;
import net.meowsers.peach.rendering.Shader;

/** Converts linear HDR to display color. Place after bloom and other HDR effects. */
public class ToneMapPass implements PostProcessingPass {
    public float exposure = 1;
    private Shader shader;
    private FullscreenQuad quad;

    @Override public void render(Framebuffer input, Framebuffer output) {
        if (shader == null) {
            shader = Shader.fromResources("/shaders/fullscreen.vert", "/shaders/tonemap.frag");
            shader.set("scene", 0);
            quad = new FullscreenQuad();
        }
        shader.set("exposure", exposure);
        quad.render(shader, input, output);
    }
    @Override public void dispose() {
        if (shader != null) shader.dispose();
        if (quad != null) quad.dispose();
        shader = null; quad = null;
    }
}

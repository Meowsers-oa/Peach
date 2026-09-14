package net.meowsers.peach.rendering.post;

import net.meowsers.peach.rendering.Framebuffer;
import net.meowsers.peach.rendering.Shader;
import org.joml.Vector2f;

import java.util.ArrayList;
import java.util.List;

/** HDR bright extraction followed by progressive downsample and tent upsample blur. */
public class BloomPass implements PostProcessingPass {
    public float threshold = 1, intensity = 0.15f;
    private final int levels;
    private final List<Framebuffer> down = new ArrayList<>(), up = new ArrayList<>();
    private Shader filter, compose;
    private FullscreenQuad quad;
    private int width, height;

    public BloomPass() { this(5); }
    public BloomPass(int levels) {
        if (levels < 1 || levels > 12) throw new IllegalArgumentException("Bloom levels must be between 1 and 12");
        this.levels = levels;
    }

    private void init(int width, int height) {
        if (filter == null) {
            try {
                filter = Shader.fromResources("/shaders/fullscreen.vert", "/shaders/bloom-filter.frag");
                compose = Shader.fromResources("/shaders/fullscreen.vert", "/shaders/bloom-compose.frag");
                filter.set("source", 0); filter.set("detail", 1);
                compose.set("scene", 0); compose.set("bloom", 1);
                quad = new FullscreenQuad();
            } catch (RuntimeException e) { dispose(); throw e; }
        }
        if (this.width == width && this.height == height) return;
        clearTargets();
        this.width = this.height = 0;
        try {
            int w = width, h = height;
            for (int i = 0; i < levels; i++) {
                w = Math.max(1, w / 2); h = Math.max(1, h / 2);
                down.add(new Framebuffer(w, h, false));
                up.add(new Framebuffer(w, h, false));
                if (w == 1 && h == 1) break;
            }
            this.width = width; this.height = height;
        } catch (RuntimeException e) { clearTargets(); throw e; }
    }

    @Override public void render(Framebuffer input, Framebuffer output) {
        if (input == null || output == null || input == output) throw new IllegalArgumentException("Bloom needs distinct input/output targets");
        init(input.getWidth(), input.getHeight());
        Framebuffer source = input;
        filter.set("threshold", threshold);
        for (int i = 0; i < down.size(); i++) {
            Framebuffer target = down.get(i);
            filter.set("mode", i == 0 ? 0 : 1);
            filter.set("texel", new Vector2f(1f / source.getWidth(), 1f / source.getHeight()));
            quad.render(filter, target, target.getWidth(), target.getHeight(), source.getColorTexture(), source.getColorTexture());
            source = target;
        }
        for (int i = down.size() - 2; i >= 0; i--) {
            Framebuffer target = up.get(i);
            filter.set("mode", 2);
            filter.set("texel", new Vector2f(1f / source.getWidth(), 1f / source.getHeight()));
            quad.render(filter, target, target.getWidth(), target.getHeight(), source.getColorTexture(), down.get(i).getColorTexture());
            source = target;
        }
        compose.set("intensity", intensity);
        quad.render(compose, output, output.getWidth(), output.getHeight(), input.getColorTexture(), source.getColorTexture());
    }

    private void clearTargets() {
        down.forEach(Framebuffer::dispose); up.forEach(Framebuffer::dispose);
        down.clear(); up.clear();
    }
    @Override public void dispose() {
        clearTargets();
        if (filter != null) filter.dispose();
        if (compose != null) compose.dispose();
        if (quad != null) quad.dispose();
        filter = compose = null; quad = null;
        width = height = 0;
    }
}

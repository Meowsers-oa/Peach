package net.meowsers.peach.rendering.post;

import net.meowsers.peach.rendering.Framebuffer;

/** Passes write all output pixels. Input and output must be distinct, non-null targets. */
public interface PostProcessingPass extends AutoCloseable {
    void render(Framebuffer input, Framebuffer output);
    void dispose();
    default void cleanup() { dispose(); }
    @Override default void close() { dispose(); }
}

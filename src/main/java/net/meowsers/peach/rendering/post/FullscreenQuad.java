package net.meowsers.peach.rendering.post;

import net.meowsers.peach.rendering.Framebuffer;
import net.meowsers.peach.rendering.Shader;
import org.lwjgl.system.MemoryStack;

import static org.lwjgl.opengl.GL41.*;

/** Fullscreen quad. Restores the GL state touched by the draw. */
public class FullscreenQuad implements AutoCloseable {
    private int vao, vbo;

    public FullscreenQuad() {
        int previousVao = glGetInteger(GL_VERTEX_ARRAY_BINDING), previousBuffer = glGetInteger(GL_ARRAY_BUFFER_BINDING);
        vao = glGenVertexArrays(); vbo = glGenBuffers();
        try {
            glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, new float[]{0, 0, 1, 0, 0, 1, 1, 1}, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * Float.BYTES, 0L);
        } finally {
            glBindVertexArray(previousVao); glBindBuffer(GL_ARRAY_BUFFER, previousBuffer);
        }
    }

    public void render(Shader shader, Framebuffer output, int width, int height, int... textures) {
        if (vao == 0) throw new IllegalStateException("Quad is disposed");
        for (int texture : textures) {
            if (texture == 0 || (output != null && texture == output.getColorTexture())) {
                throw new IllegalArgumentException("Invalid texture or framebuffer feedback loop");
            }
        }
        try (MemoryStack stack = MemoryStack.stackPush()) {
            int draw = glGetInteger(GL_DRAW_FRAMEBUFFER_BINDING), read = glGetInteger(GL_READ_FRAMEBUFFER_BINDING);
            int program = glGetInteger(GL_CURRENT_PROGRAM), previousVao = glGetInteger(GL_VERTEX_ARRAY_BINDING);
            int active = glGetInteger(GL_ACTIVE_TEXTURE);
            var viewport = stack.mallocInt(4);
            glGetIntegerv(GL_VIEWPORT, viewport);
            boolean depth = glIsEnabled(GL_DEPTH_TEST), blend = glIsEnabled(GL_BLEND);
            boolean cull = glIsEnabled(GL_CULL_FACE), scissor = glIsEnabled(GL_SCISSOR_TEST);
            int[] bindings = new int[textures.length];
            int previousUniformBuffer = glGetInteger(GL_UNIFORM_BUFFER_BINDING);
            int[] uniformBindings = new int[shader.getUniformBlockCount()];
            for (int i = 0; i < uniformBindings.length; i++) uniformBindings[i] = glGetIntegeri(GL_UNIFORM_BUFFER_BINDING, i);
            try {
                for (int i = 0; i < textures.length; i++) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    bindings[i] = glGetInteger(GL_TEXTURE_BINDING_2D);
                    glBindTexture(GL_TEXTURE_2D, textures[i]);
                }
                glBindFramebuffer(GL_FRAMEBUFFER, output == null ? 0 : output.getId());
                glViewport(0, 0, width, height);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);
                glDisable(GL_CULL_FACE);
                glDisable(GL_SCISSOR_TEST);
                shader.bind();
                glBindVertexArray(vao);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            } finally {
                for (int i = 0; i < textures.length; i++) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, bindings[i]);
                }
                glActiveTexture(active);
                glBindVertexArray(previousVao);
                glUseProgram(program);
                for (int i = 0; i < uniformBindings.length; i++) glBindBufferBase(GL_UNIFORM_BUFFER, i, uniformBindings[i]);
                glBindBuffer(GL_UNIFORM_BUFFER, previousUniformBuffer);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, read);
                glViewport(viewport.get(0), viewport.get(1), viewport.get(2), viewport.get(3));
                setEnabled(GL_DEPTH_TEST, depth); setEnabled(GL_BLEND, blend);
                setEnabled(GL_CULL_FACE, cull); setEnabled(GL_SCISSOR_TEST, scissor);
            }
        }
    }

    public void render(Shader shader, Framebuffer input, Framebuffer output) {
        if (input == null || output == null) throw new IllegalArgumentException("Pass targets must be non-null");
        render(shader, output, output.getWidth(), output.getHeight(), input.getColorTexture());
    }

    private static void setEnabled(int capability, boolean enabled) {
        if (enabled) glEnable(capability); else glDisable(capability);
    }
    public void dispose() {
        if (vao != 0) glDeleteVertexArrays(vao);
        if (vbo != 0) glDeleteBuffers(vbo);
        vao = vbo = 0;
    }
    @Override public void close() { dispose(); }
}

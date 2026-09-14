package net.meowsers.peach.rendering;

import net.meowsers.peach.utils.PeachException;

import static org.lwjgl.opengl.GL41.*;

/** HDR RGBA16F color target with optional depth. */
public class Framebuffer implements AutoCloseable {
    private int id, color, depth;
    private int width, height;
    private final boolean hasDepth;

    public Framebuffer(int width, int height) { this(width, height, true); }
    public Framebuffer(int width, int height, boolean hasDepth) {
        this.hasDepth = hasDepth;
        resize(width, height);
    }

    public void resize(int width, int height) {
        if (width <= 0 || height <= 0) throw new IllegalArgumentException("Framebuffer dimensions must be positive");
        if (id != 0 && this.width == width && this.height == height) return;
        int oldDraw = glGetInteger(GL_DRAW_FRAMEBUFFER_BINDING), oldRead = glGetInteger(GL_READ_FRAMEBUFFER_BINDING);
        int oldTexture = glGetInteger(GL_TEXTURE_BINDING_2D), oldRenderbuffer = glGetInteger(GL_RENDERBUFFER_BINDING);
        int oldId = id, oldColor = color, oldDepth = depth;
        id = glGenFramebuffers();
        color = glGenTextures();
        depth = 0;
        try {
            glBindFramebuffer(GL_FRAMEBUFFER, id);
            glBindTexture(GL_TEXTURE_2D, color);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, 0L);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
            if (hasDepth) {
                depth = glGenRenderbuffers();
                glBindRenderbuffer(GL_RENDERBUFFER, depth);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
            }
            int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) throw new PeachException("Incomplete framebuffer: 0x" + Integer.toHexString(status));
            this.width = width;
            this.height = height;
            if (oldId != 0) glDeleteFramebuffers(oldId);
            if (oldColor != 0) glDeleteTextures(oldColor);
            if (oldDepth != 0) glDeleteRenderbuffers(oldDepth);
        } catch (RuntimeException e) {
            dispose();
            id = oldId; color = oldColor; depth = oldDepth;
            throw e;
        } finally {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDraw == oldId && oldId != 0 ? id : oldDraw);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, oldRead == oldId && oldId != 0 ? id : oldRead);
            glBindTexture(GL_TEXTURE_2D, oldTexture == oldColor && oldColor != 0 ? color : oldTexture);
            glBindRenderbuffer(GL_RENDERBUFFER, oldRenderbuffer == oldDepth && oldDepth != 0 ? depth : oldRenderbuffer);
        }
    }

    public void bind() {
        if (id == 0) throw new IllegalStateException("Framebuffer is disposed");
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glViewport(0, 0, width, height);
    }
    public int getId() { return id; }
    public int getColorTexture() { return color; }
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public void dispose() {
        if (id != 0) glDeleteFramebuffers(id);
        if (color != 0) glDeleteTextures(color);
        if (depth != 0) glDeleteRenderbuffers(depth);
        id = color = depth = 0;
    }
    public void cleanup() { dispose(); }
    @Override public void close() { dispose(); }
}

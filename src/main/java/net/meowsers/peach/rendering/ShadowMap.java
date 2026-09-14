package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Light;
import net.meowsers.peach.structures.LightType;
import net.meowsers.peach.utils.PeachException;
import org.joml.Matrix4f;
import org.joml.Vector3f;

import static org.lwjgl.opengl.GL41.*;

/** A depth-only target for one directional or spot light. */
public class ShadowMap implements AutoCloseable {
    private int framebuffer, texture;
    private final int size;
    private final Matrix4f matrix = new Matrix4f();

    public ShadowMap(int size) {
        if (size < 1 || size > glGetInteger(GL_MAX_TEXTURE_SIZE)) throw new IllegalArgumentException("Invalid shadow resolution");
        this.size = size;
        int draw = glGetInteger(GL_DRAW_FRAMEBUFFER_BINDING), read = glGetInteger(GL_READ_FRAMEBUFFER_BINDING);
        int previousTexture = glGetInteger(GL_TEXTURE_BINDING_2D);
        try {
            framebuffer = glGenFramebuffers(); texture = glGenTextures();
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0L);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, new float[]{1, 1, 1, 1});
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
            glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw new PeachException("Incomplete shadow framebuffer");
            glClearDepth(1); glClear(GL_DEPTH_BUFFER_BIT);
        } catch (RuntimeException e) { dispose(); throw e; }
        finally {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw); glBindFramebuffer(GL_READ_FRAMEBUFFER, read);
            glBindTexture(GL_TEXTURE_2D, previousTexture);
        }
    }

    public void update(Light light) {
        if (light.type != LightType.DIRECTIONAL && light.type != LightType.SPOT) {
            throw new IllegalArgumentException("Shadow mapping currently supports directional and spot lights");
        }
        if (!(light.shadowNear > 0 && light.shadowFar > light.shadowNear && light.shadowExtent > 0)
                || light.direction.lengthSquared() < 1e-10f) throw new IllegalArgumentException("Invalid shadow camera");
        Vector3f direction = new Vector3f(light.direction).normalize();
        Vector3f up = Math.abs(direction.y) > 0.99f ? new Vector3f(0, 0, 1) : new Vector3f(0, 1, 0);
        if (light.type == LightType.DIRECTIONAL) {
            float extent = light.shadowExtent;
            matrix.setOrtho(-extent, extent, -extent, extent, light.shadowNear, light.shadowFar);
        } else {
            if (!(light.outerCutoff > 0 && light.outerCutoff < 89)) throw new IllegalArgumentException("Spot shadow outer cutoff must be in (0, 89) degrees");
            matrix.setPerspective((float) Math.toRadians(light.outerCutoff * 2), 1, light.shadowNear, light.shadowFar);
        }
        matrix.lookAt(light.position, new Vector3f(light.position).add(direction), up);
    }
    public void begin() {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, size, size);
        glEnable(GL_DEPTH_TEST); glDepthMask(true); glDepthFunc(GL_LESS);
        glDisable(GL_BLEND); glDisable(GL_CULL_FACE); glDisable(GL_SCISSOR_TEST);
        glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(2, 4);
        glClearDepth(1); glClear(GL_DEPTH_BUFFER_BIT);
    }
    public void end() { glDisable(GL_POLYGON_OFFSET_FILL); }
    public void bind(int slot) { glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_2D, texture); }
    public Matrix4f getMatrix() { return new Matrix4f(matrix); }
    public int getSize() { return size; }
    public int getTexture() { return texture; }
    public void dispose() {
        if (framebuffer != 0) glDeleteFramebuffers(framebuffer);
        if (texture != 0) glDeleteTextures(texture);
        framebuffer = texture = 0;
    }
    @Override public void close() { dispose(); }
}

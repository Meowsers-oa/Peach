package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.utils.Log;
import org.joml.Matrix4f;
import org.lwjgl.system.MemoryStack;

import java.nio.ByteBuffer;
import java.util.Map;

import static org.lwjgl.opengl.GL20.*;


public class Renderer {
    private static Shader defaultShader, shader;
    private static Texture whiteTexture;
    private static RenderBatch batch;
    private static int textureSlots;
    private static final Matrix4f viewProjection = new Matrix4f();

    public static void start() {
        if (batch != null) return;
        textureSlots = glGetInteger(GL_MAX_TEXTURE_IMAGE_UNITS);
        try {
            StringBuilder cases = new StringBuilder();
            for (int i = 0; i < textureSlots; i++) {
                cases.append("        case ").append(i).append(": return textureGrad(uTextures[")
                        .append(i).append("], vUv, dx, dy);\n");
            }
            defaultShader = new Shader("shaders/default.vert", "shaders/default.frag", Map.of(
                    "{{TEXTURE_SLOTS}}", Integer.toString(textureSlots),
                    "{{TEXTURE_CASES}}", cases.toString()));
            shader = defaultShader;
            try (MemoryStack stack = MemoryStack.stackPush()) {
                ByteBuffer pixel = stack.malloc(4);
                pixel.put((byte) 255).put((byte) 255).put((byte) 255).put((byte) 255).flip();
                whiteTexture = new Texture(1, 1, pixel);
            }
            batch = new RenderBatch(textureSlots, whiteTexture);
            viewProjection.identity();
            setSamplers();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            Log.message(glGetString(GL_RENDERER) + ": " + textureSlots + " texture slots (slot 0 reserved for white)");
        } catch (RuntimeException e) {
            end();
            throw e;
        }
    }

    public static void addVertices(Vertex... vertices) {
        addVertices(vertices, null, null, null);
    }

    public static void addVertices(Vertex[] vertices, Texture texture) {
        addVertices(vertices, null, texture, null);
    }

    public static void addVertices(Vertex[] vertices, int[] indices) {
        addVertices(vertices, indices, null, null);
    }

    public static void addVertices(Vertex[] vertices, int[] indices, Texture texture) {
        addVertices(vertices, indices, texture, null);
    }

    /** Indices are local to this submission. Null texture uses white; null model uses world positions. */
    public static void addVertices(Vertex[] vertices, int[] indices, Texture texture, Matrix4f model) {
        requireStarted();
        if (!batch.addVertices(vertices, indices, texture, model)) {
            flush();
            batch.addVertices(vertices, indices, texture, model);
        }
    }

    public static void setCamera(Matrix4f view, Matrix4f projection) {
        flush();
        projection.mul(view, viewProjection);
    }

    public static void setViewProjection(Matrix4f matrix) {
        flush();
        viewProjection.set(matrix);
    }

    /** Custom shaders use the batch vertex layout, uViewProjection and uTextures. Null restores the default. */
    public static void setShader(Shader customShader) {
        flush();
        shader = customShader == null ? defaultShader : customShader;
        setSamplers();
    }

    private static void setSamplers() {
        int[] slots = new int[textureSlots];
        for (int i = 0; i < slots.length; i++) slots[i] = i;
        shader.bind();
        shader.setUniform("uTextures", slots);
    }

    /** Call before changing GL state or uniforms, or ending a texture used by queued vertices. */
    public static void flush() {
        requireStarted();
        shader.bind();
        shader.setUniform("uViewProjection", viewProjection);
        batch.flush();
    }

    private static void requireStarted() {
        if (batch == null) throw new IllegalStateException("Renderer.start() must be called with a current OpenGL context");
    }

    public static void end() {
        if (batch != null) batch.end();
        if (whiteTexture != null) whiteTexture.end();
        if (defaultShader != null) defaultShader.end();
        batch = null;
        whiteTexture = null;
        defaultShader = shader = null;
        textureSlots = 0;
    }

    public static int getTextureSlots() {
        return textureSlots;
    }
}

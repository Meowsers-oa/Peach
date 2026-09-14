package net.meowsers.peach.rendering;

import net.meowsers.peach.utils.PeachException;
import org.lwjgl.stb.STBImage;
import org.lwjgl.system.MemoryStack;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import static org.lwjgl.opengl.GL41.*;

/** Create and dispose textures on the thread with the current OpenGL context. */
public class Texture implements AutoCloseable {
    public enum Filter { LINEAR, NEAREST }

    private int id;
    private int width, height;

    public Texture(String path) {
        this(path, Filter.LINEAR, true);
    }

    public Texture(String path, Filter filter, boolean mipmaps) {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer w = stack.mallocInt(1), h = stack.mallocInt(1), channels = stack.mallocInt(1);
            // Assimp flips UVs; do not change STB's process-wide flip setting.
            ByteBuffer pixels = STBImage.stbi_load(path, w, h, channels, 4);
            if (pixels == null) throw new PeachException("Cannot load texture " + path + ": " + STBImage.stbi_failure_reason());
            try { upload(pixels, w.get(0), h.get(0), 4, filter, mipmaps); }
            finally { STBImage.stbi_image_free(pixels); }
        }
    }

    public Texture(ByteBuffer pixels, int width, int height, int channels, Filter filter, boolean mipmaps) {
        upload(pixels, width, height, channels, filter, mipmaps);
    }

    public static Texture fromEncoded(ByteBuffer data) {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer w = stack.mallocInt(1), h = stack.mallocInt(1), c = stack.mallocInt(1);
            ByteBuffer pixels = STBImage.stbi_load_from_memory(data, w, h, c, 4);
            if (pixels == null) throw new PeachException("Cannot decode texture: " + STBImage.stbi_failure_reason());
            try { return new Texture(pixels, w.get(0), h.get(0), 4, Filter.LINEAR, true); }
            finally { STBImage.stbi_image_free(pixels); }
        }
    }

    private void upload(ByteBuffer pixels, int width, int height, int channels, Filter filter, boolean mipmaps) {
        if (width <= 0 || height <= 0 || (channels != 3 && channels != 4)
                || !pixels.isDirect() || pixels.remaining() < (long) width * height * channels) {
            throw new IllegalArgumentException("Expected a direct RGB/RGBA pixel buffer of the requested size");
        }
        this.width = width;
        this.height = height;
        int previous = glGetInteger(GL_TEXTURE_BINDING_2D), alignment = glGetInteger(GL_UNPACK_ALIGNMENT);
        id = glGenTextures();
        try {
            glBindTexture(GL_TEXTURE_2D, id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            int format = channels == 4 ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, channels == 4 ? GL_RGBA8 : GL_RGB8, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
            int sampling = filter == Filter.LINEAR ? GL_LINEAR : GL_NEAREST;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps
                    ? (filter == Filter.LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) : sampling);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if (mipmaps) glGenerateMipmap(GL_TEXTURE_2D);
        } catch (RuntimeException e) {
            dispose();
            throw e;
        } finally {
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
            glBindTexture(GL_TEXTURE_2D, previous);
        }
    }

    public void bind(int slot) {
        if (id == 0) throw new IllegalStateException("Texture is disposed");
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    public void bind() { bind(0); }
    public int getId() { return id; }
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public void dispose() { if (id != 0) glDeleteTextures(id); id = 0; }
    public void cleanup() { dispose(); }
    @Override public void close() { dispose(); }
}

package net.meowsers.peach.rendering;

import net.meowsers.peach.utils.PeachException;
import org.lwjgl.system.MemoryStack;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.lwjgl.opengl.GL13.*;
import static org.lwjgl.opengl.GL30.glGenerateMipmap;
import static org.lwjgl.stb.STBImage.*;
import static org.lwjgl.system.MemoryUtil.*;

public class Texture {
    private int id;
    private int width, height;

    public Texture(String path) {
        String resource = path.startsWith("/") ? path : "/" + path;
        byte[] bytes;
        try (InputStream stream = Files.isRegularFile(Path.of(path))
                ? Files.newInputStream(Path.of(path)) : Texture.class.getResourceAsStream(resource)) {
            if (stream == null) throw new PeachException("Texture not found: " + path);
            bytes = stream.readAllBytes();
        } catch (IOException e) {
            throw new PeachException("Could not read texture: " + path, e);
        }

        ByteBuffer encoded = memAlloc(bytes.length);
        try {
            encoded.put(bytes).flip();
            decode(encoded);
        } finally {
            memFree(encoded);
        }
    }

    /** Encoded image bytes, such as an embedded PNG or JPEG. The caller owns the buffer. */
    public Texture(ByteBuffer encoded) {
        if (!encoded.isDirect()) throw new IllegalArgumentException("Image buffer must be direct");
        decode(encoded);
    }

    private void decode(ByteBuffer encoded) {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer w = stack.mallocInt(1);
            IntBuffer h = stack.mallocInt(1);
            IntBuffer channels = stack.mallocInt(1);
            // UV (0, 0) is the bottom-left of the image.
            stbi_set_flip_vertically_on_load_thread(1);
            ByteBuffer pixels = stbi_load_from_memory(encoded, w, h, channels, 4);
            if (pixels == null) throw new PeachException("Could not decode texture: " + stbi_failure_reason());
            try {
                upload(w.get(0), h.get(0), pixels);
            } finally {
                stbi_image_free(pixels);
            }
        }
    }

    /** RGBA bytes, starting at the buffer's current position. Rows run bottom to top. */
    public Texture(int width, int height, ByteBuffer pixels) {
        if (width <= 0 || height <= 0 || !pixels.isDirect() || (long) width * height * 4 > pixels.remaining()) {
            throw new IllegalArgumentException("Texture needs positive dimensions and a direct RGBA buffer");
        }
        upload(width, height, pixels);
    }

    private void upload(int width, int height, ByteBuffer pixels) {
        int maxSize = glGetInteger(GL_MAX_TEXTURE_SIZE);
        if (width > maxSize || height > maxSize) throw new IllegalArgumentException("Texture exceeds GPU size limit: " + maxSize);
        this.width = width;
        this.height = height;
        int previous = glGetInteger(GL_TEXTURE_BINDING_2D);
        id = glGenTextures();
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, previous);
    }

    public void bind(int slot) {
        if (id == 0) throw new IllegalStateException("Texture has ended");
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    public void end() {
        if (id == 0) return;
        glDeleteTextures(id);
        id = 0;
    }

    public int getId() {
        return id;
    }
    public int getWidth() {
        return width;
    }
    public int getHeight() {
        return height;
    }
}

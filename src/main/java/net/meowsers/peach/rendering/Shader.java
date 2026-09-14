package net.meowsers.peach.rendering;

import net.meowsers.peach.utils.PeachException;
import org.joml.Matrix4f;
import org.joml.Vector2f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.lwjgl.system.MemoryStack;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

import static org.lwjgl.opengl.GL20.*;

public class Shader {
    private int id;
    private final Map<String, Integer> uniforms = new HashMap<>();

    public Shader(String vertexPath, String fragmentPath) {
        this(vertexPath, fragmentPath, Map.of());
    }

    public Shader(String vertexPath, String fragmentPath, Map<String, String> replacements) {
        String vertexSource = read(vertexPath);
        String fragmentSource = read(fragmentPath);
        for (var replacement : replacements.entrySet()) {
            vertexSource = vertexSource.replace(replacement.getKey(), replacement.getValue());
            fragmentSource = fragmentSource.replace(replacement.getKey(), replacement.getValue());
        }

        int vertex = compile(GL_VERTEX_SHADER, vertexSource, vertexPath);
        int fragment = 0;
        try {
            fragment = compile(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
            id = glCreateProgram();
            glAttachShader(id, vertex);
            glAttachShader(id, fragment);
            glLinkProgram(id);
            if (glGetProgrami(id, GL_LINK_STATUS) == GL_FALSE) {
                throw new PeachException("Could not link shader: " + glGetProgramInfoLog(id));
            }
        } catch (RuntimeException e) {
            end();
            throw e;
        } finally {
            glDeleteShader(vertex);
            if (fragment != 0) glDeleteShader(fragment);
        }
    }

    private static String read(String path) {
        String resource = path.startsWith("/") ? path : "/" + path;
        try (InputStream stream = Shader.class.getResourceAsStream(resource)) {
            if (stream == null) throw new PeachException("Shader not found: " + path);
            return new String(stream.readAllBytes(), StandardCharsets.UTF_8);
        } catch (IOException e) {
            throw new PeachException("Could not read shader: " + path, e);
        }
    }

    private static int compile(int type, String source, String path) {
        int shader = glCreateShader(type);
        glShaderSource(shader, source);
        glCompileShader(shader);
        if (glGetShaderi(shader, GL_COMPILE_STATUS) == GL_FALSE) {
            String error = glGetShaderInfoLog(shader);
            glDeleteShader(shader);
            throw new PeachException("Could not compile shader " + path + ":\n" + error);
        }
        return shader;
    }

    public void bind() {
        if (id == 0) throw new IllegalStateException("Shader has ended");
        glUseProgram(id);
    }

    // Bind this shader before setting uniforms.
    public void setUniform(String name, int value) {
        glUniform1i(getUniform(name), value);
    }

    public void setUniform(String name, int[] values) {
        glUniform1iv(getUniform(name), values);
    }

    public void setUniform(String name, float value) {
        glUniform1f(getUniform(name), value);
    }

    public void setUniform(String name, Vector2f value) {
        glUniform2f(getUniform(name), value.x, value.y);
    }

    public void setUniform(String name, Vector3f value) {
        glUniform3f(getUniform(name), value.x, value.y, value.z);
    }

    public void setUniform(String name, Vector4f value) {
        glUniform4f(getUniform(name), value.x, value.y, value.z, value.w);
    }

    public void setUniform(String name, Matrix4f value) {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            glUniformMatrix4fv(getUniform(name), false, value.get(stack.mallocFloat(16)));
        }
    }

    private int getUniform(String name) {
        if (id == 0) throw new IllegalStateException("Shader has ended");
        return uniforms.computeIfAbsent(name, key -> glGetUniformLocation(id, key));
    }

    public void end() {
        if (id == 0) return;
        if (glGetInteger(GL_CURRENT_PROGRAM) == id) glUseProgram(0);
        glDeleteProgram(id);
        id = 0;
        uniforms.clear();
    }

    public int getId() {
        return id;
    }
}

package net.meowsers.peach.rendering;

import net.meowsers.peach.utils.PeachException;
import net.meowsers.peach.utils.SetupSlang;
import org.joml.Matrix4f;
import org.joml.Vector2f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.lwjgl.system.MemoryStack;
import org.lwjgl.system.MemoryUtil;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.lwjgl.opengl.GL41.*;

/** Slang uniforms use reflected std140 blocks; ordinary GLSL uniforms remain supported. */
public class Shader implements AutoCloseable {
    private int id;
    private final Map<String, Uniform> uniforms = new HashMap<>();
    private final List<UniformBlock> blocks = new ArrayList<>();
    private record Uniform(int location, int block, int offset, int matrixStride) { }
    private static class UniformBlock {
        int id;
        ByteBuffer data;
        boolean dirty = true;
    }

    public Shader(String vertexSource, String fragmentSource) {
        int vertex = 0, fragment = 0;
        try {
            vertex = compile(GL_VERTEX_SHADER, vertexSource);
            fragment = compile(GL_FRAGMENT_SHADER, fragmentSource);
            id = glCreateProgram();
            glAttachShader(id, vertex); glAttachShader(id, fragment);
            glLinkProgram(id);
            if (glGetProgrami(id, GL_LINK_STATUS) == GL_FALSE) throw new PeachException("Shader link failed: " + glGetProgramInfoLog(id));
            reflectUniforms();
        } catch (RuntimeException e) { dispose(); throw e; }
        finally {
            if (vertex != 0) glDeleteShader(vertex);
            if (fragment != 0) glDeleteShader(fragment);
        }
    }

    private void reflectUniforms() {
        int previous = glGetInteger(GL_UNIFORM_BUFFER_BINDING);
        try (MemoryStack stack = MemoryStack.stackPush()) {
            int count = glGetProgrami(id, GL_ACTIVE_UNIFORM_BLOCKS);
            for (int i = 0; i < count; i++) {
                UniformBlock block = new UniformBlock();
                blocks.add(block);
                block.data = MemoryUtil.memCalloc(glGetActiveUniformBlocki(id, i, GL_UNIFORM_BLOCK_DATA_SIZE));
                block.id = glGenBuffers();
                glBindBuffer(GL_UNIFORM_BUFFER, block.id);
                glBufferData(GL_UNIFORM_BUFFER, block.data.capacity(), GL_DYNAMIC_DRAW);
                glUniformBlockBinding(id, i, i);
            }
            var size = stack.mallocInt(1);
            var type = stack.mallocInt(1);
            for (int i = 0; i < glGetProgrami(id, GL_ACTIVE_UNIFORMS); i++) {
                String name = glGetActiveUniform(id, i, size, type);
                int block = glGetActiveUniformsi(id, i, GL_UNIFORM_BLOCK_INDEX);
                int offset = glGetActiveUniformsi(id, i, GL_UNIFORM_OFFSET);
                int stride = glGetActiveUniformsi(id, i, GL_UNIFORM_ARRAY_STRIDE);
                int matrixStride = glGetActiveUniformsi(id, i, GL_UNIFORM_MATRIX_STRIDE);
                for (int element = 0; element < size.get(0); element++) {
                    String indexed = size.get(0) > 1 ? name.replaceFirst("\\[0\\]", "[" + element + "]") : name;
                    Uniform uniform = new Uniform(glGetUniformLocation(id, indexed), block, offset + element * stride, matrixStride);
                    uniforms.put(indexed, uniform);
                    // Slang disambiguates symbols with _0 suffixes; expose source-level names.
                    String alias = indexed.replaceFirst("^(block_GlobalParams_\\d+|globalParams_\\d+)\\.", "")
                            .replaceAll("_\\d+(?=\\[|\\.|$)", "");
                    uniforms.putIfAbsent(alias, uniform);
                }
            }
        } finally { glBindBuffer(GL_UNIFORM_BUFFER, previous); }
    }

    public static Shader fromResources(String vertex, String fragment) {
        return new Shader(resource(vertex), resource(fragment));
    }
    public static Shader fromSlang(Path source, String vertexEntry, String fragmentEntry) {
        return new Shader(SetupSlang.compileGlsl(source, vertexEntry, SetupSlang.Stage.VERTEX),
                SetupSlang.compileGlsl(source, fragmentEntry, SetupSlang.Stage.FRAGMENT));
    }
    public static String resource(String path) {
        try (var input = Shader.class.getResourceAsStream(path)) {
            if (input == null) throw new PeachException("Shader resource missing: " + path + "; run compileShaders");
            return new String(input.readAllBytes(), StandardCharsets.UTF_8);
        } catch (IOException e) { throw new PeachException(e); }
    }
    private static int compile(int type, String source) {
        int shader = glCreateShader(type);
        glShaderSource(shader, source); glCompileShader(shader);
        if (glGetShaderi(shader, GL_COMPILE_STATUS) == GL_FALSE) {
            String message = glGetShaderInfoLog(shader); glDeleteShader(shader);
            throw new PeachException("Shader compile failed: " + message);
        }
        return shader;
    }

    public void bind() {
        if (id == 0) throw new IllegalStateException("Shader is disposed");
        glUseProgram(id);
        int previous = glGetInteger(GL_UNIFORM_BUFFER_BINDING);
        for (int i = 0; i < blocks.size(); i++) {
            UniformBlock block = blocks.get(i);
            glBindBufferBase(GL_UNIFORM_BUFFER, i, block.id);
            if (block.dirty) { glBufferSubData(GL_UNIFORM_BUFFER, 0, block.data); block.dirty = false; }
        }
        glBindBuffer(GL_UNIFORM_BUFFER, previous);
    }

    private Uniform uniform(String name) {
        if (id == 0) throw new IllegalStateException("Shader is disposed");
        return uniforms.get(name); // Optimized-out parameters intentionally do nothing.
    }
    private ByteBuffer data(Uniform uniform) {
        UniformBlock block = blocks.get(uniform.block);
        block.dirty = true;
        return block.data;
    }
    public void set(String name, int value) {
        Uniform u = uniform(name); if (u == null) return;
        if (u.block < 0) glProgramUniform1i(id, u.location, value); else data(u).putInt(u.offset, value);
    }
    public void set(String name, float value) {
        Uniform u = uniform(name); if (u == null) return;
        if (u.block < 0) glProgramUniform1f(id, u.location, value); else data(u).putFloat(u.offset, value);
    }
    public void set(String name, Vector2f v) {
        Uniform u = uniform(name); if (u == null) return;
        if (u.block < 0) glProgramUniform2f(id, u.location, v.x, v.y);
        else data(u).putFloat(u.offset, v.x).putFloat(u.offset + 4, v.y);
    }
    public void set(String name, Vector3f v) {
        Uniform u = uniform(name); if (u == null) return;
        if (u.block < 0) glProgramUniform3f(id, u.location, v.x, v.y, v.z);
        else data(u).putFloat(u.offset, v.x).putFloat(u.offset + 4, v.y).putFloat(u.offset + 8, v.z);
    }
    public void set(String name, Vector4f v) {
        Uniform u = uniform(name); if (u == null) return;
        if (u.block < 0) glProgramUniform4f(id, u.location, v.x, v.y, v.z, v.w);
        else data(u).putFloat(u.offset, v.x).putFloat(u.offset + 4, v.y).putFloat(u.offset + 8, v.z).putFloat(u.offset + 12, v.w);
    }
    public void set(String name, Matrix4f value) {
        Uniform u = uniform(name); if (u == null) return;
        if (u.block >= 0) {
            // Slang is compiled with column-major host layout. Its emitted GLSL
            // matrix is transposed, including mul(), so preserve the host bytes.
            if (u.matrixStride != 16) throw new PeachException("Unexpected matrix layout for " + name);
            value.get(u.offset, data(u));
        } else try (MemoryStack stack = MemoryStack.stackPush()) {
            glProgramUniformMatrix4fv(id, u.location, false, value.get(stack.mallocFloat(16)));
        }
    }
    public int getUniformBlockCount() { return blocks.size(); }
    public int getId() { return id; }
    public void dispose() {
        for (UniformBlock block : blocks) {
            if (block.id != 0) glDeleteBuffers(block.id);
            MemoryUtil.memFree(block.data);
        }
        blocks.clear(); uniforms.clear();
        if (id != 0) glDeleteProgram(id);
        id = 0;
    }
    public void cleanup() { dispose(); }
    @Override public void close() { dispose(); }
}

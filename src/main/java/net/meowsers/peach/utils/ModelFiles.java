package net.meowsers.peach.utils;

import org.lwjgl.assimp.AIFile;
import org.lwjgl.assimp.AIFileIO;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

import static org.lwjgl.assimp.Assimp.*;
import static org.lwjgl.system.MemoryUtil.*;

/** Assimp file access for filesystem paths and classpath resources, including files inside JARs. */
class ModelFiles implements AutoCloseable {
    final AIFileIO io = AIFileIO.calloc();
    private final Map<Long, ByteBuffer> files = new HashMap<>();

    ModelFiles() {
        io.OpenProc((fileIO, name, mode) -> open(memUTF8(name)));
        io.CloseProc((fileIO, file) -> close(file));
    }

    private long open(String name) {
        String path = name.replace('\\', '/');
        byte[] bytes;
        try (InputStream stream = Files.isRegularFile(Path.of(path)) ? Files.newInputStream(Path.of(path))
                : ModelFiles.class.getResourceAsStream(path.startsWith("/") ? path : "/" + path)) {
            if (stream == null) return NULL;
            bytes = stream.readAllBytes();
        } catch (IOException | RuntimeException e) {
            return NULL;
        }
        ByteBuffer data = memAlloc(bytes.length);
        data.put(bytes).flip();
        AIFile file = AIFile.calloc();
        file.ReadProc((handle, destination, size, count) -> {
            if (size <= 0 || count <= 0) return 0;
            long read = Math.min(count, data.remaining() / size);
            long length = read * size;
            memCopy(memAddress(data), destination, length);
            data.position(data.position() + (int) length);
            return read;
        });
        file.TellProc(handle -> data.position());
        file.FileSizeProc(handle -> data.limit());
        file.SeekProc((handle, offset, origin) -> {
            long position = switch (origin) {
                case aiOrigin_SET -> offset;
                case aiOrigin_CUR -> data.position() + offset;
                case aiOrigin_END -> data.limit() - offset;
                default -> -1;
            };
            if (position < 0 || position > data.limit()) return aiReturn_FAILURE;
            data.position((int) position);
            return aiReturn_SUCCESS;
        });
        files.put(file.address(), data);
        return file.address();
    }

    private void close(long address) {
        ByteBuffer data = files.remove(address);
        if (data == null) return;
        AIFile file = AIFile.create(address);
        file.ReadProc().free();
        file.TellProc().free();
        file.FileSizeProc().free();
        file.SeekProc().free();
        file.free();
        memFree(data);
    }

    @Override
    public void close() {
        for (long address : files.keySet().toArray(Long[]::new)) close(address);
        io.OpenProc().free();
        io.CloseProc().free();
        io.free();
    }
}

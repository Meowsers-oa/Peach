package net.meowsers.peach.utils;

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.time.Duration;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.concurrent.TimeUnit;
import java.util.zip.ZipFile;

/** Compiles the Slang sources to OpenGL 4.1 at build time or runtime. */
public final class SetupSlang {
    public static final String VERSION = "2025.24.3";
    public static final Path DIRECTORY = Path.of("bin", "slang").toAbsolutePath();
    public enum Stage { VERTEX, FRAGMENT, GEOMETRY, COMPUTE }
    public enum Target { GLSL, SPIRV }

    private SetupSlang() { }

    public static void main(String[] args) {
        System.out.println("Slang compiler: " + (System.getProperty("peach.slangc") == null ? install() : getCompiler()));
    }

    public static String platform(String os, String arch) {
        if (!os.toLowerCase(Locale.ROOT).contains("mac")) {
            throw new IllegalArgumentException("Automatic Slang setup supports macOS; set -Dpeach.slangc=/path/to/slangc on other systems");
        }
        return switch (arch.toLowerCase(Locale.ROOT)) {
            case "aarch64", "arm64" -> "macos-aarch64";
            case "x86_64", "amd64" -> "macos-x86_64";
            default -> throw new IllegalArgumentException("Unsupported macOS architecture: " + arch);
        };
    }

    public static Path getCompiler() {
        String override = System.getProperty("peach.slangc");
        Path binary = override == null ? DIRECTORY.resolve("bin/slangc") : Path.of(override).toAbsolutePath();
        if (!Files.isExecutable(binary)) throw new PeachException("Slang compiler missing. Run ./gradlew setupSlang or set -Dpeach.slangc=/path/to/slangc");
        return binary;
    }

    public static synchronized Path install() {
        String platform = platform(System.getProperty("os.name"), System.getProperty("os.arch"));
        Path marker = DIRECTORY.resolve(".peach-version");
        String expected = VERSION + " " + platform;
        Path staging = null, archive = null;
        try {
            if (Files.isRegularFile(marker) && Files.readString(marker).equals(expected)
                    && Files.isExecutable(DIRECTORY.resolve("bin/slangc"))) return getCompiler();
            if (Files.exists(DIRECTORY)) throw new PeachException("Existing Slang directory has a different version; move it aside before setup: " + DIRECTORY);
            Files.createDirectories(DIRECTORY.getParent());
            staging = Files.createTempDirectory(DIRECTORY.getParent(), "slang-install-");
            archive = Files.createTempFile("peach-slang-", ".zip");
            URI url = URI.create("https://github.com/shader-slang/slang/releases/download/v" + VERSION
                    + "/slang-" + VERSION + "-" + platform + ".zip");
            try (HttpClient client = HttpClient.newBuilder().followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(30)).build()) {
                HttpResponse<Path> response = client.send(HttpRequest.newBuilder(url).timeout(Duration.ofMinutes(5)).build(),
                        HttpResponse.BodyHandlers.ofFile(archive));
                if (response.statusCode() != 200) throw new IOException("Slang download HTTP " + response.statusCode());
            }
            // Validate paths before native extraction, which preserves dylib symlinks and executable modes.
            try (ZipFile zip = new ZipFile(archive.toFile())) {
                var entries = zip.entries();
                while (entries.hasMoreElements()) {
                    String name = entries.nextElement().getName();
                    if (!staging.resolve(name).normalize().startsWith(staging)) throw new IOException("Unsafe archive path: " + name);
                }
            }
            run(List.of("/usr/bin/ditto", "-x", "-k", archive.toString(), staging.toString()));
            Path binary = staging.resolve("bin/slangc");
            if (!Files.isRegularFile(binary) || !binary.toFile().setExecutable(true)) throw new IOException("Archive is missing bin/slangc");
            run(List.of(binary.toString(), "-version"));
            Files.writeString(staging.resolve(".peach-version"), expected);
            Files.move(staging, DIRECTORY, StandardCopyOption.ATOMIC_MOVE);
            return getCompiler();
        } catch (IOException e) {
            throw new PeachException("Slang installation failed", e);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new PeachException("Slang installation interrupted", e);
        } finally {
            deleteTemporary(staging);
            deleteTemporary(archive);
        }
    }

    public static Path compile(Path source, Path output, String entry, Stage stage, Target target) {
        if (target == Target.GLSL && stage == Stage.COMPUTE) throw new IllegalArgumentException("OpenGL 4.1 does not support compute shaders");
        try {
            Files.createDirectories(output.toAbsolutePath().getParent());
            run(List.of(getCompiler().toString(), source.toAbsolutePath().toString(), "-entry", entry,
                    "-stage", stage.name().toLowerCase(Locale.ROOT), "-target", target == Target.GLSL ? "glsl" : "spirv",
                    "-matrix-layout-column-major", "-profile", target == Target.GLSL ? "glsl_410" : "spirv_1_0", "-o", output.toAbsolutePath().toString()));
            if (target == Target.GLSL) Files.writeString(output, linkVaryings(toGlsl410(Files.readString(output)), stage));
            return output;
        } catch (IOException e) { throw new PeachException("Slang compilation failed: " + source, e); }
        catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new PeachException("Slang compilation interrupted", e);
        }
    }

    /**
     * Slang emits Vulkan-oriented defaults even for glsl_410. Remove only those
     * defaults and descriptor binding metadata; bind samplers/UBOs through OpenGL.
     * This is a raster-shader subset, not an emulation of newer GPU features.
     */
    public static String toGlsl410(String source) {
        String glsl = source.replaceFirst("(?m)^#version[^\\r\\n]*", "#version 410 core")
                .replaceAll("(?m)^layout\\(row_major\\) buffer;\\s*", "");
        if (!glsl.startsWith("#version 410 core")) throw new PeachException("Slang output is missing a GLSL version");
        if (Pattern.compile("(?m)^\\s*#extension|\\b(buffer|atomic_uint|[iu]?image[123]D|subpassInput)\\b").matcher(glsl).find()) {
            throw new PeachException("Slang shader requires features outside OpenGL 4.1; use a raster shader without storage buffers, images, or extensions");
        }
        Matcher layouts = Pattern.compile("layout\\s*\\(([^)]*)\\)").matcher(glsl);
        StringBuilder result = new StringBuilder();
        while (layouts.find()) {
            List<String> qualifiers = new java.util.ArrayList<>();
            for (String qualifier : layouts.group(1).split(",")) {
                String value = qualifier.trim();
                if (value.matches("(binding|set)\\s*=\\s*\\d+")) continue;
                if (value.matches("(offset|align|component|index|local_size_\\w+|push_constant|std430|scalar).*")) {
                    throw new PeachException("Unsupported OpenGL 4.1 layout: " + value);
                }
                qualifiers.add(value);
            }
            layouts.appendReplacement(result, Matcher.quoteReplacement(qualifiers.isEmpty()
                    ? "" : "layout(" + String.join(", ", qualifiers) + ")"));
        }
        layouts.appendTail(result);
        return result.toString().replaceAll("\\b(flat|smooth|noperspective)\\s+(layout\\s*\\([^)]*\\))", "$2 $1");
    }

    // Apple's monolithic program linker matches stage interfaces by name. Slang
    // emits different names per entry point, so give matching locations stable names.
    private static String linkVaryings(String source, Stage stage) {
        if (stage != Stage.VERTEX && stage != Stage.FRAGMENT) return source;
        String direction = stage == Stage.VERTEX ? "out" : "in";
        Matcher matcher = Pattern.compile("layout\\(location\\s*=\\s*(\\d+)\\)\\s*(?:flat\\s+)?"
                + direction + "\\s+\\w+\\s+(\\w+)\\s*;").matcher(source);
        Map<String, String> names = new java.util.LinkedHashMap<>();
        while (matcher.find()) names.put(matcher.group(2), "peachVarying" + matcher.group(1));
        for (var name : names.entrySet()) source = source.replaceAll("\\b" + Pattern.quote(name.getKey()) + "\\b", name.getValue());
        return source;
    }

    public static String compileGlsl(Path source, String entry, Stage stage) {
        Path output = null;
        try {
            output = Files.createTempFile("peach-shader-", ".glsl");
            compile(source, output, entry, stage, Target.GLSL);
            return Files.readString(output);
        } catch (IOException e) { throw new PeachException(e); }
        finally { deleteTemporary(output); }
    }

    private static void run(List<String> command) throws IOException, InterruptedException {
        Path log = Files.createTempFile("peach-slang-log-", ".txt");
        Process process = null;
        try {
            process = new ProcessBuilder(command).redirectErrorStream(true).redirectOutput(log.toFile()).start();
            if (!process.waitFor(120, TimeUnit.SECONDS)) throw new IOException("Process timed out: " + command.getFirst());
            if (process.exitValue() != 0) throw new IOException(Files.readString(log));
        } finally {
            if (process != null && process.isAlive()) process.destroyForcibly();
            Files.deleteIfExists(log);
        }
    }

    private static void deleteTemporary(Path path) {
        if (path == null || !Files.exists(path)) return;
        try (var files = Files.walk(path)) {
            for (Path file : files.sorted(Comparator.reverseOrder()).toList()) Files.deleteIfExists(file);
        } catch (IOException e) { Log.warning("Could not remove temporary path " + path + ": " + e.getMessage()); }
    }
}

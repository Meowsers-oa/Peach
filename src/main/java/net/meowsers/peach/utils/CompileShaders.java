package net.meowsers.peach.utils;

import java.nio.file.Path;

/** Build-time entry point. GLSL files are compiler output, never engine shader sources. */
public final class CompileShaders {
    private CompileShaders() { }

    public static void main(String[] args) {
        Path source = Path.of(args[0]), output = Path.of(args[1]);
        for (String name : new String[]{"batch", "shadow"}) {
            compile(source, output, name, "vertexMain", SetupSlang.Stage.VERTEX, name + ".vert");
            compile(source, output, name, "fragmentMain", SetupSlang.Stage.FRAGMENT, name + ".frag");
        }
        compile(source, output, "fullscreen", "vertexMain", SetupSlang.Stage.VERTEX, "fullscreen.vert");
        for (String name : new String[]{"copy", "invert", "bloom-filter", "bloom-compose", "tonemap"}) {
            compile(source, output, name, "fragmentMain", SetupSlang.Stage.FRAGMENT, name + ".frag");
        }
    }

    private static void compile(Path source, Path output, String name, String entry, SetupSlang.Stage stage, String file) {
        SetupSlang.compile(source.resolve(name + ".slang"), output.resolve(file), entry, stage, SetupSlang.Target.GLSL);
    }
}

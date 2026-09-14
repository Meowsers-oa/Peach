package net.meowsers.peach;

import net.meowsers.peach.core.Peach;
import net.meowsers.peach.core.PeachLevel;
import net.meowsers.peach.rendering.Framebuffer;
import net.meowsers.peach.rendering.Mesh;
import net.meowsers.peach.rendering.Model;
import net.meowsers.peach.rendering.RenderBatch;
import net.meowsers.peach.rendering.Renderer;
import net.meowsers.peach.rendering.Shader;
import net.meowsers.peach.rendering.Texture;
import net.meowsers.peach.rendering.post.BloomPass;
import net.meowsers.peach.rendering.post.InvertPass;
import net.meowsers.peach.structures.Color;
import net.meowsers.peach.structures.Light;
import net.meowsers.peach.structures.LightType;
import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.structures.WindowParams;
import net.meowsers.peach.utils.SetupSlang;
import org.joml.Matrix4f;
import net.meowsers.peach.structures.Transform;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.lwjgl.system.MemoryStack;

import java.nio.FloatBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL41.*;

/** Run with ./gradlew graphicsSmoke (macOS needs the JVM's first thread). */
public class GraphicsSmoke {
    private static final Vertex[] TRIANGLE = {new Vertex(new Vector3f(-1, -1, 0)),
            new Vertex(new Vector3f(1, -1, 0)), new Vertex(new Vector3f(0, 1, 0))};
    private static final int[] INDICES = {0, 1, 2};

    public static void main(String[] args) throws Exception {
        if (!glfwInit()) throw new AssertionError("GLFW initialization failed");
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        long window = glfwCreateWindow(64, 64, "Peach graphics verification", 0, 0);
        if (window == 0) throw new AssertionError("OpenGL context creation failed");
        try {
            glfwMakeContextCurrent(window);
            org.lwjgl.opengl.GL.createCapabilities();
            System.out.println("Driver: " + glGetString(GL_VERSION) + " / " + glGetString(GL_RENDERER));
            batchesAndPasses();
            modelImport();
            hierarchyAndEmbeddedTexture();
            slang();
            Renderer.init();
            Renderer.addPostProcessing(new BloomPass());
            Renderer.addPostProcessing(new InvertPass());
            Light light = new Light(LightType.DIRECTIONAL);
            light.direction.set(0, 0, -1);
            Renderer.addLight(light);
            Renderer.beginFrame(64, 64, Color.Black);
            Renderer.addMesh(new Mesh(TRIANGLE, INDICES), new Transform(new Vector3f(100, 0, 0), new Vector3f(1), new Vector3f()));
            Renderer.addMesh(new Mesh(TRIANGLE, INDICES));
            Renderer.endFrame();
            require(Renderer.getCulledMeshes() == 1 && Renderer.getDrawCalls() == 1, "Renderer culling/statistics");
            Renderer.beginFrame(37, 19, Color.Black);
            Renderer.addVertices(TRIANGLE, INDICES, new Vector4f(3, 2, 1, 1));
            Renderer.endFrame();
            checkGl("Renderer and resize");
            shadowPixels();
        } finally {
            Renderer.dispose();
            glfwDestroyWindow(window);
            org.lwjgl.opengl.GL.setCapabilities(null);
            glfwTerminate();
        }
        lifecycle();
        gameScene();
        System.out.println("Graphics smoke passed: Slang shaders, shadow pixels, main Game/MyLevel scene, HDR bloom, loaders, resizing and lifecycle.");
    }

    private static void batchesAndPasses() {
        try (Framebuffer source = new Framebuffer(32, 32); Framebuffer output = new Framebuffer(32, 32, false);
             Shader shader = Shader.fromResources("/shaders/batch.vert", "/shaders/batch.frag");
             RenderBatch batch = new RenderBatch(3, 3, 1);
             InvertPass invert = new InvertPass(); BloomPass bloom = new BloomPass()) {
            shader.set("viewProjection", new Matrix4f()); shader.set("lightCount", 0);
            for (int i = 0; i < 8; i++) shader.set("textures[" + i + "]", 0);
            source.bind();
            glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shader.bind();
            // Six vertices exceed both capacities, exercising the triangle split path.
            Vertex[] oversized = {TRIANGLE[0], TRIANGLE[1], TRIANGLE[2], TRIANGLE[0], TRIANGLE[1], TRIANGLE[2]};
            batch.add(oversized, new int[]{0, 1, 2, 3, 4, 5}, null, new Vector4f(1, 0, 0, 1), new Transform());
            batch.flush();
            require(batch.getDrawCalls() == 2, "Oversized batch did not flush");
            closeTo(pixel(source, 16, 16)[0], 1, "Triangle rasterization");
            invert.render(source, output);
            float[] inverted = pixel(output, 16, 16);
            closeTo(inverted[0], 0, "Invert red"); closeTo(inverted[1], 1, "Invert green");
            // Uniform HDR scene: the normalized blur must preserve (4 - threshold) at every level.
            source.bind(); glClearColor(4, 4, 4, 1); glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND); glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE);
            glViewport(2, 3, 11, 12);
            bloom.intensity = 0.5f;
            bloom.render(source, output);
            require(glIsEnabled(GL_BLEND) && glIsEnabled(GL_DEPTH_TEST) && glIsEnabled(GL_CULL_FACE), "Post state restoration");
            closeTo(pixel(output, 16, 16)[0], 5.5f, "HDR bloom composition");
            // A localized bright pixel must spread into neighboring dark pixels.
            source.bind(); glDisable(GL_BLEND); glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
            glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_SCISSOR_TEST); glScissor(14, 14, 4, 4);
            glClearColor(8, 8, 8, 1); glClear(GL_COLOR_BUFFER_BIT); glDisable(GL_SCISSOR_TEST);
            bloom.render(source, output);
            require(pixel(output, 12, 16)[0] > 0.005, "Bloom did not spread outside bright region");
            source.resize(1, 1); output.resize(1, 1);
            source.bind(); glClearColor(2, 2, 2, 1); glClear(GL_COLOR_BUFFER_BIT);
            bloom.render(source, output);
            closeTo(pixel(output, 0, 0)[0], 2.5f, "One-pixel bloom");
            textures(shader);
            checkGl("Batch and post-processing");
        }
    }

    private static void textures(Shader shader) {
        List<Texture> textures = new ArrayList<>();
        try (MemoryStack stack = MemoryStack.stackPush(); Framebuffer target = new Framebuffer(16, 16);
             RenderBatch batch = new RenderBatch(30, 30, 1)) {
            // RGB width 1 verifies unpack alignment, RGBA verifies the second upload path.
            textures.add(new Texture(stack.bytes((byte) 255, (byte) 0, (byte) 0), 1, 1, 3, Texture.Filter.NEAREST, false));
            textures.add(new Texture(stack.bytes((byte) 0, (byte) 255, (byte) 0, (byte) 255), 1, 1, 4, Texture.Filter.LINEAR, true));
            target.bind(); shader.bind();
            glDisable(GL_DEPTH_TEST);
            for (Texture texture : textures) batch.add(TRIANGLE, INDICES, texture, new Vector4f(1), new Transform());
            batch.flush();
            require(batch.getDrawCalls() == 2, "Texture capacity did not flush");
            closeTo(pixel(target, 8, 8)[1], 1, "Textured triangle");
        } finally { textures.forEach(Texture::dispose); }
    }

    private static void modelImport() throws Exception {
        Path file = Files.createTempFile("peach-mesh-", ".obj");
        try {
            Files.writeString(file, "v -1 -1 0\nv 1 -1 0\nv 0 1 0\nf 1 2 3\n");
            try (Model model = new Model(file.toString())) {
                require(model.getMeshes().size() == 1, "Assimp mesh import");
                require(model.getMeshes().getFirst().getIndices().length == 3, "Assimp indices");
            }
        } finally { Files.deleteIfExists(file); }
    }

    private static void hierarchyAndEmbeddedTexture() throws Exception {
        java.awt.image.BufferedImage image = new java.awt.image.BufferedImage(1, 1, java.awt.image.BufferedImage.TYPE_INT_ARGB);
        image.setRGB(0, 0, 0xffff0000);
        java.io.ByteArrayOutputStream encoded = new java.io.ByteArrayOutputStream();
        javax.imageio.ImageIO.write(image, "png", encoded);
        byte[] png = encoded.toByteArray();
        java.nio.ByteBuffer data = java.nio.ByteBuffer.allocate(104 + png.length).order(java.nio.ByteOrder.LITTLE_ENDIAN);
        for (Vertex vertex : TRIANGLE) data.putFloat(vertex.position.x).putFloat(vertex.position.y).putFloat(vertex.position.z);
        for (int i = 0; i < 3; i++) data.putFloat(0).putFloat(0).putFloat(1);
        for (int i = 0; i < 3; i++) data.putFloat(0).putFloat(0);
        data.putShort((short) 0).putShort((short) 1).putShort((short) 2).putShort((short) 0).put(png);
        String document = """
                {
                  "asset":{"version":"2.0"}, "scene":0,
                  "scenes":[{"nodes":[0]}],
                  "nodes":[{"translation":[1,2,3],"children":[1,2]},
                           {"mesh":0,"translation":[0,1,0]}, {"mesh":0,"scale":[2,2,2]}],
                  "buffers":[{"uri":"data:application/octet-stream;base64,%s","byteLength":%d}],
                  "bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":36},
                    {"buffer":0,"byteOffset":36,"byteLength":36},
                    {"buffer":0,"byteOffset":72,"byteLength":24},
                    {"buffer":0,"byteOffset":96,"byteLength":6},
                    {"buffer":0,"byteOffset":104,"byteLength":%d}],
                  "accessors":[{"bufferView":0,"componentType":5126,"count":3,"type":"VEC3","min":[-1,-1,0],"max":[1,1,0]},
                    {"bufferView":1,"componentType":5126,"count":3,"type":"VEC3"},
                    {"bufferView":2,"componentType":5126,"count":3,"type":"VEC2"},
                    {"bufferView":3,"componentType":5123,"count":3,"type":"SCALAR"}],
                  "images":[{"bufferView":4,"mimeType":"image/png"}], "textures":[{"source":0}],
                  "materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}}}],
                  "meshes":[{"primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2},"indices":3,"material":0}]}]
                }
                """.formatted(java.util.Base64.getEncoder().encodeToString(data.array()), data.capacity(), png.length);
        Path file = Files.createTempFile("peach-hierarchy-", ".gltf");
        int textureId;
        try {
            Files.writeString(file, document);
            try (Model model = new Model(file.toString())) {
                List<Mesh> meshes = model.getMeshes();
                require(meshes.size() == 2, "Nested Assimp mesh instances");
                require(meshes.get(0).getMin().equals(new Vector3f(0, 2, 3)), "Accumulated node translation");
                require(meshes.get(1).getMin().equals(new Vector3f(-1, 0, 3)), "Baked node scale/translation");
                require(meshes.get(1).getMax().equals(new Vector3f(3, 4, 3)), "Baked node bounds");
                require(meshes.get(0).getTexture() != null, "Embedded texture missing");
                require(meshes.get(0).getTexture() == meshes.get(1).getTexture(), "Model textures are not shared");
                textureId = meshes.get(0).getTexture().getId();
                require(glIsTexture(textureId), "Embedded texture upload");
            }
            require(!glIsTexture(textureId), "Model texture cleanup");
        } finally { Files.deleteIfExists(file); }
    }

    private static void slang() throws Exception {
        if (!Files.isExecutable(SetupSlang.DIRECTORY.resolve("bin/slangc"))) {
            System.out.println("Slang not installed; optional compiler check skipped (run setupSlang first).");
            return;
        }
        Path source = Files.createTempFile("peach-", ".slang"), spirv = Files.createTempFile("peach-", ".spv");
        try {
            Files.writeString(source, """
                    [shader("vertex")]
                    float4 vertexMain(float3 position : POSITION) : SV_Position { return float4(position, 1); }
                    [shader("fragment")]
                    float4 fragmentMain() : SV_Target { return float4(1, 0, 0, 1); }
                    """);
            try (Shader shader = Shader.fromSlang(source, "vertexMain", "fragmentMain")) {
                require(shader.getId() != 0, "Slang GLSL link");
            }
            SetupSlang.compile(source, spirv, "vertexMain", SetupSlang.Stage.VERTEX, SetupSlang.Target.SPIRV);
            require(Files.size(spirv) > 20, "Slang SPIR-V output");
            System.out.println("Slang GLSL 410 and offline SPIR-V passed.");
        } finally { Files.deleteIfExists(source); Files.deleteIfExists(spirv); }
    }

    private static void lifecycle() {
        WindowParams.width = WindowParams.height = 64;
        Peach peach = new Peach();
        int[] events = new int[3];
        peach.start(new PeachLevel() {
            @Override public void start() { events[0]++; }
            @Override public void update(float dt) {
                events[1]++;
                Renderer.addVertices(TRIANGLE, INDICES, new Vector4f(1));
                if (events[1] == 2) peach.stop();
            }
            @Override public void end() { events[2]++; }
        });
        require(events[0] == 1 && events[1] == 2 && events[2] == 1, "Lifecycle hook counts");
        // A throwing level must still end and release the context; a second run must work.
        try {
            peach.start(new PeachLevel() {
                @Override public void update(float dt) { throw new IllegalStateException("intentional"); }
                @Override public void end() { events[2]++; }
            });
            throw new AssertionError("Expected level exception");
        } catch (IllegalStateException e) { require(e.getMessage().equals("intentional"), "Unexpected lifecycle error"); }
        require(events[2] == 2 && peach.getWindow().getHandle() == 0, "Exception cleanup");
    }

    private static void shadowPixels() {
        Renderer.dispose(); Renderer.init();
        Renderer.getCamera().position.set(0, 0, 3);
        Renderer.getCamera().lookAt(new Vector3f());
        Mesh receiver = new Mesh(new Vertex[]{new Vertex(new Vector3f(-3, -3, 0)),
                new Vertex(new Vector3f(3, -3, 0)), new Vertex(new Vector3f(3, 3, 0)),
                new Vertex(new Vector3f(-3, 3, 0))}, new int[]{0, 1, 2, 0, 2, 3});
        receiver.material.color.set(0.5f, 0.5f, 0.5f, 1);
        Light light = new Light(LightType.DIRECTIONAL);
        light.position.set(6, 0, 3);
        light.direction.set(-3, 0, -1).normalize();
        light.intensity = 2;
        Renderer.addLight(light);
        try (Model occluder = Model.fromResource("/models/cube.obj")) {
            Transform transform = new Transform(new Vector3f(3, 0, 1), new Vector3f(0.3f), new Vector3f());
            Renderer.beginFrame(64, 64, Color.Black);
            Renderer.addMesh(receiver); Renderer.addModel(occluder, transform); Renderer.endFrame();
            float lit = screenPixel(32, 32);
            light.castsShadows = true;
            Renderer.beginFrame(64, 64, Color.Black);
            Renderer.addMesh(receiver); Renderer.addModel(occluder, transform); Renderer.endFrame();
            float shadowed = screenPixel(32, 32);
            require(Renderer.getCulledMeshes() == 1, "Regression fixture occluder must be outside the camera frustum");
            require(Renderer.getShadowDrawCalls() > 0, "No shadow depth draw");
            require(lit > 0.2f && shadowed < lit * 0.4f, "Off-screen caster failed: lit=" + lit + ", shadowed=" + shadowed);
            // A spotlight uses a perspective shadow map rather than the orthographic directional map.
            light.type = LightType.SPOT;
            light.position.set(6, 0, 2);
            light.direction.set(-3, 0, -1).normalize();
            light.innerCutoff = 25; light.outerCutoff = 40;
            light.constant = 1; light.linear = light.quadratic = 0;
            Renderer.beginFrame(64, 64, Color.Black);
            Renderer.addMesh(receiver); Renderer.addModel(occluder, transform); Renderer.endFrame();
            require(screenPixel(32, 32) < lit * 0.4f, "Spotlight shadow depth comparison failed");
            checkGl("Shadow pixel regression");
            System.out.println("Shadow pixels passed: directional and spot, including a camera-culled caster.");
        }
    }

    private static float screenPixel(int x, int y) {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            FloatBuffer pixel = stack.mallocFloat(4);
            glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, pixel);
            return pixel.get(0);
        }
    }

    private static void gameScene() {
        WindowParams.width = 960; WindowParams.height = 680;
        Peach peach = new Peach();
        net.meowsers.Game game = new net.meowsers.Game();
        // Runs the real Game -> MyLevel hierarchy and saves its presented output.
        game.addLevel(new PeachLevel() {
            private int frames;
            @Override public void update(float dt) {
                if (++frames == 4) peach.stop();
            }
            @Override public void end() {
                require(Renderer.getShadowDrawCalls() > 0, "Main scene has no shadow pass");
                int width = peach.getWindow().getFramebufferWidth(), height = peach.getWindow().getFramebufferHeight();
                java.nio.ByteBuffer pixels = org.lwjgl.system.MemoryUtil.memAlloc(width * height * 4);
                try {
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                    glReadBuffer(GL_FRONT);
                    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
                    java.awt.image.BufferedImage image = new java.awt.image.BufferedImage(width, height, java.awt.image.BufferedImage.TYPE_INT_RGB);
                    int bright = 0;
                    for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) {
                        int offset = (y * width + x) * 4;
                        int r = pixels.get(offset) & 255, g = pixels.get(offset + 1) & 255, b = pixels.get(offset + 2) & 255;
                        image.setRGB(x, height - y - 1, r << 16 | g << 8 | b);
                        if (r > 230 && g > 200) bright++;
                    }
                    require(bright > 20, "Main scene has no visible bright light");
                    javax.imageio.ImageIO.write(image, "png", Path.of("build/scene-preview.png").toFile());
                    glReadBuffer(GL_BACK);
                    checkGl("Main scene shadow/bloom");
                } catch (java.io.IOException e) { throw new AssertionError(e); }
                finally { org.lwjgl.system.MemoryUtil.memFree(pixels); }
            }
        });
        peach.start(game);
    }

    private static float[] pixel(Framebuffer framebuffer, int x, int y) {
        int previous = glGetInteger(GL_READ_FRAMEBUFFER_BINDING);
        try (MemoryStack stack = MemoryStack.stackPush()) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.getId());
            FloatBuffer pixel = stack.mallocFloat(4);
            glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, pixel);
            return new float[]{pixel.get(0), pixel.get(1), pixel.get(2), pixel.get(3)};
        } finally { glBindFramebuffer(GL_READ_FRAMEBUFFER, previous); }
    }
    private static void closeTo(float value, float expected, String message) { require(Math.abs(value - expected) < 0.03f, message + ": " + value); }
    private static void require(boolean condition, String message) { if (!condition) throw new AssertionError(message); }
    private static void checkGl(String message) { int error = glGetError(); require(error == GL_NO_ERROR, message + " GL error: " + error); }
}

package net.meowsers.peach.rendering;

import net.meowsers.peach.core.Camera;
import net.meowsers.peach.rendering.post.FullscreenQuad;
import net.meowsers.peach.rendering.post.PostProcessingPass;
import net.meowsers.peach.structures.Color;
import net.meowsers.peach.structures.Light;
import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.structures.Transform;
import org.joml.FrustumIntersection;
import org.joml.Matrix4f;
import org.joml.Vector4f;
import org.joml.Vector3f;

import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.opengl.GL41.*;

/** Single-window renderer. All methods run on the OpenGL thread, inside a Peach level's lifecycle. */
public final class Renderer {
    private static final int TEXTURE_SLOTS = 8;
    public static final int MAX_LIGHTS = 8;
    private static final Transform IDENTITY = new Transform();
    private static final Vector4f WHITE = new Vector4f(1);
    private static final List<Light> lights = new ArrayList<>();
    private static final List<PostProcessingPass> passes = new ArrayList<>();
    private static Camera camera = new Camera();
    private static RenderBatch batch;
    private static Shader shader, copy, shadowShader;
    private static ShadowMap shadowMap;
    private static int shadowDrawCalls;
    public static final Vector3f ambient = new Vector3f(0.05f);
    private record Submission(Vertex[] vertices, int[] indices, Texture texture, Vector4f color,
                              Vector3f emission, Matrix4f transform, Mesh mesh, boolean castsShadow) { }
    private static final List<Submission> submissions = new ArrayList<>();
    private static FullscreenQuad quad;
    private static Framebuffer scene, ping, pong;
    private static FrustumIntersection frustum;
    private static boolean inFrame;
    private static int width, height, culledMeshes;

    private Renderer() { }

    public static void init() {
        if (batch != null) throw new IllegalStateException("Renderer already initialized");
        try {
            shader = Shader.fromResources("/shaders/batch.vert", "/shaders/batch.frag");
            shadowShader = Shader.fromResources("/shaders/shadow.vert", "/shaders/shadow.frag");
            shadowMap = new ShadowMap(2048);
            shader.set("shadowMap", TEXTURE_SLOTS);
            for (int i = 0; i < TEXTURE_SLOTS; i++) shadowShader.set("textures[" + i + "]", i);
            copy = Shader.fromResources("/shaders/fullscreen.vert", "/shaders/copy.frag");
            for (int i = 0; i < TEXTURE_SLOTS; i++) shader.set("textures[" + i + "]", i);
            copy.set("scene", 0);
            batch = new RenderBatch(20_000, 60_000, TEXTURE_SLOTS);
            quad = new FullscreenQuad();
        } catch (RuntimeException e) { dispose(); throw e; }
    }

    public static void beginFrame(int width, int height, Color clearColor) {
        if (batch == null || inFrame) throw new IllegalStateException("Initialize the renderer and finish the previous frame first");
        if (width < 1 || height < 1) throw new IllegalArgumentException("Cannot render a zero-sized framebuffer");
        if (scene == null) {
            try {
                scene = new Framebuffer(width, height);
                ping = new Framebuffer(width, height, false);
                pong = new Framebuffer(width, height, false);
            } catch (RuntimeException e) {
                if (scene != null) scene.dispose();
                if (ping != null) ping.dispose();
                scene = ping = pong = null;
                throw e;
            }
        } else {
            scene.resize(width, height); ping.resize(width, height); pong.resize(width, height);
        }
        Renderer.width = width; Renderer.height = height;
        camera.aspect = (float) width / height;
        frustum = null;
        culledMeshes = shadowDrawCalls = 0;
        submissions.clear();
        batch.resetStats();
        scene.bind();
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(true);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(true, true, true, true);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inFrame = true;
    }

    private static void prepare(int shadowLight) {
        Matrix4f viewProjection = camera.getViewProjectionMatrix();
        frustum = new FrustumIntersection(viewProjection);
        shader.set("viewProjection", viewProjection);
        shader.set("lightCount", lights.size());
        shader.set("ambient", ambient);
        shader.set("shadowLight", shadowLight);
        shader.set("lightViewProjection", shadowMap.getMatrix());
        shader.set("shadowTexel", 1f / shadowMap.getSize());
        shader.set("shadowBias", shadowLight < 0 ? 0 : lights.get(shadowLight).shadowBias);
        shadowMap.bind(TEXTURE_SLOTS);
        for (int i = 0; i < lights.size(); i++) {
            Light light = lights.get(i);
            String prefix = "lights[" + i + "].";
            shader.set(prefix + "type", light.type.ordinal());
            shader.set(prefix + "position", light.position);
            shader.set(prefix + "direction", light.direction);
            shader.set(prefix + "color", light.color);
            shader.set(prefix + "intensity", light.intensity);
            shader.set(prefix + "innerCutoff", (float) Math.cos(Math.toRadians(light.innerCutoff)));
            shader.set(prefix + "outerCutoff", (float) Math.cos(Math.toRadians(light.outerCutoff)));
            shader.set(prefix + "constant", light.constant);
            shader.set(prefix + "linear", light.linear);
            shader.set(prefix + "quadratic", light.quadratic);
        }
    }

    private static void checkFrame() {
        if (!inFrame) throw new IllegalStateException("Submit geometry from level.update");
    }
    public static void addVertices(Vertex[] vertices, int[] indices, Texture texture) {
        submit(vertices, indices, texture, WHITE, new Vector3f(), IDENTITY.toMatrix(), null, true);
    }
    public static void addVertices(Vertex[] vertices, int[] indices, Vector4f color) {
        submit(vertices, indices, null, color, new Vector3f(), IDENTITY.toMatrix(), null, true);
    }
    public static void addMesh(Mesh mesh, Transform transform) {
        addMesh(mesh, transform.toMatrix());
    }
    private static void addMesh(Mesh mesh, Matrix4f parent) {
        submit(mesh.getVertices(), mesh.getIndices(), mesh.getTexture(), mesh.material.color, mesh.material.emission,
                new Matrix4f(parent).mul(mesh.transform.toMatrix()), mesh, mesh.material.castsShadow);
    }
    private static void submit(Vertex[] vertices, int[] indices, Texture texture, Vector4f color,
                               Vector3f emission, Matrix4f world, Mesh mesh, boolean castsShadow) {
        checkFrame();
        if (!world.isFinite() || !world.isAffine() || Math.abs(world.determinant3x3()) < 1e-10f) throw new IllegalArgumentException("Invalid mesh transform");
        // Copy per-draw state. Geometry/textures stay borrowed until endFrame has flushed both passes.
        submissions.add(new Submission(vertices, indices, texture, new Vector4f(color), new Vector3f(emission),
                new Matrix4f(world), mesh, castsShadow));
    }
    public static void addMesh(Mesh mesh) { addMesh(mesh, IDENTITY); }
    public static void addModel(Model model, Transform transform) {
        Matrix4f world = transform.toMatrix().mul(model.transform.toMatrix());
        for (Mesh mesh : model.getMeshes()) addMesh(mesh, world);
    }
    public static void addModel(Model model) { addModel(model, IDENTITY); }

    private static void draw(Submission submission) {
        batch.addTransformed(submission.vertices, submission.indices, submission.texture, submission.color,
                submission.transform, submission.emission);
    }
    private static int renderShadows() {
        int index = -1;
        for (int i = 0; i < lights.size(); i++) {
            if (lights.get(i).castsShadows) {
                if (index != -1) throw new IllegalStateException("Only one shadow-casting light is supported");
                index = i;
            }
        }
        if (index == -1) return -1;
        shadowMap.update(lights.get(index));
        shadowMap.begin();
        try {
            shadowShader.set("viewProjection", shadowMap.getMatrix());
            shadowShader.bind();
            // Do not camera-cull casters: an off-screen mesh can cast a visible shadow.
            for (Submission submission : submissions) if (submission.castsShadow) draw(submission);
            batch.flush();
            shadowDrawCalls = batch.getDrawCalls();
            batch.resetStats();
        } finally { shadowMap.end(); }
        return index;
    }

    public static void endFrame() {
        if (!inFrame) throw new IllegalStateException("No active frame");
        try {
            int shadowLight = renderShadows();
            scene.bind();
            prepare(shadowLight);
            shader.bind();
            for (Submission submission : submissions) {
                if (submission.mesh != null && !submission.mesh.isVisible(frustum, submission.transform)) {
                    culledMeshes++;
                } else draw(submission);
            }
            batch.flush();
            Framebuffer input = scene;
            for (PostProcessingPass pass : List.copyOf(passes)) {
                Framebuffer output = input == ping ? pong : ping;
                pass.render(input, output);
                input = output;
            }
            quad.render(copy, null, width, height, input.getColorTexture());
        } finally {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, width, height);
            glBindVertexArray(0);
            glUseProgram(0);
            glActiveTexture(GL_TEXTURE0);
            inFrame = false;
            submissions.clear();
        }
    }

    public static Camera getCamera() { return camera; }
    public static void setCamera(Camera camera) {
        if (camera == null) throw new IllegalArgumentException("Camera cannot be null");
        if (Renderer.camera != camera) Renderer.camera.releaseCameraMovement();
        Renderer.camera = camera;
        if (inFrame) camera.aspect = (float) width / height;
    }
    public static void addLight(Light light) {
        if (lights.size() >= MAX_LIGHTS) throw new IllegalStateException("At most " + MAX_LIGHTS + " lights are supported");
        lights.add(java.util.Objects.requireNonNull(light));
    }
    public static void removeLight(Light light) { lights.remove(light); }
    public static List<Light> getLights() { return List.copyOf(lights); }
    /** Registered passes are owned and disposed by the renderer. */
    public static void addPostProcessing(PostProcessingPass pass) {
        if (!passes.contains(pass)) passes.add(java.util.Objects.requireNonNull(pass));
    }
    public static void removePostProcessing(PostProcessingPass pass) {
        if (passes.remove(pass)) pass.dispose();
    }
    public static int getDrawCalls() { return batch == null ? 0 : batch.getDrawCalls(); }
    public static int getShadowDrawCalls() { return shadowDrawCalls; }
    public static int getCulledMeshes() { return culledMeshes; }
    public static void dispose() {
        RuntimeException failure = null;
        for (PostProcessingPass pass : passes) {
            try { pass.dispose(); } catch (RuntimeException e) {
                if (failure == null) failure = e; else failure.addSuppressed(e);
            }
        }
        passes.clear(); lights.clear();
        if (batch != null) batch.dispose();
        if (shader != null) shader.dispose();
        if (shadowShader != null) shadowShader.dispose();
        if (shadowMap != null) shadowMap.dispose();
        shadowShader = null; shadowMap = null;
        submissions.clear(); ambient.set(0.05f);
        if (copy != null) copy.dispose();
        if (quad != null) quad.dispose();
        if (scene != null) scene.dispose();
        if (ping != null) ping.dispose();
        if (pong != null) pong.dispose();
        batch = null; shader = copy = null; quad = null; scene = ping = pong = null;
        frustum = null; inFrame = false;
        camera.releaseCameraMovement();
        camera = new Camera();
        if (failure != null) throw failure;
    }
    public static void cleanup() { dispose(); }
}

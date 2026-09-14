package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Material;
import net.meowsers.peach.structures.Vertex;
import net.meowsers.peach.utils.PeachException;
import org.joml.Matrix4f;
import org.joml.Matrix3f;
import net.meowsers.peach.structures.Transform;
import org.joml.Vector2f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.lwjgl.PointerBuffer;
import org.lwjgl.assimp.AIColor4D;
import org.lwjgl.assimp.AIMaterial;
import org.lwjgl.assimp.AIMatrix4x4;
import org.lwjgl.assimp.AIMesh;
import org.lwjgl.assimp.AINode;
import org.lwjgl.assimp.AIScene;
import org.lwjgl.assimp.AIString;
import org.lwjgl.assimp.AITexel;
import org.lwjgl.assimp.AITexture;
import org.lwjgl.assimp.AIVector3D;
import org.lwjgl.system.MemoryStack;
import org.lwjgl.system.MemoryUtil;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Path;
import java.nio.file.Files;
import java.io.IOException;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.lwjgl.assimp.Assimp.*;

/** Imports static meshes. Node transforms are baked into geometry; textures are owned by this model. */
public class Model implements AutoCloseable {
    private final List<Mesh> meshes = new ArrayList<>();
    private final Map<String, Texture> textures = new HashMap<>();
    private boolean disposed;
    public final Transform transform = new Transform();

    public Model(String path) {
        Path file = Path.of(path).toAbsolutePath().normalize();
        AIScene scene = aiImportFile(file.toString(), aiProcess_Triangulate | aiProcess_GenSmoothNormals
                | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_FlipUVs);
        if (scene == null) throw new PeachException("Cannot load model " + path + ": " + aiGetErrorString());
        try {
            if (scene.mRootNode() == null || (scene.mFlags() & AI_SCENE_FLAGS_INCOMPLETE) != 0) {
                throw new PeachException("Incomplete model: " + path);
            }
            visit(scene, scene.mRootNode(), new Matrix4f(), file.getParent());
        } catch (RuntimeException e) {
            dispose();
            throw e;
        } finally { aiReleaseImport(scene); }
    }

    /** For self-contained packaged assets, such as OBJ geometry or embedded GLB files. */
    public static Model fromResource(String resource) {
        Path file = null;
        try (var input = Model.class.getResourceAsStream(resource)) {
            if (input == null) throw new PeachException("Model resource missing: " + resource);
            String suffix = resource.substring(resource.lastIndexOf('.'));
            file = Files.createTempFile("peach-model-", suffix);
            Files.copy(input, file, StandardCopyOption.REPLACE_EXISTING);
            return new Model(file.toString());
        } catch (IOException e) { throw new PeachException("Cannot read model resource: " + resource, e); }
        finally {
            if (file != null) try { Files.deleteIfExists(file); }
            catch (IOException e) { file.toFile().deleteOnExit(); }
        }
    }

    private void visit(AIScene scene, AINode node, Matrix4f parent, Path directory) {
        Matrix4f world = new Matrix4f(parent).mul(toMatrix(node.mTransformation()));
        IntBuffer references = node.mMeshes();
        PointerBuffer sceneMeshes = scene.mMeshes();
        for (int i = 0; i < node.mNumMeshes(); i++) {
            Mesh mesh = readMesh(scene, AIMesh.create(sceneMeshes.get(references.get(i))), directory);
            // Static imports retain the exact affine hierarchy (including shear),
            // without decomposing it into a lossy position/rotation/scale pose.
            if (!world.isFinite() || !world.isAffine() || Math.abs(world.determinant3x3()) < 1e-10f) {
                throw new PeachException("Invalid imported node transform: " + node.mName().dataString());
            }
            Matrix3f normalMatrix = world.normal(new Matrix3f());
            for (Vertex vertex : mesh.getVertices()) {
                world.transformPosition(vertex.position);
                normalMatrix.transform(vertex.normal);
                if (vertex.normal.lengthSquared() > 0) vertex.normal.normalize();
            }
            mesh.updateBounds();
            meshes.add(mesh);
        }
        PointerBuffer children = node.mChildren();
        for (int i = 0; i < node.mNumChildren(); i++) visit(scene, AINode.create(children.get(i)), world, directory);
    }

    public static Matrix4f toMatrix(AIMatrix4x4 m) {
        return new Matrix4f(m.a1(), m.b1(), m.c1(), m.d1(), m.a2(), m.b2(), m.c2(), m.d2(),
                m.a3(), m.b3(), m.c3(), m.d3(), m.a4(), m.b4(), m.c4(), m.d4());
    }

    private Mesh readMesh(AIScene scene, AIMesh mesh, Path directory) {
        Vertex[] vertices = new Vertex[mesh.mNumVertices()];
        AIVector3D.Buffer positions = mesh.mVertices(), normals = mesh.mNormals(), uv = mesh.mTextureCoords(0);
        AIColor4D.Buffer colors = mesh.mColors(0);
        for (int i = 0; i < vertices.length; i++) {
            AIVector3D p = positions.get(i);
            AIVector3D n = normals == null ? null : normals.get(i);
            AIVector3D t = uv == null ? null : uv.get(i);
            AIColor4D c = colors == null ? null : colors.get(i);
            vertices[i] = new Vertex(new Vector3f(p.x(), p.y(), p.z()),
                    c == null ? new Vector4f(1) : new Vector4f(c.r(), c.g(), c.b(), c.a()),
                    t == null ? new Vector2f() : new Vector2f(t.x(), t.y()),
                    n == null ? new Vector3f(0, 0, 1) : new Vector3f(n.x(), n.y(), n.z()));
        }
        List<Integer> elements = new ArrayList<>();
        for (int i = 0; i < mesh.mNumFaces(); i++) {
            IntBuffer face = mesh.mFaces().get(i).mIndices();
            if (face.remaining() == 3) for (int j = 0; j < 3; j++) elements.add(face.get(j));
        }
        Material material = new Material();
        List<Texture> maps = new ArrayList<>();
        if (mesh.mMaterialIndex() < scene.mNumMaterials()) {
            AIMaterial source = AIMaterial.create(scene.mMaterials().get(mesh.mMaterialIndex()));
            try (MemoryStack stack = MemoryStack.stackPush()) {
                AIColor4D color = AIColor4D.malloc(stack);
                if (aiGetMaterialColor(source, AI_MATKEY_COLOR_DIFFUSE, aiTextureType_NONE, 0, color) == aiReturn_SUCCESS) {
                    material.color.set(color.r(), color.g(), color.b(), color.a());
                }
                AIString name = AIString.calloc(stack);
                if (aiGetMaterialString(source, AI_MATKEY_NAME, 0, 0, name) == aiReturn_SUCCESS) material.name = name.dataString();
                var shininess = stack.mallocFloat(1);
                if (aiGetMaterialFloatArray(source, AI_MATKEY_SHININESS, 0, 0, shininess, stack.ints(1)) == aiReturn_SUCCESS) {
                    material.shininess = shininess.get(0);
                }
                int type = aiGetMaterialTextureCount(source, aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : aiTextureType_DIFFUSE;
                for (int i = 0; i < aiGetMaterialTextureCount(source, type); i++) {
                    AIString texturePath = AIString.calloc(stack);
                    if (aiGetMaterialTexture(source, type, i, texturePath, (IntBuffer) null, null, null, null, null, null) == aiReturn_SUCCESS) {
                        String key = texturePath.dataString().replace('\\', '/');
                        maps.add(textures.computeIfAbsent(key, value -> loadTexture(scene, directory, value)));
                    }
                }
            }
        }
        return new Mesh(vertices, elements.stream().mapToInt(Integer::intValue).toArray(), material, maps);
    }

    private Texture loadTexture(AIScene scene, Path directory, String name) {
        AITexture embedded = aiGetEmbeddedTexture(scene, name);
        if (embedded == null) return new Texture(directory.resolve(name).normalize().toString());
        if (embedded.mHeight() == 0) return Texture.fromEncoded(embedded.pcDataCompressed());
        int size = Math.multiplyExact(Math.multiplyExact(embedded.mWidth(), embedded.mHeight()), 4);
        ByteBuffer pixels = MemoryUtil.memAlloc(size);
        try {
            AITexel.Buffer texels = embedded.pcData();
            for (int i = 0; i < size / 4; i++) {
                AITexel t = texels.get(i);
                pixels.put(t.r()).put(t.g()).put(t.b()).put(t.a());
            }
            pixels.flip();
            return new Texture(pixels, embedded.mWidth(), embedded.mHeight(), 4, Texture.Filter.LINEAR, true);
        } finally { MemoryUtil.memFree(pixels); }
    }

    public List<Mesh> getMeshes() {
        if (disposed) throw new IllegalStateException("Model is disposed");
        return List.copyOf(meshes);
    }

    public void dispose() {
        textures.values().forEach(Texture::dispose);
        textures.clear();
        disposed = true;
    }
    public void cleanup() { dispose(); }
    @Override public void close() { dispose(); }
}

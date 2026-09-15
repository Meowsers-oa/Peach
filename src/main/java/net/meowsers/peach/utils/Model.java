package net.meowsers.peach.utils;

import net.meowsers.peach.rendering.Texture;
import net.meowsers.peach.structures.Color;
import net.meowsers.peach.structures.Mesh;
import net.meowsers.peach.structures.Vertex;
import org.joml.Matrix4f;
import org.joml.Vector2f;
import org.joml.Vector3f;
import org.lwjgl.assimp.*;
import org.lwjgl.system.MemoryStack;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.lwjgl.assimp.Assimp.*;
import static org.lwjgl.system.MemoryUtil.*;

/** Imports model files into ordinary meshes. Load on the OpenGL thread when textures are present. */
public class Model {
    private final Mesh result = new Mesh();
    private final Map<String, Texture> textures = new HashMap<>();
    private String directory;

    private Model() {
    }

    /** Returns one mesh with all model parts, node transforms and face textures preserved. */
    public static Mesh loadModel(String path) {
        return new Model().load(path);
    }

    private Mesh load(String path) {
        path = Path.of(path.replace('\\', '/')).normalize().toString();
        Path parent = Path.of(path).getParent();
        directory = parent == null ? "" : parent.toString();
        try (ModelFiles files = new ModelFiles()) {
            AIScene scene = aiImportFileEx(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
                    | aiProcess_SortByPType | aiProcess_TransformUVCoords, files.io);
            if (scene == null) throw new PeachException("Could not load model " + path + ": " + aiGetErrorString());
            try {
                if (scene.mRootNode() == null || (scene.mFlags() & AI_SCENE_FLAGS_INCOMPLETE) != 0) {
                    throw new PeachException("Incomplete model: " + path);
                }
                processNode(scene.mRootNode(), scene);
                if (result.getIndices().isEmpty()) throw new PeachException("Model contains no triangle meshes: " + path);
            } finally {
                aiReleaseImport(scene);
            }
        } catch (RuntimeException e) {
            for (Texture texture : textures.values()) texture.end();
            throw e;
        }
        return result;
    }

    private void processNode(AINode node, AIScene scene) {
        processNode(node, scene, new Matrix4f());
    }

    private void processNode(AINode node, AIScene scene, Matrix4f parent) {
        AIMatrix4x4 m = node.mTransformation();
        Matrix4f model = parent.mul(new Matrix4f(
                m.a1(), m.b1(), m.c1(), m.d1(), m.a2(), m.b2(), m.c2(), m.d2(),
                m.a3(), m.b3(), m.c3(), m.d3(), m.a4(), m.b4(), m.c4(), m.d4()), new Matrix4f());
        for (int i = 0; i < node.mNumMeshes(); i++) {
            AIMesh source = AIMesh.create(scene.mMeshes().get(node.mMeshes().get(i)));
            if ((source.mPrimitiveTypes() & aiPrimitiveType_TRIANGLE) == 0) continue;
            Mesh mesh = processMesh(source, scene);
            // Bake the full node hierarchy, including shear and repeated mesh instances.
            for (Vertex vertex : mesh.getVertices()) model.transformPosition(vertex.position);
            if (model.determinant3x3() < 0) {
                List<Integer> indices = mesh.getIndices();
                for (int j = 0; j < indices.size(); j += 3) {
                    int second = indices.get(j + 1);
                    indices.set(j + 1, indices.get(j + 2));
                    indices.set(j + 2, second);
                }
            }
            // Append each imported part once; its vertices already include the node transform.
            int offset = result.getVertices().size();
            result.addVertices(mesh.getVertices());
            for (int index : mesh.getIndices()) result.getIndices().add(offset + index);
            Texture texture = mesh.getTextures().isEmpty() ? null : mesh.getTextures().get(0);
            for (int j = 0; j < mesh.getIndices().size(); j += 3) result.getTextures().add(texture);
        }
        for (int i = 0; i < node.mNumChildren(); i++) {
            processNode(AINode.create(node.mChildren().get(i)), scene, model);
        }
    }

    private Mesh processMesh(AIMesh source, AIScene scene) {
        Mesh mesh = new Mesh();
        AIMaterial material = AIMaterial.create(scene.mMaterials().get(source.mMaterialIndex()));
        int type = textureType(material);
        try (MemoryStack stack = MemoryStack.stackPush()) {
            AIColor4D tint = AIColor4D.calloc(stack).set(1, 1, 1, 1);
            if (aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, 0, 0, tint) != aiReturn_SUCCESS) {
                aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, 0, 0, tint);
            }
            var opacity = stack.floats(1);
            if (aiGetMaterialFloatArray(material, AI_MATKEY_OPACITY, 0, 0, opacity, stack.ints(1)) == aiReturn_SUCCESS) {
                tint.a(opacity.get(0));
            }
            IntBuffer uvIndex = stack.ints(0);
            if (aiGetMaterialTextureCount(material, type) > 0) {
                aiGetMaterialTexture(material, type, 0, AIString.calloc(stack), null, uvIndex, null, null, null, null);
            }
            int channel = uvIndex.get(0);
            if (channel < 0 || channel >= AI_MAX_NUMBER_OF_TEXTURECOORDS) throw new PeachException("Invalid model UV channel: " + channel);
            AIVector3D.Buffer positions = source.mVertices();
            AIVector3D.Buffer uvs = source.mTextureCoords(channel);
            AIColor4D.Buffer colors = source.mColors(0);
            for (int i = 0; i < source.mNumVertices(); i++) {
                AIVector3D position = positions.get(i);
                Color color = new Color(tint.r(), tint.g(), tint.b(), tint.a());
                if (colors != null) {
                    AIColor4D vertexColor = colors.get(i);
                    color.r *= vertexColor.r();
                    color.g *= vertexColor.g();
                    color.b *= vertexColor.b();
                    color.a *= vertexColor.a();
                }
                Vector2f uv = uvs == null ? new Vector2f() : new Vector2f(uvs.get(i).x(), uvs.get(i).y());
                mesh.getVertices().add(new Vertex(new Vector3f(position.x(), position.y(), position.z()), color, uv));
            }
        }
        AIFace.Buffer faces = source.mFaces();
        for (int i = 0; i < source.mNumFaces(); i++) {
            AIFace face = faces.get(i);
            if (face.mNumIndices() != 3) throw new PeachException("Assimp could not triangulate a model face");
            for (int j = 0; j < 3; j++) mesh.getIndices().add(face.mIndices().get(j));
        }
        mesh.addTextures(loadMaterialTextures(material, scene));
        return mesh;
    }

    private int textureType(AIMaterial material) {
        return aiGetMaterialTextureCount(material, aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : aiTextureType_DIFFUSE;
    }

    private List<Texture> loadMaterialTextures(AIMaterial material, AIScene scene) {
        int type = textureType(material);
        int count = aiGetMaterialTextureCount(material, type);
        if (count == 0) return List.of();
        if (count > 1) Log.warning("Model material has layered color textures; Peach uses the first color layer");
        try (MemoryStack stack = MemoryStack.stackPush()) {
            AIString name = AIString.calloc(stack);
            if (aiGetMaterialTexture(material, type, 0, name, (IntBuffer) null, null, null, null, null, null) != aiReturn_SUCCESS) {
                throw new PeachException("Could not read model material texture");
            }
            String path = name.dataString().replace('\\', '/');
            AITexture embedded = aiGetEmbeddedTexture(scene, path);
            String key = embedded == null ? Path.of(directory).resolve(path).normalize().toString() : "embedded:" + embedded.address();
            Texture texture = textures.get(key);
            if (texture == null) {
                texture = embedded == null ? new Texture(key) : loadEmbeddedTexture(embedded);
                textures.put(key, texture);
            }
            return List.of(texture);
        }
    }

    private Texture loadEmbeddedTexture(AITexture source) {
        if (source.mHeight() == 0) return new Texture(source.pcDataCompressed());
        int width = source.mWidth(), height = source.mHeight();
        ByteBuffer pixels = memAlloc(Math.multiplyExact(Math.multiplyExact(width, height), 4));
        try {
            AITexel.Buffer texels = source.pcData();
            for (int y = height - 1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    AITexel texel = texels.get(y * width + x);
                    pixels.put(texel.r()).put(texel.g()).put(texel.b()).put(texel.a());
                }
            }
            pixels.flip();
            return new Texture(width, height, pixels);
        } finally {
            memFree(pixels);
        }
    }

}

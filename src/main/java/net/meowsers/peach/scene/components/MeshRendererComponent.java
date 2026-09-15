package net.meowsers.peach.scene.components;

import net.meowsers.peach.rendering.Renderer;
import net.meowsers.peach.scene.Component;
import net.meowsers.peach.structures.Mesh;
import org.joml.Matrix4f;

public class MeshRendererComponent extends Component {

    private Mesh mesh;
    private final Matrix4f model = new Matrix4f();

    public MeshRendererComponent(Mesh mesh) {
        this.mesh = mesh;
    }
    public MeshRendererComponent() {
        this.mesh = new Mesh();
    }

    @Override
    public void update(float dt) {
        super.update(dt);
        Renderer.addMesh(mesh, getParent().transform.toMatrix(model));
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mesh.end();
    }

    public Mesh getMesh() {
        return mesh;
    }
    public void setMesh(Mesh mesh) {
        this.mesh = mesh;
    }
}

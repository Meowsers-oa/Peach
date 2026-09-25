# Peach 

#### a small 3d renderer/engine in C you can just play around in.
I made this because I just like graphics programming/rendering and I want to get a little more comfortable
in C.

### Building

---
```sh
cmake -S . -B cmake-build-debug -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DGLAD_REPRODUCIBLE=ON && cmake --build cmake-build-debug --parallel
```

### Libraries used:

---

- Rendering - [OpenGL 4.1](https://www.opengl.org) 
- Window API - [GLFW](https://www.glfw.org)
- Image loader - [Stb image](https://github.com/nothings/stb/tree/master)
- Math library - [CGLM](https://github.com/recp/cglm)
- OpenGL Loader - [GLAD](https://glad.dav1d.de)
- 3D model loader - [Assimp](https://assimp.org)
- UI backend - [C-ImGui](https://github.com/cimgui/cimgui)

### What can it do?

---
idk, figure it out yourself. I'm not writing this, okay?

###### - Made with <3 by Meowsers

### OBJ / MTL materials

Load the OBJ; Assimp follows its `mtllib` reference and Peach loads each
material's diffuse texture automatically. Texture paths resolve relative to the
MTL, with an OBJ-relative fallback. Load after `mInit`, and zero-initialize meshes.

```c
mMesh model = {0};
if (!mMeshLoadEx("assets/models/Yoshi/Yoshi.obj", &model, mMeshImportFlipFacing)) {
    // Handle load failure. A previously loaded model remains intact.
}
mTransform transform = {.position = {0, 2.73f, 0}, .scale = {.015f, .015f, .015f}};

// Inside the frame, after setting the camera:
mDrawMesh(&ctx, &model, &transform);

// After the final frame, before mEnd:
mMeshFree(&model);
```


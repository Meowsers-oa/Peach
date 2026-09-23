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
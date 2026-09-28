# Peach

#### A lightweight 2D Game engine in C

I'm not entirely sure where I'm going with this yet. It's mostly just me playing and messing around. And mainly just having fun!

You're free to do whatever with this, really.

----

### Libraries used:



- Rendering: [OpenGL 4.1](https://www.opengl.org)
- Window API: [GLFW](https://www.glfw.org/)
- Image/Texture loading: [STB_Image](https://github.com/nothings/stb)
- Math Library: [C-GLM](https://github.com/nothings/stb)
- Loading OpenGL: [GLAD](https://glad.dav1d.de/)

----

### Building:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPEACH_BUILD_TESTS=ON && cmake --build build --parallel
```

that should build it (I hope)

###### Made with <3 by Meowsers

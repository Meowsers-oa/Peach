# Peach

A C11 library using GLFW and OpenGL 4.1.

The 2D renderer is created with the window and destroyed by `mDestroy`. Submit
geometry before `mUpdate`, which flushes the batch, presents the frame, polls input,
and clears the next frame using `ctx.window.bgColor`.

```c
#include <Peach/Peach.h>

mVertex vertices[] = {
    {.x = 100, .y = 100, .u = 0, .v = 0, .color = M_COLOR_WHITE},
    {.x = 200, .y = 100, .u = 1, .v = 0, .color = M_COLOR_WHITE},
    {.x = 200, .y = 200, .u = 1, .v = 1, .color = M_COLOR_WHITE},
    {.x = 100, .y = 200, .u = 0, .v = 1, .color = M_COLOR_WHITE}
};
unsigned int indices[] = {0, 1, 2, 2, 3, 0};

// Inside the loop, after creating ctx and its window:
mAddVerticesIndexed(&ctx, vertices, 4, indices, 6, NULL, NULL);
mUpdate(&ctx);
```

`mAddVertices` accepts non-indexed triangle lists; `mAddVerticesIndexed` accepts
indices local to the supplied vertex array. Both return `M_SUCCESS` or `M_FAILURE`.
Invalid index lists are rejected before drawing anything from that submission.
Vertex, index, and texture capacity limits automatically flush the current batch.
Oversized submissions are split on triangle boundaries.

Coordinates default to window units with the origin at the top left and positive Y
down. The viewport follows framebuffer size, including HiDPI displays. Each
submission accepts an optional column-major 4x4 model matrix (`NULL` is identity).
Pass a cglm `mat4` as `&matrix[0][0]`; translation, rotation, scale, shear, and
homogeneous W are preserved. `mRendererSetViewProjection` sets a persistent custom
camera, flushing queued geometry first. Pass `NULL` to restore the default camera.

`Camera.h` provides `mCameraCreate`, `mCameraUpdate`, and `mCameraResize`. A camera's
`x` and `y` specify the world point at the viewport center, `rotation` is in radians,
and `zoom` must be positive. `mCameraResize` preserves position. After changing its
fields, call `mCameraUpdate` and apply it with `mRendererSetCamera(&ctx, &camera)`.
The renderer copies its current matrix, so reapply after later changes. Pass `NULL`
to return to the automatic window-sized camera.

```c
mCamera camera;
mCameraCreate(&camera, 1200, 800);
camera.zoom = 2.0f;
mCameraUpdate(&camera);
mRendererSetCamera(&ctx, &camera);

mTexture texture = {0};
if (mTextureLoad(&texture, "sprite.png") == M_SUCCESS) {
    mAddVerticesIndexed(&ctx, vertices, 4, indices, 6, NULL, &texture);
    mUpdate(&ctx);
    mTextureDestroy(&ctx, &texture);
}
```

Initialize an `mTexture` to `{0}` and call `mTextureCreate` with packed RGBA8 pixels.
Uploads use linear filtering and clamp-to-edge wrapping. The first uploaded row
corresponds to `v = 0`. Pass the texture to either submission function, or `NULL`
for solid vertex colors. Sampled texture colors are multiplied by vertex colors;
use `M_COLOR_WHITE` for an untinted image. `mTextureLoad` loads PNG, JPEG, BMP, TGA and other stb_image formats directly from
files; `mTextureLoadMemory` accepts encoded image bytes. Both produce RGBA8 textures
without flipping rows. The vendored stb_image implementation lives in `external/stb`.
`mTextureDestroy` flushes queued draws before releasing the texture. Destroy user
textures before `mDestroy`; the renderer owns only its internal white texture.

Drawing preserves submission order with straight-alpha blending and no depth test.
Vertices and matrices can be reused immediately after submission; textures must
remain alive until the batch is flushed. Call `mRendererFlush` before modifying
texture contents or using external OpenGL rendering. The renderer sets its own
shader, vertex array, texture units, and rasterization state without restoring
previous state. Keep the owning OpenGL context current and do not copy or move
`mContext` after creating its window.

The sandbox shows a rotating textured quad and a colored triangle. Press Escape
to exit. Optional GPU tests require a graphical session with OpenGL 4.1:

```sh
cmake -S . -B build -DPEACH_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

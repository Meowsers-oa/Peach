# Lights and shadows

Register lights once, then draw between `mRendererBegin` and `mRendererEnd`.
Call `mLoadDefaultShaders(&ctx)` after window creation to load the lighting
shaders from `assets/shaders/scene.vert` and `scene.frag`. Shadow shaders live
alongside them in `shadow.vert` and `shadow.frag` and load automatically.

```c
mLight point = mCreatePointLight((vec3){-3, 5, 3}, WHITE, 45.0f);
mLight spot = mCreateSpotLight((vec3){4, 6, 1},
                              (vec3){-0.4f, -1, -0.2f}, GOLD, 70.0f);
addLight(&ctx, &point);
addLight(&ctx, &spot);

// Each frame:
mRendererSetCamera3D(&ctx, &camera);
mRendererBegin(&ctx);
mAddCubeRotated(&ctx, (vec3){0, 0, 0}, (vec3){1, 1, 1},
                (vec3){0, angleRadians, 0}, RED);
mAddQuad3D(&ctx, (vec3){0, -1, 0}, (vec2){12, 12},
           (vec3){0, 1, 0}, LIGHT_GRAY);
mRendererEnd(&ctx);

// When no longer needed:
removeLight(&ctx, &point);
removeLight(&ctx, &spot);
```

The renderer stores **borrowed pointers**, not copies. Keep each light alive until
removed or renderer shutdown. Change its fields before `mRendererEnd` to update
it. Registering the same pointer twice is harmless. `mRendererAddLight` returns
zero for an invalid light or a full registry; `addLight` logs that failure.

| Field | Meaning |
| --- | --- |
| `type` | `mPointLight` radiates in all directions; `mSpotLight` uses a cone. |
| `position`, `direction` | World space. A zero direction defaults to down. |
| `color`, `intensity` | RGB color and brightness. Zero intensity disables its contribution. |
| `range` | Maximum distance in world units, with smooth attenuation. Default: 25. |
| `innerCone`, `outerCone` | Spotlight half angles in **degrees**. Constructor defaults: 20 and 30. |
| `castsShadows` | Enabled by the constructors. Set to zero to disable shadows for a light. |
| `shadowBias` | Minimum world-space receiver offset. Default: 0.02; also adjusted for texel size and surface angle. |

Use `mRendererSetAmbientLight(&ctx, 0.12f)` to adjust ambient lighting.
With no registered lights, the shaders preserve the original unlit colors.
Lights use diffuse shading with distance attenuation. Transparent colors still
blend; shadow casting uses an alpha cutoff of 0.5.

## Rendering and limits

- Up to **8 lights**, with **512 × 512** depth maps per view. Point lights use
  six views; spotlights use one. Maps share a depth texture array and use a
  3 × 3 comparison filter.
- Fifteen geometry texture slots per batch, including white. The remaining
  OpenGL 4.1 fragment texture unit is reserved for shadows.
- Shadow resources are renderer-owned and reused. The full depth array is
  allocated on the first shadowed scene; shutdown releases it.
- Inside Begin/End, `mRendererFlush` seals a batch. `mRendererEnd` builds maps
  from **all** its batches before drawing, so a caster submitted after a receiver
  still casts a shadow onto it. Each Begin/End pair is a separate scene.
- Keep textures and shader programs alive until End. Use renderer matrix
  setters, which seal pending geometry before changing matrices. Finish a scene
  before directly changing render targets, clearing buffers, or performing
  unrelated OpenGL rendering.
- Normals follow rotations and the inverse transpose of `uModel`. Quad normals
  define the visible front face.
- All submitted triangles cast two-sided shadows. Partially transparent
  surfaces use cutout shadows, not translucent shadows. Point-view edges use
  clamped filtering rather than seamless cross-face filtering.
- Shadow near clipping is 0.05 world units; the far plane follows light range.
  Custom shaders must implement the lighting uniforms themselves.
- Renderer draw-call statistics include shadow passes.

## Verification

The integration test creates a hidden OpenGL 4.1 window and needs a working
graphics/display session.

```sh
cmake -S . -B cmake-build-debug -DPEACH_BUILD_TESTS=ON
cmake --build cmake-build-debug
ctest --test-dir cmake-build-debug --output-on-failure
```

Tests cover rendered shadows across batches, all six point views, model
transforms, live spotlight changes, registry limits/removal, unlit rendering,
batch rollover, graphics-state restoration, and resource cleanup.
Set `PEACH_TEST_IMAGES=1` when running `cmake-build-debug/tests/peach_lighting_tests` to write comparison
images to `/tmp/peach-*-lit.ppm` and `/tmp/peach-*-shadow.ppm`.

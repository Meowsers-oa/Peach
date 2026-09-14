# Peach Engine

Java 21 / LWJGL 3 / OpenGL 4.1 Core on macOS. Engine shader sources are written in
**Slang**; Gradle generates the GLSL that the macOS driver consumes.

## Run the game

```sh
./gradlew runPeach
```

This runs the existing `Main → Game → MyLevel` path. `MyLevel` loads a plane and
cube through Assimp, adds a warm spotlight with a visible emissive marker, and
renders shadows, HDR bloom and tone mapping. The cube rotates slowly.

- Hold right mouse: look around; WASD flies, Q/E moves down/up.
- Hold Shift while flying: move faster. Scroll while flying: adjust movement speed.
- Middle mouse drag: pan. Scroll without right mouse: move forward/backward.
- Space: pause/resume cube rotation.
- Escape: quit.

`runDemo` is an alias for the same main game. There is no separate demo level.

## Game and level hierarchy

The user's game only registers levels:

```java
package net.meowsers;

import net.meowsers.peach.core.PeachProgram;

public class Game extends PeachProgram {
    public Game() {
        addLevel(new MyLevel());
    }
}
```

The user implements scene behavior in a level:

```java
public class MyLevel extends PeachLevel {
    private Model model;

    @Override public void start() {
        model = Model.fromResource("/models/cube.obj");
    }

    @Override public void update(float dt) {
        Renderer.addModel(model);
    }

    @Override public void end() {
        model.dispose();
    }
}
```

Imports in the level are `net.meowsers.peach.core.PeachLevel`,
`net.meowsers.peach.rendering.Model` and `net.meowsers.peach.rendering.Renderer`.
See `src/main/java/net/meowsers/MyLevel.java` for the complete working scene.

`Peach` owns the window, frame timing, input, renderer and ImGui. It calls the
game's lifecycle; `PeachProgram` starts its registered levels, calls each active
level's `update(dt)` **once per frame**, and ends them in reverse order. Levels do
not create threads or independent while-loops. Future entities can live under
levels without changing this hierarchy.

`addLevel` and `removeLevel` apply at the next update boundary. `getLevels()` returns
an immutable snapshot of active levels. A level can use protected `getPeach()` and
`getWindow()` while attached. Cleanup continues if a level throws. Minimized
windows pause level updates. All engine and OpenGL calls run on the main thread.

## Slang build pipeline

Sources live in `src/main/slang/`:

| Source | Purpose |
| --- | --- |
| `geometry.slang` | Shared batch vertices, colors, textures, normals and emission |
| `batch.slang` | Lighting and shadow sampling |
| `shadow.slang` | Depth-only alpha-tested shadow pass |
| `fullscreen.slang` | Fullscreen quad vertex stage |
| `bloom-filter.slang` | Bright extraction and downsample/upsample blur |
| `bloom-compose.slang` | Add bloom to the HDR scene |
| `tonemap.slang` | Exposure and display gamma |
| `copy.slang`, `invert.slang` | Copy and inversion examples |

`runPeach` and `build` automatically run `setupSlang` and `compileShaders` when
needed. The pinned Slang 2025.24.3 distribution is installed into ignored
`bin/slang/`, matching the JVM's macOS arm64 or x86_64 architecture. Generated GLSL
410 resources live in `build/generated/shaderResources/shaders/` and are packaged
in the JAR. Never edit generated shader files. Packaged games need no compiler at
runtime. For another platform or an existing compiler, pass
`-Dpeach.slangc=/absolute/path/to/slangc` to Gradle.

The compatibility step removes Vulkan-only binding defaults, orders GLSL qualifiers,
and matches varying names by location for Apple's monolithic linker. Unsupported
extensions/storage features are rejected. Slang matrices use column-major host
layout; `Shader` reflects uniform block offsets and uploads JOML matrices correctly.
`shader.set("name", value)` accepts source-level uniform names, including
`lights[0].position`, vectors, scalars, matrices and sampler units. Set uniforms
**before `shader.bind()`**, which uploads pending block data. The engine handles
this order for all built-in passes.

For custom runtime compilation, `Shader.fromSlang(path, "vertexMain", "fragmentMain")`
is available after installing Slang. `SetupSlang.compile` can also emit offline
SPIR-V; OpenGL 4.1 uses the generated GLSL path. Dear ImGui's third-party backend
continues to manage its own OpenGL shader internally.

## Rendering and shadows

`Renderer.addMesh(mesh, transform)` and `addModel(model, transform)` submit geometry;
omitting the transform uses identity. `addVertices(vertices, indices, texture)`
and `addVertices(vertices, indices, color)` remain available. `Vertex` holds
position, color, UV and normal; `Mesh` holds vertices, indices, textures and material.

Submission records copy transforms and material values. Geometry and textures are
borrowed until the frame finishes: do not mutate geometry or dispose its textures
between submission and the end of the frame. The renderer first draws the shadow
map, then the camera-visible geometry, then the post-processing stack. Batches
split at triangle boundaries on vertex/index capacity or texture limits. Up to
eight material textures share a batch; texture unit 8 is reserved for shadows.

```java
Light light = new Light(LightType.SPOT);
light.position.set(-3.5f, 6, 2.5f);
light.direction.set(light.position).negate().normalize();
light.intensity = 7;
light.innerCutoff = 38;
light.outerCutoff = 55;
light.castsShadows = true;
Renderer.addLight(light);
```

The renderer supports **one shadow-casting directional or spot light** among up to
eight lights. Its 2048² depth map uses slope offset, receiver bias and 3×3 PCF.
Off-screen meshes remain shadow casters. Point-light cubemap shadows, cascades,
area-light shadows and transparent shadow transmission are not implemented.
`AREA` still uses point-light shading. A second shadow-casting light reports an
explicit error instead of silently picking one.

`shadowNear`, `shadowFar`, `shadowExtent` (directional half-width), and `shadowBias`
are configurable on the light. Directional shadow coverage is anchored to the
light's position and direction; spot coverage uses twice its outer cone half-angle.
`mesh.material.castsShadow = false` excludes geometry such as the glowing lamp.
`material.emission` adds linear HDR radiance; bloom needs bright pixels, so a light
has a separate visible emissive mesh. `Renderer.ambient` controls ambient illumination.

Camera culling uses transformed mesh AABBs, including nonuniform scale and shear.
After modifying vertex positions, call `mesh.updateBounds()`. Normals use the
inverse-transpose transform. Transform matrices must be affine and invertible.
`getDrawCalls()` counts camera draws, `getShadowDrawCalls()` counts depth draws,
and `getCulledMeshes()` counts camera-culled meshes.

## Camera, post-processing and ownership

`Renderer.getCamera()` exposes position, pitch/yaw/roll, FOV and near/far planes.
Angles are degrees; `camera.lookAt(target)` provides an easier orientation helper.
Call `Renderer.getCamera().handleCameraMovement()` once in your level's `update()`
for the fly/pan/scroll controls above. An overload accepts `deltaTime` explicitly.
Tune `movementSpeed`, `mouseSensitivity`, `fastMultiplier`, `panSensitivity`, and
`scrollSensitivity` on the camera. Movement is time-scaled, diagonals are normalized,
and pitch is clamped to avoid flipping. Q/E uses world up/down. Release the mouse
to restore cursor capture; losing focus clears held inputs. Call
`releaseCameraMovement()` if you stop updating that camera (camera replacement and
renderer shutdown also release it). `MyLevel` already calls the movement helper.

Aspect follows the actual framebuffer size, including Retina. Camera and lighting
are evaluated after all levels have submitted their scene.

Add effects with `Renderer.addPostProcessing(new BloomPass())`, followed by
`new ToneMapPass()` for display output. `InvertPass` remains an example effect.
Custom `PostProcessingPass` implementations receive distinct non-null input/output
framebuffers and must write the whole output. Bloom keeps intermediate data in
RGBA16F and resizes its pyramid automatically. `ToneMapPass` should run last.
The renderer owns registered passes and disposes them on removal/shutdown.

Models load static Assimp mesh hierarchies, copy node transforms, and share their
owned textures. Use `new Model(path)` for filesystem models with external assets;
`Model.fromResource(path)` is for self-contained packaged geometry or embedded GLB.
RGB/RGBA textures support STB decoding, filtering and mipmaps. A CPU `Mesh` borrows
textures; a `Model` owns its imported textures. Create GL resources in `start()` and
release them in `end()` while the context still exists. Skinning, PBR, and sorted
translucency remain outside this implementation.

## Verification

```sh
./gradlew build
./gradlew graphicsSmoke
```

Unit tests cover camera/bounds, Slang compatibility and game/level lifecycle order.
The desktop smoke test compiles/links the generated Slang shaders on the local
OpenGL 4.1 driver; checks batch/texture capacity, bloom pixel values, Assimp embedded
textures, framebuffer resizing, directional/spot shadow pixels including an
off-screen caster; then runs the actual Game/MyLevel scene with ImGui. It saves
`build/scene-preview.png` from the displayed game for visual inspection.

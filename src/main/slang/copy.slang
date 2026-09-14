#include "fullscreen.slang"
Sampler2D scene;
[shader("fragment")]
float4 fragmentMain(ScreenVertex input) : SV_Target {
    return scene.Sample(input.uv);
}

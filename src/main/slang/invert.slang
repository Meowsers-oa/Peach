#include "fullscreen.slang"
Sampler2D scene;
[shader("fragment")]
float4 fragmentMain(ScreenVertex input) : SV_Target {
    float4 color = scene.Sample(input.uv);
    return float4(1 - color.rgb, color.a);
}

#include "fullscreen.slang"
Sampler2D scene;
uniform float exposure;
[shader("fragment")]
float4 fragmentMain(ScreenVertex input) : SV_Target {
    float4 color = scene.Sample(input.uv);
    float3 mapped = 1 - exp(-max(color.rgb, float3(0)) * exposure);
    return float4(pow(mapped, float3(1.0 / 2.2)), color.a);
}

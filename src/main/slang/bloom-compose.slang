#include "fullscreen.slang"
Sampler2D scene;
Sampler2D bloom;
uniform float intensity;
[shader("fragment")]
float4 fragmentMain(ScreenVertex input) : SV_Target {
    float4 color = scene.Sample(input.uv);
    return float4(color.rgb + bloom.Sample(input.uv).rgb * max(intensity, 0), color.a);
}

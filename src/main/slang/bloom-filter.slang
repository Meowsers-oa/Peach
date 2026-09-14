#include "fullscreen.slang"
Sampler2D source;
Sampler2D detail;
uniform float2 texel;
uniform float threshold;
uniform int mode;
float3 sampleSource(float2 uv, float2 offset) {
    float3 color = source.Sample(uv + offset * texel).rgb;
    if (mode == 0) {
        float brightness = max(max(color.r, color.g), color.b);
        color *= max(brightness - threshold, 0) / max(brightness, 0.00001);
    }
    return color;
}
[shader("fragment")]
float4 fragmentMain(ScreenVertex input) : SV_Target {
    float2 uv = input.uv;
    float3 color = sampleSource(uv, float2(0)) * 4;
    color += (sampleSource(uv, float2(-1, 0)) + sampleSource(uv, float2(1, 0))
            + sampleSource(uv, float2(0, -1)) + sampleSource(uv, float2(0, 1))) * 2;
    color += sampleSource(uv, float2(-1, -1)) + sampleSource(uv, float2(1, -1))
            + sampleSource(uv, float2(-1, 1)) + sampleSource(uv, float2(1, 1));
    color /= 16;
    if (mode == 2) color = (color + detail.Sample(uv).rgb) * 0.5;
    return float4(color, 1);
}

#include "geometry.slang"
struct Light {
    int type;
    float3 position;
    float3 direction;
    float3 color;
    float intensity;
    float innerCutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
};
uniform Light lights[8];
uniform int lightCount;
uniform float3 ambient;
uniform float4x4 lightViewProjection;
uniform int shadowLight;
uniform float shadowTexel;
uniform float shadowBias;
Sampler2D shadowMap;

float visibility(float3 position, float3 normal, float3 toLight) {
    float4 clip = mul(lightViewProjection, float4(position, 1));
    if (clip.w <= 0) return 1;
    float3 projected = clip.xyz / clip.w * 0.5 + 0.5;
    if (any(projected < 0) || any(projected > 1)) return 1;
    float bias = max(shadowBias * (1 - dot(normal, toLight)), shadowBias * 0.25);
    float visible = 0;
    // 3x3 PCF softens the depth comparison, not the depth texture itself.
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            float depth = shadowMap.SampleLevel(projected.xy + float2(x, y) * shadowTexel, 0).r;
            visible += projected.z - bias <= depth ? 1 : 0;
        }
    }
    return visible / 9;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float4 base = input.color * sampleColor(input.uv, input.textureSlot);
    if (base.a < 0.01) discard;
    float3 illumination = lightCount == 0 ? float3(1) : ambient;
    float3 normal = input.normal / max(length(input.normal), 0.00001);
    for (int i = 0; i < lightCount; i++) {
        Light light = lights[i];
        float3 ray = light.type == 0 ? -light.direction : light.position - input.worldPosition;
        float distance = length(ray);
        float3 toLight = ray / max(distance, 0.00001);
        float attenuation = 1;
        if (light.type != 0) {
            attenuation = 1 / max(light.constant + light.linear * distance + light.quadratic * distance * distance, 0.00001);
        }
        if (light.type == 2) {
            float3 direction = light.direction / max(length(light.direction), 0.00001);
            attenuation *= saturate((dot(-toLight, direction) - light.outerCutoff) / max(light.innerCutoff - light.outerCutoff, 0.00001));
        }
        float shadow = i == shadowLight ? visibility(input.worldPosition, normal, toLight) : 1;
        illumination += max(dot(normal, toLight), 0) * light.color * max(light.intensity, 0) * attenuation * shadow;
    }
    return float4(base.rgb * illumination + input.emission, base.a);
}

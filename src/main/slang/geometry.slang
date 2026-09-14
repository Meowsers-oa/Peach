// Shared raster interface. Attribute locations match RenderBatch.
struct VertexInput {
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float textureSlot : TEXCOORD1;
    float3 emission : COLOR1;
};
struct VertexOutput {
    float4 position : SV_Position;
    float3 worldPosition : TEXCOORD0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD1;
    float3 normal : NORMAL;
    nointerpolation int textureSlot : TEXCOORD2;
    float3 emission : COLOR1;
};
uniform float4x4 viewProjection;
Sampler2D textures[8];

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    output.position = mul(viewProjection, float4(input.position, 1));
    output.worldPosition = input.position;
    output.color = input.color;
    output.uv = input.uv;
    output.normal = input.normal;
    output.textureSlot = int(input.textureSlot);
    output.emission = input.emission;
    return output;
}

float4 sampleColor(float2 uv, int slot) {
    float2 dx = ddx(uv), dy = ddy(uv);
    // Constant sampler indices are required by Apple's OpenGL 4.1 driver.
    switch (slot) {
        case 0: return textures[0].SampleGrad(uv, dx, dy);
        case 1: return textures[1].SampleGrad(uv, dx, dy);
        case 2: return textures[2].SampleGrad(uv, dx, dy);
        case 3: return textures[3].SampleGrad(uv, dx, dy);
        case 4: return textures[4].SampleGrad(uv, dx, dy);
        case 5: return textures[5].SampleGrad(uv, dx, dy);
        case 6: return textures[6].SampleGrad(uv, dx, dy);
        case 7: return textures[7].SampleGrad(uv, dx, dy);
        default: return float4(1);
    }
}

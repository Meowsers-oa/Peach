struct ScreenVertex {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};
[shader("vertex")]
ScreenVertex vertexMain(float2 uv : POSITION) {
    ScreenVertex output;
    output.uv = uv;
    output.position = float4(output.uv * 2 - 1, 0, 1);
    return output;
}

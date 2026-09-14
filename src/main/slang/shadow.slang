#include "geometry.slang"
[shader("fragment")]
void fragmentMain(VertexOutput input) {
    if (input.color.a * sampleColor(input.uv, input.textureSlot).a < 0.01) discard;
}

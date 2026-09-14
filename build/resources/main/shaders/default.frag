#version 410 core

in vec4 vColor;
in vec2 vUv;
flat in int vTextureSlot;

uniform sampler2D uTextures[{{TEXTURE_SLOTS}}];

out vec4 fragmentColor;

vec4 sampleTexture() {
    // Compute derivatives before the switch so mipmaps work across texture boundaries.
    vec2 dx = dFdx(vUv);
    vec2 dy = dFdy(vUv);
    switch (vTextureSlot) {
{{TEXTURE_CASES}}
        default: return vec4(1.0f);
    }
}

void main() {
    fragmentColor = sampleTexture() * vColor;
    if (fragmentColor.a == 0.0f) discard;
}

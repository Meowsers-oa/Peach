#version 410 core

in vec2 vOffset;
uniform vec3 uColor;
out vec4 fragColor;

void main() {
    float falloff = max(0.0, 1.0 - length(vOffset));
    fragColor = vec4(uColor * falloff * falloff, 0.0);
}

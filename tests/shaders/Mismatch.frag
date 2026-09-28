#version 410 core

// The batch vertex shader supplies vec2 vUV; this must fail during linking.
in vec3 vUV;
out vec4 fragColor;

void main() {
    fragColor = vec4(vUV, 1.0);
}

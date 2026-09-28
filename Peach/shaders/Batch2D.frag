#version 410 core

in vec2 vUV;
in vec4 vColor;
flat in int vTextureSlot;

uniform sampler2D uTextures[16];

out vec4 fragColor;

void main() {
    vec2 dx = dFdx(vUV);
    vec2 dy = dFdy(vUV);
    vec4 sampled = vec4(1.0);

    // OpenGL 4.1 requires constant sampler indices for per-primitive selection.
    switch (vTextureSlot) {
        case 0: sampled = textureGrad(uTextures[0], vUV, dx, dy); break;
        case 1: sampled = textureGrad(uTextures[1], vUV, dx, dy); break;
        case 2: sampled = textureGrad(uTextures[2], vUV, dx, dy); break;
        case 3: sampled = textureGrad(uTextures[3], vUV, dx, dy); break;
        case 4: sampled = textureGrad(uTextures[4], vUV, dx, dy); break;
        case 5: sampled = textureGrad(uTextures[5], vUV, dx, dy); break;
        case 6: sampled = textureGrad(uTextures[6], vUV, dx, dy); break;
        case 7: sampled = textureGrad(uTextures[7], vUV, dx, dy); break;
        case 8: sampled = textureGrad(uTextures[8], vUV, dx, dy); break;
        case 9: sampled = textureGrad(uTextures[9], vUV, dx, dy); break;
        case 10: sampled = textureGrad(uTextures[10], vUV, dx, dy); break;
        case 11: sampled = textureGrad(uTextures[11], vUV, dx, dy); break;
        case 12: sampled = textureGrad(uTextures[12], vUV, dx, dy); break;
        case 13: sampled = textureGrad(uTextures[13], vUV, dx, dy); break;
        case 14: sampled = textureGrad(uTextures[14], vUV, dx, dy); break;
        case 15: sampled = textureGrad(uTextures[15], vUV, dx, dy); break;
    }
    fragColor = sampled * vColor;
}

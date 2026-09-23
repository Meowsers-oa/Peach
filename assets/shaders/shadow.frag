#version 410 core

in vec2 uv;
in float alpha;
flat in int texId;

uniform sampler2D uTextures[15];

void main() {
    float textureAlpha = 1.0;
    // Literal sampler indices are portable on OpenGL 4.1 drivers.
    switch (texId) {
        case 0: textureAlpha = texture(uTextures[0], uv).a; break;
        case 1: textureAlpha = texture(uTextures[1], uv).a; break;
        case 2: textureAlpha = texture(uTextures[2], uv).a; break;
        case 3: textureAlpha = texture(uTextures[3], uv).a; break;
        case 4: textureAlpha = texture(uTextures[4], uv).a; break;
        case 5: textureAlpha = texture(uTextures[5], uv).a; break;
        case 6: textureAlpha = texture(uTextures[6], uv).a; break;
        case 7: textureAlpha = texture(uTextures[7], uv).a; break;
        case 8: textureAlpha = texture(uTextures[8], uv).a; break;
        case 9: textureAlpha = texture(uTextures[9], uv).a; break;
        case 10: textureAlpha = texture(uTextures[10], uv).a; break;
        case 11: textureAlpha = texture(uTextures[11], uv).a; break;
        case 12: textureAlpha = texture(uTextures[12], uv).a; break;
        case 13: textureAlpha = texture(uTextures[13], uv).a; break;
        case 14: textureAlpha = texture(uTextures[14], uv).a; break;
    }
    if (textureAlpha * alpha < 0.5) discard;
}

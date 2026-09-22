#version 410 core

in vec4 vColor;
in vec3 vNormal;
in vec2 vTexCoord;
in float vTexId;
in vec3 vFragPos;

out vec4 FragColor;

uniform sampler2D uTextures[16];

void main() {
    int id = int(round(vTexId));
    vec4 texColor = vec4(1.0);

    switch (id) {
        case 0:  texColor = texture(uTextures[0],  vTexCoord); break;
        case 1:  texColor = texture(uTextures[1],  vTexCoord); break;
        case 2:  texColor = texture(uTextures[2],  vTexCoord); break;
        case 3:  texColor = texture(uTextures[3],  vTexCoord); break;
        case 4:  texColor = texture(uTextures[4],  vTexCoord); break;
        case 5:  texColor = texture(uTextures[5],  vTexCoord); break;
        case 6:  texColor = texture(uTextures[6],  vTexCoord); break;
        case 7:  texColor = texture(uTextures[7],  vTexCoord); break;
        case 8:  texColor = texture(uTextures[8],  vTexCoord); break;
        case 9:  texColor = texture(uTextures[9],  vTexCoord); break;
        case 10: texColor = texture(uTextures[10], vTexCoord); break;
        case 11: texColor = texture(uTextures[11], vTexCoord); break;
        case 12: texColor = texture(uTextures[12], vTexCoord); break;
        case 13: texColor = texture(uTextures[13], vTexCoord); break;
        case 14: texColor = texture(uTextures[14], vTexCoord); break;
        case 15: texColor = texture(uTextures[15], vTexCoord); break;
        default: texColor = vec4(1.0); break;
    }

    FragColor = vColor * texColor;
}
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in float aTexId;

out vec4 vColor;
out vec3 vNormal;
out vec2 vTexCoord;
out float vTexId;
out vec3 vFragPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vColor = aColor;
    vNormal = aNormal;
    vTexCoord = aTexCoord;
    vTexId = aTexId;
    vFragPos = vec3(uModel * vec4(aPos, 1.0));

    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
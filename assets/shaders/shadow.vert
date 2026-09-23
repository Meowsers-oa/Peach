#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 3) in vec2 aTexCoord;
layout(location = 4) in float aTexId;

uniform mat4 uModel;
uniform mat4 uLightMatrix;

out vec2 uv;
out float alpha;
flat out int texId;

void main() {
    uv = aTexCoord;
    alpha = aColor.a;
    texId = int(round(aTexId));
    gl_Position = uLightMatrix * uModel * vec4(aPos, 1.0);
}

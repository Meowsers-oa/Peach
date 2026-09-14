#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aUv;
layout(location = 3) in float aTextureSlot;

uniform mat4 uViewProjection;

out vec4 vColor;
out vec2 vUv;
flat out int vTextureSlot;

void main() {
    vColor = aColor;
    vUv = aUv;
    vTextureSlot = int(aTextureSlot);
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}

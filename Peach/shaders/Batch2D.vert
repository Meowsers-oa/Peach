#version 410 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(location = 3) in float aTextureSlot;

uniform mat4 uViewProjection;

out vec2 vUV;
out vec4 vColor;
flat out int vTextureSlot;

void main() {
    gl_Position = uViewProjection * aPosition;
    vUV = aUV;
    vColor = aColor;
    vTextureSlot = int(aTextureSlot);
}

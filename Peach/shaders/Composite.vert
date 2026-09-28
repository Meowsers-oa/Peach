#version 410 core

out vec2 vUV;

void main() {
    vec2 corners[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
    vec2 position = corners[gl_VertexID];
    vUV = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0, 1);
}

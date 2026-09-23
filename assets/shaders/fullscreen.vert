#version 410 core

out vec2 vTexCoord;

void main() {
    const vec2 corners[4] = vec2[4](
        vec2(-1.0, -1.0), vec2(1.0, -1.0),
        vec2(-1.0, 1.0), vec2(1.0, 1.0)
    );
    vec2 position = corners[gl_VertexID];
    vTexCoord = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}
